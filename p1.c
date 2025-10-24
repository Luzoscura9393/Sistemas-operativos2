#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>   
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf("Uso: p1 N a1 a2 a3\n");
    return 3;
  }else{
  sem_t *semaforo = sem_open("/semaforo", 0);
  sem_t *semaforo4 = sem_open("/semaforo4", 0);

  if (semaforo == SEM_FAILED || semaforo4 == SEM_FAILED) {
    perror("sem_open fallo\n");
    return 1;
  }

  int valor;
  int valor4;
  if (sem_getvalue(semaforo, &valor) == -1 || sem_getvalue(semaforo4, &valor4) == -1) {
    perror("getvalue error en P3 y P4\n");
    sem_close(semaforo);
    sem_close(semaforo4);
    return 1;
  }

  if (valor > 0 && valor4 > 0) {
    pid_t pid;
    int N = atoi(argv[1]);
    int a1 = atoi(argv[2]);
    int a2 = atoi(argv[3]);
    int a3 = atoi(argv[4]);
    sem_unlink("/dato1");
    sem_unlink("/dato2");
    const char maximo[] = "/tmp/miFifo";


    if (mkfifo(maximo, 0666) == -1 && errno != EEXIST) {
       perror("Error creando /tmp/miFifo");
       exit(1);
    }


    int fd = open(maximo, O_WRONLY);
    if (fd == -1) {
        perror("Error abriendo /tmp/miFifo para escritura");
        exit(1);
     }


    if (write(fd, &N, sizeof(int)) != sizeof(int)) {
        perror("Error escribiendo en /tmp/miFifo");
    }
    close(fd);

    const char maximo4[] = "/tmp/miFifo4";


    if (mkfifo(maximo4, 0666) == -1 && errno != EEXIST) {
        perror("Error creando /tmp/miFifo4");
        exit(1);
    }


    int fd4 = open(maximo4, O_WRONLY);
    if (fd4 == -1) {
        perror("Error abriendo /tmp/miFifo4 para escritura");
        exit(1);
    } 


    if (write(fd4, &N, sizeof(int)) != sizeof(int)) {
        perror("Error escribiendo en /tmp/miFifo4");
    }
    close(fd4);

   const char p1_confirm[] = "/tmp/p1_confirm";

   if (mkfifo(p1_confirm, 0666) == -1 && errno != EEXIST) {
        perror("Error creando /tmp/p1_confirm");
        exit(1);
   }

   const char p2_confirm[] = "/tmp/p2_confirm";
   if (mkfifo(p2_confirm, 0666) == -1 && errno != EEXIST) {
        perror("Error creando /tmp/p2_confirm");
        exit(1);
   }
    

    pid = fork();

    if (pid < 0) {
      printf("Error al crear el segundo proceso\n");
    }
    else if (pid == 0){
      sem_t *esperando2 = sem_open("/esperando2", 0);
      sem_t *esperando3 = sem_open("/esperando3", 0);

      if (esperando2 == SEM_FAILED || esperando3 == SEM_FAILED) {
        perror("Error abriendo semáforos en P2");
        exit(1);
      }
      const char *nombre_memoria = "/mem_compartida";
      int ft = shm_open(nombre_memoria, O_RDWR, 0666);
      if (ft == -1) {
        perror("Error abriendo memoria compartida");
        return 1;
      }

      int *buffer = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, ft, 0);
      if (buffer == MAP_FAILED) {
        perror("Error en mmap");
        close(ft);
        return 1;
      }
      close(ft);
      for(int i = a3; i < N + a3; i++) {
         sem_wait(esperando3);
         *buffer = pow(2, i);
         printf("2: %d\n", *buffer);
         sem_post(esperando2);

      }
      sem_wait(esperando3);
      *buffer = -2;
      sem_post(esperando2);
      int confirm;
      int rfd = open(p2_confirm, O_RDONLY);
      if (rfd == -1) {
         perror("Error abriendo FIFO de confirmación en P2");
      } else {
        if (read(rfd, &confirm, sizeof(int)) == sizeof(int)) {
            if (confirm == -3) {
                printf("P2 termina\n");
            }
        }
        close(rfd);
      }
      sem_close(esperando2);
      sem_close(esperando3);
      return 0;
    }
    else {
      sem_t *esperando1 = sem_open("/esperando1", 0);
      sem_t *esperando4 = sem_open("/esperando4", 0);
      if (esperando1 == SEM_FAILED || esperando4 == SEM_FAILED) {
         perror("Error abriendo semáforos en P1");
         exit(1);
      }
      const char *nombre_memoria = "/mem_compartida";
      int ft = shm_open(nombre_memoria, O_RDWR, 0666);
      if (ft == -1) {
        perror("Error abriendo memoria compartida");
        return 1;
      }

      int *buffer = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, ft, 0);
      if (buffer == MAP_FAILED) {
        perror("Error en mmap");
        close(ft);
        return 1;
      }
      close(ft);
      *buffer = a1;
      printf("1: %d\n", *buffer);

      sem_post(esperando1);
      sem_wait(esperando4);
      
      *buffer = a2;
      printf("1: %d\n", *buffer);
      sem_post(esperando1);
      sem_wait(esperando4);

      for(int i = 0; i < N - 2; i ++){
        int j = a2;
        a2 = a1 + a2;
        *buffer = a2;
        printf("1: %d\n", *buffer);
        a1 = j;
        sem_post(esperando1);
        sem_wait(esperando4);
      }
    *buffer = -1;                
    sem_post(esperando1);        
    int confirm;
    int rfd = open("/tmp/p1_confirm", O_RDONLY);
    if (rfd == -1) {
        perror("Error abriendo FIFO de confirmación en P1");
    } else {
      if (read(rfd, &confirm, sizeof(int)) == sizeof(int)) {
        if (confirm == -3) {
            printf("P1 termina\n");
        }
      }
    close(rfd);
   }
    munmap(buffer, sizeof(int));
    if (shm_unlink("/mem_compartida") == -1) {  
        if (errno != ENOENT) perror("P1: shm_unlink fallo");
     }
    sem_close(esperando1);
    sem_close(esperando4);
    unlink("/tmp/miFifo");
    unlink("/tmp/miFifo4");
    unlink("/tmp/p1_confirm");
    unlink("/tmp/p2_confirm");
    return 0;
    }
  }
  else if (valor > 0) {
    printf("P4 no esta en ejecucion\n");
    sem_close(semaforo);
    sem_close(semaforo4);
    return 1;
  } else if (valor4 > 0) {
    printf("P3 no esta en ejecucion\n");
    sem_close(semaforo);
    sem_close(semaforo4);
    return 1;
  }  else {
    printf("P3 y P4 no se estan ejecutando\n");
    sem_close(semaforo);
    sem_close(semaforo4);
    return 1;
  }
  }
}

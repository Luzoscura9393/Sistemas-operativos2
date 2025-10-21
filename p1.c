#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>   
#include <sys/mman.h>

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf("Uso: p1 N a1 a2 a3\n");
    return 3;
  }else{
  sem_t *semaforo = sem_open("/semaforo", O_CREAT | O_RDWR, 0666, 0);
  sem_t *semaforo4 = sem_open("/semaforo4", O_CREAT | O_RDWR, 0666, 0);

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

    sem_t *dato1 = sem_open("/dato1", 0666, 0);
    sem_t *dato3 = sem_open("/dato3", O_CREAT | O_RDWR, 0666, 0);
    const char maximo[] = "/tmp/miFifo";
    //unlink(maximo);
    mkfifo(maximo, 0666);
    int fd = open(maximo, O_WRONLY);

    write(fd, &N, sizeof(int));
    close(fd);
    sem_post(dato1);
    
    sem_wait(dato3);

    pid = fork();

    if (pid < 0) {
      printf("Error al crear el segundo proceso\n");
    }
    else if (pid == 0){
      sem_t *esperando2 = sem_open("/esperando2", 0666, 0);
      sem_t *esperando3 = sem_open("/esperando3", O_CREAT | O_RDWR, 0666, 0);

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
      sem_wait(esperando3);

      for(int i = a3; i < N + a3; i++) {
         *buffer = pow(2, i);
         printf("2: %d\n", *buffer);
         sem_post(esperando2);
         sem_wait(esperando3);
      }
      printf("P2 terminado\n");
      return -2;
    }
    else {
      sem_t *esperando1 = sem_open("/esperando1", 0666, -1);
      sem_t *esperando4 = sem_open("/esperando4", O_CREAT | O_RDWR, 0666, 0);

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
    printf("P1 terminado\n");
    return -1;
    }
  }
  else if (valor > 0) {
    printf("P4 no esta en ejecucion\n");
    sem_destroy(semaforo);
    sem_init(semaforo, 0666, 0);
    sem_close(semaforo);
    sem_close(semaforo4);
    return 1;
  } else if (valor4 > 0) {
    printf("P3 no esta en ejecucion\n");
    sem_close(semaforo);
    sem_destroy(semaforo4);
    sem_init(semaforo4, 0666, 0);
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

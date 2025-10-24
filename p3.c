#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/mman.h>

int main(void) {
  const char nombre[] = "/semaforo";

  sem_t *semaforo = sem_open(nombre, O_CREAT | O_RDWR, 0666, 0);

  if (semaforo == SEM_FAILED) {
    perror("sem_open fallo\n");
    return 1;
  }
  sem_post(semaforo);

  sem_t *esperando1 = sem_open("/esperando1", O_CREAT | O_RDWR, 0666, 0);
  sem_t *esperando3 = sem_open("/esperando3", O_CREAT | O_RDWR, 0666, 0);  
  
  if (esperando1 == SEM_FAILED) {
    perror("Error al abrir semáforo dato1\n");
    exit(1);
  }
  if (esperando3 == SEM_FAILED) {
    perror("Error al abrir semáforo dato3\n");
    exit(1);
  }

  sem_t *dato1 = sem_open("/dato1", O_CREAT | O_RDWR, 0666, 0);
  sem_t *dato2 = sem_open("/dato2", O_CREAT | O_RDWR, 0666, 0);

  if (dato1 == SEM_FAILED) {
    perror("Error al abrir semáforo dato1\n");
    exit(1);
  }
  if (dato2 == SEM_FAILED) {
    perror("Error al abrir semáforo dato2\n");
    exit(1);
  }

  const char maximo[] = "/tmp/miFifo";
  printf("Esperando por P1\n");

  int fd = open(maximo, O_RDONLY);
  if (fd == -1) {
    perror("Error abriendo FIFO /tmp/miFifo en P3");
    exit(1);
  }

  int N;
  if (read(fd, &N, sizeof(int)) != sizeof(int)) {
    perror("Error leyendo N desde FIFO /tmp/miFifo en P3");
    close(fd);
    exit(1);
  }  


  const char *nombre_memoria = "/mem_compartida";

  int ft = shm_open(nombre_memoria, O_CREAT | O_RDWR, 0666);
  if(ft == -1){
  perror("ERROR EN SHM_OPEN");
  return 1;
  }
  if(ftruncate(ft,sizeof(int)) == -1){
  perror("ERROR EN FTRUNCATE");
  close(ft);
  return 1;
  }
  
  int *buffer = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, ft, 0);
  for (int i = 0; i < N + 1; i++) {
      sem_wait(esperando1);

      if (*buffer == -1) {  
        printf("P3 recibió testigo -1\n");
        sem_post(esperando3);
        
        const char *fifo_conf = "/tmp/p1_confirm";
        if (mkfifo(fifo_conf, 0666) == -1 && errno != EEXIST) {
        perror("P3: mkfifo /tmp/p1_confirm falló");
        }
        int wfd = open(fifo_conf, O_WRONLY);
        if (wfd != -1) {
            int confirm = -3;
            if (write(wfd, &confirm, sizeof(int)) != sizeof(int)) {
                perror("P3: error escribiendo confirmacion en FIFO");
            }
            close(wfd);
        } else {
             perror("P3: error abriendo /tmp/p1_confirm para escritura");
        }

      break;  
    }    

    printf("%d\n", *buffer);
    sem_post(esperando3);
}

  printf("P3 termina\n");
  munmap(buffer, sizeof(int));
  close(ft); 
  sem_close(esperando1);
  sem_close(esperando3);
  sem_unlink("/esperando1");
  sem_unlink("/esperando3");
  sem_close(semaforo);
  return 0;
}

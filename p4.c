#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/mman.h>

int main(void) {
  const char nombre[] = "/semaforo4";

  sem_t *semaforo = sem_open(nombre, O_CREAT | O_RDWR, 0666, 0);

  if (semaforo == SEM_FAILED) {
    perror("sem_open fallo\n");
    return 1;
  }
  sem_post(semaforo);

  sem_t *esperando2 = sem_open("/esperando2", O_CREAT | O_RDWR, 0666, 0);
  sem_t *esperando4 = sem_open("/esperando4", O_CREAT | O_RDWR, 0666, 0);

  if (esperando2 == SEM_FAILED) {
    perror("Error al abrir semáforo dato1\n");
    exit(1);
  }
  if (esperando4 == SEM_FAILED) {
    perror("Error al abrir semáforo dato3\n");
    exit(1);
  }


  sem_t *dato2 = sem_open("/dato2", O_CREAT | O_RDWR, 0666, 0);
  sem_t *dato3 = sem_open("/dato3", O_CREAT | O_RDWR, 0666, 0);

  if (dato2 == SEM_FAILED) {
    perror("Error al abrir semáforo dato2\n");
    exit(1);
  }
  if (dato3 == SEM_FAILED) {
    perror("Error al abrir semáforo dato3\n");
    exit(1);
  }

  const char maximo[] = "/tmp/miFifo4";
  printf("Esperando por P2\n");


  int fd = open(maximo, O_RDONLY);
  if (fd == -1) {
    perror("Error abriendo FIFO /tmp/miFifo4 en P4");
    exit(1);
  }

  int N;
  if (read(fd, &N, sizeof(int)) != sizeof(int)) {
    perror("Error leyendo N desde FIFO /tmp/miFifo4 en P4");
    close(fd);
    exit(1);
  }
  close(fd);

  const char *nombre_memoria = "/mem_compartida";

  int ft = shm_open(nombre_memoria, O_RDWR, 0);
  if (ft == -1) {
    perror("Error abriendo memoria compartida en P4");
    return 1;
  }

  int *buffer = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, ft, 0);
  if (buffer == MAP_FAILED) {
    perror("Error en mmap en P4");
    close(ft);
    return 1;
  }
  close(ft);
  for (int i = 0; i < N + 1; i++) {
      sem_wait(esperando2);

      if (*buffer == -2) {
         printf("P4 recibió testigo -2\n");
         sem_post(esperando4);
        
          const char *fifo_conf = "/tmp/p2_confirm";
          if (mkfifo(fifo_conf, 0666) == -1 && errno != EEXIST) {
              perror("P4: mkfifo /tmp/p2_confirm falló");
          }
          int wfd = open(fifo_conf, O_WRONLY);
          if (wfd != -1) {
              int confirm = -3;
              if (write(wfd, &confirm, sizeof(int)) != sizeof(int)) {
                  perror("P4: error escribiendo confirmacion en FIFO");
              }
          close(wfd);
          } else {
                perror("P4: error abriendo /tmp/p2_confirm para escritura");
          }

           break; 
}
      printf("%d\n", *buffer);
      sem_post(esperando4);
} 
  printf("P4 termina\n");

  munmap(buffer, sizeof(int));

  sem_close(esperando2);
  sem_close(esperando4);
  sem_unlink("/esperando2");
  sem_unlink("/esperando4");
  sem_close(semaforo);

  return 0;
}

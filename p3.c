#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main(void) {
  const char nombre[] = "/semaforo4";

  sem_t *semaforo = sem_open(nombre, O_CREAT | O_RDWR, 0666, 0);
  sem_t *esperando2 = sem_open("/esperando2", O_CREAT | O_RDWR, 0666, 0);
  sem_t *esperando1 = sem_open("/esperando1", 0666, 0);

  if (semaforo == SEM_FAILED) {
    perror("sem_open fallo\n");
    return 1;
  }

  sem_post(semaforo);

  sem_t *dato2 = sem_open("/dato2", O_CREAT | O_RDWR, 0666, 0);
  sem_t *dato3 = sem_open("/dato3", 0666, 0);
  const char maximo[] = "/tmp/miFifo";
  int fd = open(maximo, O_RDONLY);
  int N;
  read(fd, &N, sizeof(int));
  close(fd);
  printf("Esperando por P2\n");
  sem_wait(dato2);
  
  
  
  sem_post(dato3);

  const char *nombre_memoria = "/mem_compartida";
  int fd = shm_open(nombre_memoria, O_RDWR, 0666);
  if (fd == -1) {
    perror("Error abriendo memoria compartida");
    return 1;
  }


  int *buffer = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (buffer == MAP_FAILED) {
    perror("Error en mmap");
    close(fd);
    return 1;
  }

  for(int i = 0; i < N; i++) {
    sem_wait(esperando2);
    printf("%d\n", *buffer);
    sem_post(esperando1);
  }

  sem_wait(semaforo);
  sem_close(semaforo);
  return 0;
}

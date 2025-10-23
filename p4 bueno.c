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

  //sem_unlink("/dato2");
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
  //unlink(maximo);
  mkfifo(maximo, 0666);
  printf("Esperando por P2\n");
  //sem_wait(dato2);
  int fd = open(maximo, O_RDONLY | O_CREAT, 0666);
  int N;
  read(fd, &N, sizeof(int));
  close(fd);


  //sem_wait(dato2);

  //sem_post(dato3);

  const char *nombre_memoria = "/mem_compartida";
  int ft = shm_open(nombre_memoria, O_RDWR, 0666);
  if (ft == -1) {
    perror("Error abriendo memoria compartida");
    return 1;
  }
  int *buffer = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  for(int i = 0; i < N; i++) {
  sem_wait(esperando2);
  printf("%d\n", *buffer);
  sem_post(esperando4);
  }
  printf("P4 termina\n");
  sem_wait(semaforo);
  sem_close(semaforo);
  return 0;
}

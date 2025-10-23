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
  //sem_unlink("/dato1");
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
  //unlink(maximo);
  mkfifo(maximo, 0666);
  printf("Esperando por P1\n");
  //sem_wait(dato1);
  int fd = open(maximo, O_RDONLY | O_CREAT, 0666);
  int N;
  read(fd, &N, sizeof(int));
  close(fd);
  
  //sem_post(dato2);

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
  for(int i = 0; i < N; i++) {
    sem_wait(esperando1);
    printf("%d\n", *buffer);
    sem_post(esperando3);
  }

  printf("P3 termina\n");
  sem_wait(semaforo);
  sem_close(semaforo);
  return 0;
}

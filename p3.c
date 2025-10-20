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
  sem_t *esperando1 = sem_open("/esperando1", O_CREAT | O_RDWR, 0666, 0);
  sem_t *esperando3 = sem_open("/esperando3", 0666, 0);


  if (semaforo == SEM_FAILED) {
    perror("sem_open fallo\n");
    return 1;
  }

  sem_post(semaforo);
  
  sem_t *dato1 = sem_open("/dato1", O_CREAT | O_RDWR, 0666, 0);
  sem_t *dato2 = sem_open("/dato2", 0666, 0);
  const char maximo[] = "/tmp/miFifo";
  int fd = open(maximo, O_RDONLY);
  int N;
  read(fd, &N, sizeof(int));
  close(fd);
  
  printf("Esperando por P1\n");
  sem_wait(dato1);
  
  sem_post(dato2);


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
  int *buffer = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(buffer == MAP_FAILED){
  perror("ERROR EN EL MMAP");
  close(ft);
  return 1;
  }



  
  for(int i = 0; i < N; i++) {
    sem_wait(esperando1);
    printf("%d\n", *buffer);
    sem_post(esperando3);
  }

  sem_wait(semaforo);
  sem_close(semaforo);
  return 0;
}

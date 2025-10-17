#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/ips.h>
#include <sys/shm.h>

int main(void) {
  const char nombre[] = "/semaforo";

  sem_t *semaforo = sem_open(nombre, O_CREAT | O_RDWR, 0666, 0);

  if (semaforo == SEM_FAILED) {
    perror("sem_open fallo\n");
    return 1;
  }

  sem_post(semaforo);

  int conexion = shmget(12345, sizeof(int), IPC_CREAT | 0666);
  int *buffer (int *)shmat(shmid, NULL, 0);

  sem_wait(semaforo);
  sem_close(semaforo);
  return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(void) {
  sem_t *semaforo;
  int valor;

  semaforo = sem_open("/tmp/semaforo", 0);

  if (semaforo == SEM_FAILED) {
    perror("sem_open fallo\n");
    return 1;
  }
  if (sem_getvalue(semaforo, &valor) == -1) {
    perror("sem_getvaalue fallo\n");
    sem_close(semaforo);
    return 1;
  }
  

  if (valor > 0) {
    printf("Desactvado\n");
  }
  else {
    printf("%d\n", valor);
  }
  sem_close(semaforo);
  sem_unlink("/tmp/semaforo");
  return 0;
}

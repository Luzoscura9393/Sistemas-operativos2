#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>

int main(int argc, char *argv[]) {
  sem_t *semaforo = sem_open("/semaforo", 0666, 1);
  sem_t *semaforo4 = sem_open("/semaforo4", 0666, 1);

  if (semaforo == SEM_FAILED | semaforo4 == SEM_FAILED) {
    perror("sem_open fallo\n");
    return 1;
  }

  int valor;
  int valor4;
  if (sem_getvalue(semaforo, &valor) == -1 | sem_getvalue(semaforo4, &valor4) == -1) {
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
  
    pid = fork();

    if (pid < 0) {
      printf("Error al crear el segundo proceso\n");
    }
    else if (pid == 0){
      for(int i = a3; i < N + a3; i++) {
        printf("%.0f\n", pow(2, i));
      }
    printf("P2 termina\n");
    return -2;
    }
    else {
      printf("%d\n", a1);
      printf("%d\n", a2);
      for(int i = 0; i < N - 2; i ++){
        int j = a2;
        a2 = a1 + a2;
        printf("%d\n", a2);
        a1 = j;
      }
    printf("P1 terminado\n");
    return -1;
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

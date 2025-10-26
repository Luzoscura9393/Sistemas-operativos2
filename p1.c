#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf("Uso: p1 N a1 a2 a3\n");
    return 3;
  } else {
    /* Evitar bloqueo: comprobar existencia en /dev/shm antes de sem_open */
    if (access("/dev/shm/sem.semaforo", F_OK) == -1 ||
        access("/dev/shm/sem.semaforo4", F_OK) == -1) {
      printf("P3 y P4 no se estan ejecutando\n");
      return 1;
    }

    sem_t *semaforo = sem_open("/semaforo", O_RDWR);
    sem_t *semaforo4 = sem_open("/semaforo4", O_RDWR);

    if (semaforo == SEM_FAILED || semaforo4 == SEM_FAILED) {
      printf("P3 o P4 no estan en ejecucion\n");
      return 1;
    }

    int valor, valor4;
    if (sem_getvalue(semaforo, &valor) == -1 ||
        sem_getvalue(semaforo4, &valor4) == -1) {
      perror("getvalue error en P3 y P4\n");
      sem_close(semaforo);
      sem_close(semaforo4);
      return 1;
    }

    if (!(valor > 0 && valor4 > 0)) {
      if (valor <= 0 && valor4 > 0) {
        printf("P3 no esta en ejecucion\n");
      } else if (valor > 0 && valor4 <= 0) {
        printf("P4 no esta en ejecucion\n");
      } else {
        printf("P3 y P4 no se estan ejecutando\n");
      }
      sem_close(semaforo);
      sem_close(semaforo4);
      return 1;
    }

    /* Parámetros */
    int N = atoi(argv[1]);
    int a1 = atoi(argv[2]);
    int a2 = atoi(argv[3]);
    int a3 = atoi(argv[4]);

    /* Crear FIFOs para pasar N y confirmaciones */
    const char fifo_N1[] = "/tmp/miFifo";
    const char fifo_N2[] = "/tmp/miFifo4";
    const char p1_confirm[] = "/tmp/p1_confirm";
    const char p2_confirm[] = "/tmp/p2_confirm";

    if (mkfifo(fifo_N1, 0666) == -1 && errno != EEXIST) {
      perror("Error creando /tmp/miFifo");
      exit(1);
    }
    if (mkfifo(fifo_N2, 0666) == -1 && errno != EEXIST) {
      perror("Error creando /tmp/miFifo4");
      exit(1);
    }
    /* confirm FIFOs (puede que ya existan) */
    if (mkfifo(p1_confirm, 0666) == -1 && errno != EEXIST) { }
    if (mkfifo(p2_confirm, 0666) == -1 && errno != EEXIST) { }

    /* Escribir N en ambas FIFOs (bloquea hasta que lectores abran) */
    int fd1 = open(fifo_N1, O_WRONLY);
    if (fd1 == -1) {
      perror("Error abriendo /tmp/miFifo para escritura"); exit(1);
    }
    if (write(fd1, &N, sizeof(int)) != sizeof(int)) {
      perror("Error escribiendo en /tmp/miFifo");
    }
    close(fd1);

    int fd4 = open(fifo_N2, O_WRONLY);
    if (fd4 == -1) {
      perror("Error abriendo /tmp/miFifo4 para escritura");
      exit(1);
    }
    if (write(fd4, &N, sizeof(int)) != sizeof(int)) {
      perror("Error escribiendo en /tmp/miFifo4");
    }
    close(fd4);

    /* Abrir memoria compartida creada por P3 */
    const char *nombre_memoria = "/mem_compartida";
    int ft = shm_open(nombre_memoria, O_RDWR, 0);
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
    close(ft);

    /* Crear P2 (hijo) */
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork fallo");
      munmap(buffer, sizeof(int));
      exit(1);
    }

    if (pid == 0) {
      /* P2: potencias de 2, sincronizado con esperando3/esperando2 */
      sem_t *esperando2 = sem_open("/esperando2", 0);
      sem_t *esperando3 = sem_open("/esperando3", 0);
      if (esperando2 == SEM_FAILED || esperando3 == SEM_FAILED) {
        perror("Error abriendo semaforos en P2");
        exit(1);
      }

      for (int i = a3; i < N + a3; ++i) {
        sem_wait(esperando3);
        *buffer = (int)pow(2, i);
        fflush(stdout);
        sem_post(esperando2);
      }
      sem_wait(esperando3);
      *buffer = -2;
      sem_post(esperando2);

      /* esperar confirmación de P4 por FIFO */
      int confirm = 0;
      int rfd = open(p2_confirm, O_RDONLY);
      if (rfd == -1) perror("Error abriendo FIFO de confirmacion en P2");
      else {
        if (read(rfd, &confirm, sizeof(int)) == sizeof(int)) {
          if (confirm == -3) { printf("-3 P2 termina\n"); fflush(stdout); }
        }
        close(rfd);
      }
      sem_close(esperando2);
      sem_close(esperando3);
      munmap(buffer, sizeof(int));
      return 0;
    } else {
      /* P1: Fibonacci, sincronizado con esperando1/esperando4 */
      sem_t *esperando1 = sem_open("/esperando1", 0);
      sem_t *esperando4 = sem_open("/esperando4", 0);
      if (esperando1 == SEM_FAILED || esperando4 == SEM_FAILED) {
         perror("Error abriendo semaforos en P1");
         exit(1);
      }

      /* primer término */
      *buffer = a1;
      fflush(stdout);
      sem_post(esperando1);
      sem_wait(esperando4);

      /* segundo término */
      *buffer = a2;
      fflush(stdout);
      sem_post(esperando1);
      sem_wait(esperando4);

      for (int i = 0; i < N - 2; ++i) {
        int j = a2;
        a2 = a1 + a2;
        *buffer = a2;
        fflush(stdout);
        a1 = j;
        sem_post(esperando1);
        sem_wait(esperando4);
      }

      /* enviar testigo -1 y esperar confirmación de P3 */
      *buffer = -1;
      sem_post(esperando1);

      int confirm = 0;
      int rfd = open(p1_confirm, O_RDONLY);
      if (rfd == -1) {perror("Error abriendo FIFO de confirmacion en P1");}
      else {
        if (read(rfd, &confirm, sizeof(int)) == sizeof(int)) {
          if (confirm == -3) { printf("-3 P1 termina\n"); fflush(stdout); }
        }
        close(rfd);
      }

      /* limpieza */
      munmap(buffer, sizeof(int));
      if (shm_unlink("/mem_compartida") == -1 && errno != ENOENT) {
        perror("P1: shm_unlink fallo");
      }
      sem_close(esperando1);
      sem_close(esperando4);

      /* eliminar FIFOs creados por P1 */
      unlink(fifo_N1);
      unlink(fifo_N2);
      unlink(p1_confirm);
      unlink(p2_confirm);

      wait(NULL); /* esperar P2 */
      sem_close(semaforo);
      sem_close(semaforo4);
      return 0;
    }
  }
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

int main(void) {
    const char nombre[] = "/semaforo4";
    const char maximo[] = "/tmp/miFifo4";

    /* Sem�foro de presencia de P4 */
    sem_t *semaforo = sem_open(nombre, O_CREAT, 0666, 0);
    if (semaforo == SEM_FAILED) {
        perror("sem_open fallo");
        return 1;
    }
    sem_post(semaforo);

    /* Sem�foros de sincronizaci�n con P2 y P1 */
    sem_t *esperando2 = sem_open("/esperando2", O_CREAT, 0666, 0);
    sem_t *esperando4 = sem_open("/esperando4", O_CREAT, 0666, 0);
    if (esperando2 == SEM_FAILED || esperando4 == SEM_FAILED) {
        perror("Error creando semaforos en P4");
        return 1;
    }

    if (mkfifo(maximo, 0666) == -1 && errno != EEXIST) {
        perror("P4: Error creando /tmp/miFifo4");
        exit(1);
    }

    printf("Esperando por P2\n");
    fflush(stdout);

    /* Leer N desde FIFO */
    int fd = open(maximo, O_RDONLY);
    if (fd == -1) {
        perror("Error abriendo FIFO /tmp/miFifo4 en P4");
        return 1;
    }
    int N;
    if (read(fd, &N, sizeof(int)) != sizeof(int)) {
        perror("Error leyendo N desde FIFO /tmp/miFifo4 en P4");
        close(fd);
        return 1;
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

    /* Leer valores de P2 */
    for (int i = 0; i < N + 1; i++) {
        sem_wait(esperando2);

        if (*buffer == -2) {
            fflush(stdout);
            sem_post(esperando4);

            /* Confirmar a P2 */
            const char *fifo_conf = "/tmp/p2_confirm";
            if (mkfifo(fifo_conf, 0666) == -1 && errno != EEXIST) {}
            int wfd = open(fifo_conf, O_WRONLY);
            if (wfd != -1) {
                int confirm = -3;
                write(wfd, &confirm, sizeof(int));
                close(wfd);
            }
            break;
        }

        printf("%d\n", *buffer);
        fflush(stdout);
        sem_post(esperando4);
    }

    printf("-3 P4 termina\n");
    fflush(stdout);

    munmap(buffer, sizeof(int));
    sem_close(esperando2);
    sem_close(esperando4);
    sem_unlink("/esperando2");
    sem_unlink("/esperando4");
    sem_close(semaforo);
    sem_unlink("/semaforo4");
    unlink("/tmp/miFifo4");
    return 0;
}
  sem_close(semaforo);

  return 0;
}

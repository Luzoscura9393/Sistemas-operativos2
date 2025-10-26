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
    const char nombre[] = "/semaforo";
    const char maximo[] = "/tmp/miFifo";

    /* Crear sem�foro principal para indicar que P3 est� activo */
    sem_t *semaforo = sem_open(nombre, O_CREAT, 0666, 0);
    if (semaforo == SEM_FAILED) {
        perror("sem_open fallo");
        return 1;
    }
    sem_post(semaforo);

    /* Crear sem�foros de sincronizaci�n */
    sem_t *esperando1 = sem_open("/esperando1", O_CREAT, 0666, 0);
    sem_t *esperando3 = sem_open("/esperando3", O_CREAT, 0666, 0);
    if (esperando1 == SEM_FAILED || esperando3 == SEM_FAILED) {
        perror("Error creando semaforos en P3");
        return 1;
    }

    /* Crear FIFO para recibir N desde P1 */
    if (mkfifo(maximo, 0666) == -1 && errno != EEXIST) {
        perror("Error creando /tmp/miFifo");
        exit(1);
    }

    /* Crear memoria compartida (antes de esperar a P1) */
    const char *nombre_memoria = "/mem_compartida";
    int ft = shm_open(nombre_memoria, O_CREAT | O_RDWR, 0666);
    if (ft == -1) {
        perror("ERROR EN SHM_OPEN");
        return 1;
    }
    if (ftruncate(ft, sizeof(int)) == -1) {
        perror("ERROR EN FTRUNCATE");
        close(ft);
        return 1;
    }
    int *buffer = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, ft, 0);
    if (buffer == MAP_FAILED) {
        perror("Error en mmap en P3");
        close(ft);
        return 1;
    }
    close(ft);

    printf("Esperando por P1\n");
    fflush(stdout);

    /* Leer N enviado por P1 */
    int fd = open(maximo, O_RDONLY);
    if (fd == -1) {
        perror("Error abriendo FIFO /tmp/miFifo en P3");
        exit(1);
    }
    int N;
    if (read(fd, &N, sizeof(int)) != sizeof(int)) {
        perror("Error leyendo N desde FIFO /tmp/miFifo en P3");
        close(fd);
        exit(1);
    }
    close(fd);

    /* Leer N valores + testigo */
    for (int i = 0; i < N + 1; i++) {
        sem_wait(esperando1);

        if (*buffer == -1) {
            fflush(stdout);
            sem_post(esperando3);

            /* Confirmar a P1 por FIFO */
            const char *fifo_conf = "/tmp/p1_confirm";
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
        sem_post(esperando3);
    }

    printf("-3 P3 termina\n");
    fflush(stdout);

    /* Liberar recursos */
    munmap(buffer, sizeof(int));
    sem_close(esperando1);
    sem_close(esperando3);
    sem_unlink("/esperando1");
    sem_unlink("/esperando3");
    sem_close(semaforo);
    sem_unlink("/semaforo");
    unlink("/tmp/miFifo");
    return 0;
}

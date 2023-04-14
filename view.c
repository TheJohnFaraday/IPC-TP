// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>


#define MAX_MEM_NAME_LEN 20
#define MAX_OUTPUT_CHARACTERS 130
#define PIPE_BUF 1024
#define SHM_SIZE 8192
#define MAX_SEM_NAME_LEN 20

int main(int argc, char const *argv[]) {

    // Abrimos el pipe para recibir la información del md5
    int pipe_fd = STDIN_FILENO;

    // Leemos el nombre de la memoria compartida y el nombre del semáforo del pipe

    char pipe_data[PIPE_BUF];
    memset(pipe_data, 0, PIPE_BUF);
    ssize_t pipe_data_len = read(pipe_fd, pipe_data, PIPE_BUF);
    if (pipe_data_len == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    // Extraemos el nombre de la memoria compartida y el nombre del semáforo
    int shm;
    char sem[MAX_SEM_NAME_LEN];
    memset(sem, 0, MAX_SEM_NAME_LEN);
    sscanf(pipe_data, "%d %s", &shm, sem);

    // Abrimos el semáforo
    sem_t *semaphore = sem_open(sem, 0);
    if (semaphore == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    //Mapeamos la memoria compartida
    char *mem_ptr = shmat(shm, NULL, 0);

    // Bucle principal
    while (1) {

        // Esperamos a que el semáforo esté disponible para lectura
        sem_wait(semaphore);

        // Leemos el valor de la me

        if (mem_ptr[0] == '\0') {
            break;
        }

        // Escribimos el valor de la memoria compartida en la salida estándar
        printf("%s", mem_ptr);

    }

    exit (EXIT_SUCCESS);
}

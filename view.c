// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#define PIPE_BUF 20
#define MAX_SEM_NAME_LEN 20
#define SEM_NAME "/sem_name"
#define SHM_NAME "/shm_name"
#define STD_IN 0
#define ERROR -1
#define PAGE_SIZE 4096

int main(int argc, char const *argv[]) {

    // Abrimos el pipe para recibir la información del md5
    char pipe_data[PIPE_BUF];
    memset(pipe_data, 0, PIPE_BUF);

    // Leemos el nombre de la memoria compartida y el nombre del semáforo del pipe

    if(read(STD_IN, pipe_data, PIPE_BUF) == ERROR){
        perror("read");
        exit(EXIT_FAILURE);
    }
    
    int shm_size = atoi(pipe_data);
    int files_toRead = shm_size/PAGE_SIZE;

    // Abrimos la memoria compartida
    int shm_fd;
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == ERROR){
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    char * shm_base_ptr;
    shm_base_ptr = mmap(NULL, shm_size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_base_ptr == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    //Cerramos el fd de la memoria (no lo necesitamos mas)
    if(close(shm_fd) == ERROR){
        perror("close");
        exit(EXIT_FAILURE);
    }

    // Abrimos el semáforo
    sem_t *semaphore = sem_open(SEM_NAME, 0);
    if (semaphore == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    int offset = 0;
    // Bucle principal, se ejecuta hasta que ya no haya mas para leer
    while (files_toRead) {

        // Esperamos a que el semáforo esté disponible para lectura
        if(sem_wait(semaphore) < 0){
            perror("sem_wait");
            exit(EXIT_FAILURE);
        }

        // Leemos el valor de la memoria y lo imprimimos
        offset += printf("%s", shm_base_ptr + offset);

        // Escribimos el valor de la memoria compartida en la salida estándar
        files_toRead--;

    }


    //Termino con el semaforo
    if (sem_close(semaphore) == ERROR) {
        perror("Error al cerrar el semaforo");
        exit(EXIT_FAILURE);
    }

    if(sem_unlink(SEM_NAME) < 0){
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
    //Terminamos con la memoria

    if(munmap(shm_base_ptr, shm_size) < 0){
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    if(shm_unlink(SHM_NAME) < 0){
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
    

    exit (EXIT_SUCCESS);
}

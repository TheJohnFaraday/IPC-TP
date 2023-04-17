// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/defs.h"


int main(int argc, char const *argv[]) {

    //Variable en la que guardamos el size de la memoria compartida
    int shm_size;
    //Si no se recibe ningun parametro por consola, entonces el view esta siendo conectado por un pipe 
    if(argc == NO_PARAMETER){
        // Abrimos el pipe para recibir la información del md5
        char pipe_data[MAX_SHM_SIZE];
        memset(pipe_data, 0, MAX_SHM_SIZE);
        // Leemos el nombre de la memoria compartida y el nombre del semáforo del pipe

        if(read(STD_IN, pipe_data, MAX_SHM_SIZE) == ERROR){
            perror("read");
            exit(EXIT_FAILURE);
        }

        shm_size = atoi(pipe_data);

    } else if(argc == YES_PARAMETER){
        shm_size = atoi(argv[1]);
    } else {
        perror("Incorrect amount of arguments");
        exit(EXIT_FAILURE);
    }

    int files_toRead = shm_size/PAGE_SIZE;

    // Abrimos la memoria compartida
    int shm_fd;
    char * shm_base_ptr;
    
    shm_base_ptr = ropen_shm(&shm_fd, shm_size);

    // Abrimos el semáforo
    sem_t *semaphore = create_sem();

    int offset = 0;
    // Bucle principal, se ejecuta hasta que ya no haya mas para leer
    while (files_toRead) {

        // Esperamos a que el semáforo esté disponible para lectura
        wait_sem(semaphore);

        // Leemos el valor de la memoria y lo imprimimos
        offset += printf("%s", shm_base_ptr + offset);

        // Escribimos el valor de la memoria compartida en la salida estándar
        files_toRead--;

    }


    //Termino con el semaforo
    close_sem(semaphore);

    close_shm(shm_fd, shm_base_ptr, shm_size);

    exit (EXIT_SUCCESS);
}

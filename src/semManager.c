// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include "../include/semManager.h"

//Crea un semaforo inizializado en cero, retorna un puntero a el.
sem_t * create_sem(){

    sem_t * semaphore;

    semaphore = sem_open(SEM_NAME, O_CREAT, 0666, INITIAL_STATE);
        if (semaphore == SEM_FAILED){
            perror("sem_open");
            exit(EXIT_FAILURE);
        }

    return semaphore;    

}

//Recibe el puntero a semaforo y lo incrementa
void post_sem(sem_t * sem){
    if(sem_post(sem) == ERROR){
        perror("sem_post");
        exit(EXIT_FAILURE);
    }
}

//Recibe el puntero a semaforo y lo decrementa, si su valor llega a cero entonces bloquea el proceso
void wait_sem(sem_t * sem){
    if(sem_wait(sem) == ERROR){
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }
}

//Cierro el semaforo pero no lo elimino
void close_sem(sem_t * sem){
    if(sem_close(sem) == ERROR) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }
}

//Elimino el semaforo
void unlink_sem(){
    if(sem_unlink(SEM_NAME) == ERROR){
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
}
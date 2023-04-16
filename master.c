// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

//Modelo naive del master que recibe un solo path y lo deriva a un slave mediante un pipe. El resultado lo obtiene mediante otro pipe

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
#include <sys/stat.h>
#include <pthread.h>
#include <sys/mman.h>


#define INITIAL_PATH 1
#define PIPE_ENTRIES 2
#define MAX_PATH_CHARACTERS 260
#define FILE_RESULT 300
#define MAX_SLAVES 10
#define READ_FD 0
#define WRITE_FD 1
#define STD_OUT 1
#define STD_IN 0
#define ERROR -1
#define SLAVE_PROCESS 0

#define PAGE_SIZE 4096
#define SHM_NAME "/shm_name"
#define SEM_NAME "/sem_name"
#define PIPE_BUF 1024

//Par de pipes por cada relacion master-slave
typedef struct {
    int Path_pipe_fd[PIPE_ENTRIES];
    int Result_pipe_fd[PIPE_ENTRIES];
}slave_pipes;

int main(int argc, char const *argv[])
{
    sleep(5);
    int cant_files = argc-1;
    int cant_slaves;
    size_t shm_size = PAGE_SIZE*cant_files;
    char first_run[MAX_SLAVES];
    memset(first_run, 0, MAX_SLAVES);

    if(cant_files > MAX_SLAVES){
        cant_slaves = MAX_SLAVES;
        if (cant_files > MAX_SLAVES*2){
            memset(first_run, 1, MAX_SLAVES);
            shm_size = PAGE_SIZE * (cant_files - MAX_SLAVES);
        }
        else{
            for (int i = 0; i < cant_files - MAX_SLAVES; i++){
                first_run[i] = 1;
                shm_size -= PAGE_SIZE;
            }
        }
        
    }
    else {
        cant_slaves = cant_files;
    }
    

    int shm_fd;
    char * shm_base_ptr;
    sem_t * semaphore;

    //Creo la memoria compartida
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == ERROR){
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    
    //Ahora que la tenemos definida, asigno un tamano
    if((ftruncate(shm_fd, shm_size) == ERROR)){
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    //Ahora debemos mapear el objeto de memoria compartida a una serie de direcciones dentro del espacio de memoria del proceso
    //Al pasar NULL como primer argumento le damos la libertad al SO para que eliga donde mapear
    //En el 3er parametro asignamos permisos de escritura, mientras que en el 4to aclaramos que la memoria sera compartida entre procesos
    //El ultimo argumento es el offset. En este caso se lee/escribe desde la posicion cero del conjunto de paginas
    shm_base_ptr = mmap(NULL, shm_size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_base_ptr == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    //Cerramos el fd de la memoria (no lo necesitamos mas)
    if(close(shm_fd) < 0){
        perror("close");
        exit(EXIT_FAILURE);
    }


    //Creo el semaforo
    semaphore = sem_open(SEM_NAME, O_CREAT, 0666, 0);
    if (semaphore == SEM_FAILED){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    //Le paso el tamano de la memoria
    printf("%ld", shm_size);
    fflush(stdout);


    slave_pipes pipes[cant_slaves];

    //Creo los pares de pipes
    //Pipe que lee el path y lo manda al slave. Pipe que lee el resultado desde el slave y lo manda al master
    for (int i = 0; i < cant_slaves; i++){
        //Pipe que labura con el path
        if ((pipe(pipes[i].Path_pipe_fd) == ERROR) || (pipe(pipes[i].Result_pipe_fd) == ERROR)){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    FILE * file;
    char filename[] = "resultados.txt";
    file = fopen(filename, "w");
    if (file == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    //Creamos a los esclavo
    pid_t pid[cant_slaves];

    for (int i = 0; i < cant_slaves; i++)
    {
        pid[i] = fork();
        if(pid[i] == ERROR){
            perror("Error al crear proceso esclavo");
            exit(EXIT_FAILURE);
        }


        if(pid[i] == SLAVE_PROCESS){
            //El esclavo solo lee del pipe de path
            close(pipes[i].Path_pipe_fd[WRITE_FD]);
            //Reacomodamos los fds
            close(READ_FD);
            dup(pipes[i].Path_pipe_fd[READ_FD]);
            close(pipes[i].Path_pipe_fd[READ_FD]);
            
            //El esclavo solo escribe del pipe de Result
            close(pipes[i].Result_pipe_fd[READ_FD]);
            //Reacomodamos los fds
            close(WRITE_FD);
            dup(pipes[i].Result_pipe_fd[WRITE_FD]);
            close(pipes[i].Result_pipe_fd[WRITE_FD]);
            
            //Me preparo para el execv
            char * const paramList[] = {"./slave", NULL}; 
            execv("./slave", paramList);
            perror("Error al ejecutar el slave");
            exit(EXIT_FAILURE);
        }
    }

    //Codigo del master
    //Cierro los extremos de los pipes que no uso
    for (int i = 0; i < cant_slaves; i++)
    {
        close(pipes[i].Path_pipe_fd[READ_FD]);
        close(pipes[i].Result_pipe_fd[WRITE_FD]);
    }

    //Me preparo para monitorear las entradas de los pipes
    fd_set read_set, write_set;
    //Inicializo el conjunto
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    for (int i = 0; i < cant_slaves; i++)
    {
        FD_SET(pipes[i].Path_pipe_fd[WRITE_FD], &write_set);
        FD_SET(pipes[i].Result_pipe_fd[READ_FD], &read_set);
    }
    
    int how_many_can_i_read_write;

    //Espero a que esclavo hijo termine
    //Escribo en el pipe el path al archivo
    char md5_result[MAX_PATH_CHARACTERS];
    char toFile_result[FILE_RESULT];
    char slave_status[cant_slaves];
    memset(slave_status, 0, cant_slaves);
    
    int read_files = 0, processed_files= 0, offset=0;
    while(processed_files < cant_files){

        //Select va a retornar la cantidad de fds disponibles para leer o escribir
        how_many_can_i_read_write = select(pipes[cant_slaves-1].Result_pipe_fd[WRITE_FD] + 1, &read_set, &write_set, NULL, NULL);
        if(how_many_can_i_read_write == ERROR){
            perror("select");
            exit(EXIT_FAILURE);
        }

        //Vemos ahora donde podemos escribir y donde podemos leer, no se va a bloquear el proceso
        for (int i = 0; i < cant_slaves; i++)
        {

            if(FD_ISSET(pipes[i].Path_pipe_fd[WRITE_FD], &write_set) && (read_files < cant_files)){
                if(!slave_status[i]) {
                    //Escribo un path is el first_run da 0 o dos paths si da 1
                    if (first_run[i] == 1){
                        char file_paths[MAX_PATH_CHARACTERS];
                        memset(file_paths, 0, MAX_PATH_CHARACTERS);
                        strcat(file_paths, argv[read_files+1]);
                        strcat(file_paths, " ");
                        strcat(file_paths, argv[read_files+2]);
                        strcat(file_paths, " ");
                        
                        write(pipes[i].Path_pipe_fd[WRITE_FD], file_paths, strlen(file_paths));
                        read_files += 2;
                    }
                    else{
                        char file_paths[MAX_PATH_CHARACTERS];
                        memset(file_paths, 0, MAX_PATH_CHARACTERS);
                        strcat(file_paths, argv[read_files+1]);
                        strcat(file_paths, " ");
                        
                        write(pipes[i].Path_pipe_fd[WRITE_FD], file_paths, strlen(file_paths));
                        read_files++;
                    }
                    slave_status[i] = 1;
                }
            }

            if(FD_ISSET(pipes[i].Result_pipe_fd[READ_FD], &read_set) && (processed_files < cant_files) ){
                memset(md5_result, 0 , MAX_PATH_CHARACTERS);
                memset(toFile_result, 0 , FILE_RESULT);
                ssize_t md5_dim = read(pipes[i].Result_pipe_fd[READ_FD], md5_result, MAX_PATH_CHARACTERS);
                if(md5_dim == ERROR){
                    perror("read");
                    exit(EXIT_FAILURE);
                }

                if (first_run[i]) {
                    //Separo los resultados
                    first_run[i] = 0;
                    processed_files += 2;
                    // char * first_result = strtok(md5_result, " ");
                    // char * second_result = strtok(NULL, " ");
                    // strcat(toFile_result, first_result);
                    // sprintf(toFile_result + strlen(first_result), "Slave ID: %d\n", pid[i]);
                    // strcat(toFile_result, second_result);
                    // sprintf(toFile_result + strlen(first_result) + strlen(second_result), "Slave ID: %d\n", pid[i]);
                }
                else{
                    processed_files++;
                    // strcat(toFile_result, md5_result);
                    // sprintf(toFile_result + strlen(md5_result), "Slave ID: %d\n", pid[i]);
                }

                strcat(toFile_result, md5_result);
                sprintf(toFile_result + strlen(md5_result), "Slave ID: %d\n", pid[i]);

                //printf("%s", toFile_result);


                //Los escribo en el archivo
                fprintf(file, "%s", toFile_result); 
                fflush(file);
                
                //Ahora escribo en la memoria
                int entry_size = strlen(toFile_result);
                memcpy(shm_base_ptr + offset, toFile_result, entry_size);

                offset+=entry_size;

                if(sem_post(semaphore)){
                    perror("sem_post");
                    exit(EXIT_FAILURE);
                }
                slave_status[i] = 0;
            }

        }

        //Actualizo la informacion para el select
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        for (int i = 0; i < cant_slaves; i++)
        {
            FD_SET(pipes[i].Path_pipe_fd[WRITE_FD], &write_set);
            FD_SET(pipes[i].Result_pipe_fd[READ_FD], &read_set);
        }
    }
    

    //Termino con el view
    memset(md5_result, 0 , MAX_PATH_CHARACTERS);
    //strncpy(shm_ptr, md5_result, SHM_SIZE);
    //sem_post(semaphore);

    
    //Termino con el esclavo
    for (int i = 0; i < cant_slaves; i++)
    {
        if (kill(pid[i], SIGINT) == ERROR) {
            perror("Error al enviar la señal SIGINT");
            exit(EXIT_FAILURE);
        }
    }
    
    //Terminamos con la memoria
    
    
    if(munmap(shm_base_ptr, shm_size) < 0){
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    
    //Termino con el semaforo
    if(sem_close(semaphore) < 0) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    //Cierro el archivo de resultados
    if (fclose(file) == ERROR) {
        perror("Error al cerrar el archivo de resultados");
        exit(EXIT_FAILURE);
    }
    

    exit(EXIT_SUCCESS);
}
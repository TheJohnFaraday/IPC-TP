// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

//Modelo naive del master que recibe un solo path y lo deriva a un slave mediante un pipe. El resultado lo obtiene mediante otro pipe

#include "../include/defs.h"

int main(int argc, char const *argv[])
{
    
    //Cantidad de esclavos y archivos a procesar
    int cant_files = argc-1;
    int cant_slaves;

    //calculo de memoria a utilizar
    size_t shm_size = PAGE_SIZE*cant_files;
    char first_run[MAX_SLAVES];
    memset(first_run, 0, MAX_SLAVES);

    //Calculamos cantidad de esclavos y recalculamos tamano de memoria segun si hay carga incial de 2 paths
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
    
    //Le paso el tamano de la memoria
    printf("%ld", shm_size);
    fflush(stdout);
    
    //Creamos la memoria
    int shm_fd = create_shm(shm_size);

    //Ahora debemos mapear el objeto de memoria compartida a una serie de direcciones dentro del espacio de memoria del proceso
    char * shm_base_ptr = mmap_shm(shm_fd, shm_size);

    sleep(WAITING_FOR_VIEW_PROCESS);

    //Creo el semaforo
    sem_t * semaphore = create_sem();


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
    updateSelect(pipes, cant_slaves, &read_set, &write_set);
    
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
                        strncat(file_paths, argv[read_files+1], MAX_PATH_CHARACTERS - 1);
                        strncat(file_paths, " ", MAX_PATH_CHARACTERS - strlen(file_paths) - 1); 
                        strncat(file_paths, argv[read_files+2], MAX_PATH_CHARACTERS - strlen(file_paths) - 1);
                        strncat(file_paths, " ", MAX_PATH_CHARACTERS - strlen(file_paths) - 1);
                        
                        write(pipes[i].Path_pipe_fd[WRITE_FD], file_paths, strlen(file_paths));
                        read_files += 2;
                    }
                    else{
                        char file_paths[MAX_PATH_CHARACTERS];
                        memset(file_paths, 0, MAX_PATH_CHARACTERS);
                        strncat(file_paths, argv[read_files+1], MAX_PATH_CHARACTERS - 1);
                        strncat(file_paths, " ", MAX_PATH_CHARACTERS - strlen(file_paths) - 1); 
                        
                        
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

                }
                else{
                    processed_files++;
                }

                strcat(toFile_result, md5_result);
                sprintf(toFile_result + strlen(md5_result), "Slave ID: %d\n", pid[i]);

                //Los escribo en el archivo
                fprintf(file, "%s", toFile_result); 
                fflush(file);
                
                //Ahora escribo en la memoria
                int entry_size = strlen(toFile_result);
                memcpy(shm_base_ptr + offset, toFile_result, entry_size);

                offset+=entry_size;

                post_sem(semaphore);
                slave_status[i] = 0;
            }

        }

        //Actualizo la informacion para el select
        updateSelect(pipes, cant_slaves, &read_set, &write_set);
    }

    
    //Termino con el esclavo
    for (int i = 0; i < cant_slaves; i++)
    {
        if (kill(pid[i], SIGINT) == ERROR) {
            perror("Error al enviar la señal SIGINT");
            exit(EXIT_FAILURE);
        }
    }

    
    //Terminamos con la memoria, la cerramos
    close_shm(shm_fd, shm_base_ptr, shm_size);

    //La eliminamos
    unlink_shm();

    //Termino con el semaforo
    close_sem(semaphore);

    unlink_sem();

    //Cierro el archivo de resultados
    if (fclose(file) == ERROR) {
        perror("Error al cerrar el archivo de resultados");
        exit(EXIT_FAILURE);
    }
    

    exit(EXIT_SUCCESS);
}

/*******************************************************FUNCIONES**************************************************************/


//Actualiza/Inicia informacion necesaria para que el select sepa que extremos de escritura/lectura estan disponibles
void updateSelect(slave_pipes * pipes, int cant_slaves,  fd_set *__restrict __readfds, fd_set *__restrict __writefds){
    //Inicializo el conjunto
    FD_ZERO(__readfds);
    FD_ZERO(__writefds);
    for (int i = 0; i < cant_slaves; i++)
    {
        FD_SET(pipes[i].Path_pipe_fd[WRITE_FD], __writefds);
        FD_SET(pipes[i].Result_pipe_fd[READ_FD], __readfds);
    }
}
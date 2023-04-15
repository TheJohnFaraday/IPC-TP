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

#define INITIAL_PATH 1
#define PIPE_ENTRIES 2
#define MAX_PATH_CHARACTERS 100
#define MAX_SLAVES 10
#define READ_FD 0
#define WRITE_FD 1
#define STD_OUT 1
#define STD_IN 0
#define ERROR -1
#define SLAVE_PROCESS 0

//Par de pipes por cada relacion master-slave
typedef struct {
    int Path_pipe_fd[PIPE_ENTRIES];
    int Result_pipe_fd[PIPE_ENTRIES];
}slave_pipes;

int main(int argc, char const *argv[])
{

    int cant_files = argc-1;
    int cant_slaves;

    if(cant_files > MAX_SLAVES){
        cant_slaves = MAX_SLAVES;
    }
    else {
        cant_slaves = cant_files;
    }

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
    int read_files = 0, processed_files= 0;
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
                write(pipes[i].Path_pipe_fd[WRITE_FD], argv[read_files+1], strlen(argv[read_files+1]));
                read_files++;
            }

            if(FD_ISSET(pipes[i].Result_pipe_fd[READ_FD], &read_set)){
                memset(md5_result, 0 , MAX_PATH_CHARACTERS);
                ssize_t md5_dim = read(pipes[i].Result_pipe_fd[READ_FD], md5_result, MAX_PATH_CHARACTERS);
                if(md5_dim == ERROR){
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                printf("%s \n", md5_result);
                processed_files++;
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
    
    //Termino con el esclavo
    for (int i = 0; i < cant_slaves; i++)
    {
        if (kill(pid[i], SIGINT) == ERROR) {
            perror("Error al enviar la señal SIGINT");
            exit(EXIT_FAILURE);
        }
    }
    
    
    printf("Proceso finalizado \n");
    

    exit(EXIT_SUCCESS);
}
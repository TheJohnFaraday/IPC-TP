
//Modelo naive del master que recibe un solo path y lo deriva a un slave mediante un pipe. El resultado lo obtiene mediante otro pipe

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_PATH 1
#define PIPE_ENTRIES 2
#define READ_FD 0
#define WRITE_FD 1
#define STD_OUT 1
#define STD_IN 0
#define ERROR -1


int main(int argc, char const *argv[])
{
    //Lectura de parametros, paths a partir de argv[1], argv[0] contiene el path al programa
    //Creo que los pipes
    int Path_pipe_fd[PIPE_ENTRIES];
    int Result_pipe_fd[PIPE_ENTRIES];

    //Pipe que labura con el path
    if (pipe(Path_pipe_fd) == ERROR){
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    //Pipe que labura con el hash ya hecho
    if (pipe(Result_pipe_fd) == ERROR){
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    //Creamos al esclavo
    pid_t pid;

    pid = fork();
    if(pid == ERROR){
        perror("Error al crear proceso esclavo");
        exit(EXIT_FAILURE);
    }


    if(pid == 0){
        //El esclavo solo lee del pipe de path
        close(Path_pipe_fd[WRITE_FD]);
        //Reacomodamos los fds
        close(READ_FD);
        dup(Path_pipe_fd[READ_FD]);
        close(Path_pipe_fd[READ_FD]);
        
        //El esclavo solo escribe del pipe de Result
        close(Result_pipe_fd[READ_FD]);
        //Reacomodamos los fds
        close(WRITE_FD);
        dup(Result_pipe_fd[WRITE_FD]);
        close(Result_pipe_fd[WRITE_FD]);

        
        
        //Me preparo para el execv
        char * const paramList[] = {"./slave", NULL}; 
        execv("./slave", paramList);
        perror("Error al ejecutar el slave");
        exit(EXIT_FAILURE);
    }

    //Codigo del master
    //El master no lee del path_pipe, solo escribe. 
    close(Path_pipe_fd[READ_FD]);

    //El master no escribe del Result_pipe, solo lee. 
    close(Result_pipe_fd[WRITE_FD]);

    //Espero a que esclavo hijo termine
    //Escribo en el pipe el path al archivo
    write(Path_pipe_fd[WRITE_FD], argv[1], strlen(argv[1]));
    wait(NULL);
    //printf("El esclavo termino");

    //Leo el resultado
    //Leo el path del path
    char md5_result[100];
    
    ssize_t md5_dim = read(Result_pipe_fd[READ_FD], &md5_result, 100);
    if(md5_dim == -1){
        perror("read");
        exit(EXIT_FAILURE);
    }

    printf("%s \n", md5_result);
    

    exit(EXIT_SUCCESS);
}

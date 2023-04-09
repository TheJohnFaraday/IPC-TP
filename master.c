
//Modelo naive del master que recibe un solo path y lo deriva a un slave mediante un pipe. El resultado lo obtiene mediante otro pipe

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


#define INITIAL_PATH 1
#define PIPE_ENTRIES 2
#define MAX_PATH_CHARACTERS 100
#define READ_FD 0
#define WRITE_FD 1
#define STD_OUT 1
#define STD_IN 0
#define ERROR -1
#define SLAVE_PROCESS 0


int main(int argc, char const *argv[])
{
    //Lectura de parametros, paths a partir de argv[1], argv[0] contiene el path al programa
    //Creo que los pipes
    int Path_pipe_fd[PIPE_ENTRIES];
    int Result_pipe_fd[PIPE_ENTRIES];

    //PPipe que lee el path y lo manda al slave. Pipe que lee el resultado desde el slave y lo manda al master
    if (pipe(Path_pipe_fd) == ERROR || pipe(Result_pipe_fd) == ERROR){
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


    if(pid == SLAVE_PROCESS){
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
    char md5_result[MAX_PATH_CHARACTERS];
    for (int i = INITIAL_PATH; i < argc; i++)
    {
        memset(md5_result, 0 , MAX_PATH_CHARACTERS);
        write(Path_pipe_fd[WRITE_FD], argv[i], strlen(argv[i]));
        ssize_t md5_dim = read(Result_pipe_fd[READ_FD], md5_result, MAX_PATH_CHARACTERS);
        if(md5_dim == ERROR){
            perror("read");
            exit(EXIT_FAILURE);
        }
        printf("%s \n", md5_result);
    }
    
    //Termino con el esclavo
    if (kill(pid, SIGINT) == ERROR) {
        perror("Error al enviar la señal SIGINT");
        exit(EXIT_FAILURE);
    }
    
    printf("Proceso finalizado \n");
    

    exit(EXIT_SUCCESS);
}

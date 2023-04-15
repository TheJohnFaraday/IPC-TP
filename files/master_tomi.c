// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

typedef struct {
    int Path_pipe_fd[PIPE_ENTRIES];
    int Result_pipe_fd[PIPE_ENTRIES];
}slave_pipes;

int calc_slaves (int x){
    int y;
    if (x == 2) {
        y = 2;
    } else if (x >= 100) {
        y = 10;
    } else {
        y = ((x - 2) / 98.0) * 8 + 2;
    }
    return y;

}

int main(int argc, char const *argv[])
{
    //Lectura de parametros, paths a partir de argv[1], argv[0] contiene el path al programa
    //Creo que los pipes
    
    int cant_slaves = argc - 1;
    printf("Cantidad de slaves: %d \n", cant_slaves);
    printf("Cantidad de paths: %d \n", argc-1);
    slave_pipes pipes[cant_slaves];
    int i;
    for (i = 0; i < cant_slaves; i++){
        //Pipe que labura con el path
        if (pipe(pipes[i].Path_pipe_fd) == ERROR){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        //Pipe que labura con el hash ya hecho
        if (pipe(pipes[i].Result_pipe_fd) == ERROR){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    //Creamos a los esclavos
    pid_t pid[cant_slaves];

    for (i = 0; i < cant_slaves; i++)
    {

        pid[i] = fork();
        if(pid[i] == ERROR){
            perror("Error al crear proceso esclavo");
            exit(EXIT_FAILURE);
        }

        printf("pid[%d] = %d     ", i, pid[i]);
        if (pid[i] == 0){
            
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


    //Cierro los pipes que no uso
    for (int i = 0; i < cant_slaves; i++)
    {
        close(pipes[i].Path_pipe_fd[READ_FD]);
        close(pipes[i].Result_pipe_fd[WRITE_FD]);
    }
    
    printf("Pase que se cierren los pipes que no uso \n");

    //Espero a que esclavo hijo termine
    //Escribo en el pipe el path al archivo
    for (int i = 1; i < argc; i++)
    {
        ssize_t path_dim = write(pipes[i-1].Path_pipe_fd[WRITE_FD], argv[i], strlen(argv[i]));
        if(path_dim == -1){
            perror("write");
            exit(EXIT_FAILURE);
        }
        close(pipes[i-1].Path_pipe_fd[WRITE_FD]);
    }
    
    for (int i = 0; i < cant_slaves; i++)
    {
        waitpid(pid[i], NULL, 0);
    }
    //printf("El esclavo termino");

    //Leo el resultado
    //Leo el path del path
    
    
    for (int i = 0; i < cant_slaves; i++)
    {
        char md5_result[100] = {0};
        ssize_t md5_dim = read(pipes[i].Result_pipe_fd[READ_FD], &md5_result, 100);
        if(md5_dim == -1){
            perror("read");
            exit(EXIT_FAILURE);
        }
        printf("%s \n", md5_result);
    }
    

    exit(EXIT_SUCCESS);
}


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

<<<<<<< HEAD

=======
>>>>>>> 1707c7a (prueba de cambios)
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

<<<<<<< HEAD
=======
    printf("Buenasss");
>>>>>>> 1707c7a (prueba de cambios)

    if(pid == 0){
        //El esclavo solo lee del pipe de path
        close(Path_pipe_fd[WRITE_FD]);
        //Reacomodamos los fds
<<<<<<< HEAD
        close(READ_FD);
        dup(Path_pipe_fd[READ_FD]);
        close(Path_pipe_fd[READ_FD]);
=======
        //close(READ_FD);
        //dup(Path_pipe_fd[READ_FD]);
        //close(Path_pipe_fd[READ_FD]);
>>>>>>> 1707c7a (prueba de cambios)
        
        //El esclavo solo escribe del pipe de Result
        close(Result_pipe_fd[READ_FD]);
        //Reacomodamos los fds
<<<<<<< HEAD
        close(WRITE_FD);
        dup(Result_pipe_fd[WRITE_FD]);
        close(Result_pipe_fd[WRITE_FD]);
=======
        //close(WRITE_FD);
        //dup(Result_pipe_fd[WRITE_FD]);
        //close(Result_pipe_fd[WRITE_FD]);
>>>>>>> 1707c7a (prueba de cambios)

        
        
        //Me preparo para el execv
        char * const paramList[] = {"./slave", NULL}; 
<<<<<<< HEAD
        execv("./slave", paramList);
=======
        execve("/usr/bin/md5sum", paramList, NULL);
>>>>>>> 1707c7a (prueba de cambios)
        perror("Error al ejecutar el slave");
        exit(EXIT_FAILURE);
    }

    //Codigo del master
<<<<<<< HEAD
    //El master no lee del path_pipe, solo escribe. 
    close(Path_pipe_fd[READ_FD]);

    //El master no escribe del Result_pipe, solo lee. 
    close(Result_pipe_fd[WRITE_FD]);

    //Espero a que esclavo hijo termine
    //Escribo en el pipe el path al archivo
    write(Path_pipe_fd[WRITE_FD], argv[1], strlen(argv[1]));
=======
    //El master no lee del path_pipe, solo escribe. Necesito igual que el pipe path represente el stdin
    close(Path_pipe_fd[READ_FD]);
    //Reacomodamos los fds
    close(READ_FD);
    dup(Path_pipe_fd[WRITE_FD]);
    close(Path_pipe_fd[WRITE_FD]);

    //El master no escribe del Result_pipe, solo lee. Necesito que lo lea de stdout
    close(Result_pipe_fd[WRITE_FD]);
    //Reacomodamos los fds
    close(WRITE_FD);
    dup(Result_pipe_fd[READ_FD]);
    close(Result_pipe_fd[READ_FD]);

    //Espero a que esclavo hijo termine
    //Escribo en el pipe el path al archivo
    write(STD_IN, argv[1], strlen(argv[1]));
>>>>>>> 1707c7a (prueba de cambios)
    wait(NULL);
    //printf("El esclavo termino");

    //Leo el resultado
    //Leo el path del path
<<<<<<< HEAD
    char md5_result[100];
    
    ssize_t md5_dim = read(Result_pipe_fd[READ_FD], &md5_result, 100);
    if(md5_dim == -1){
        perror("read");
        exit(EXIT_FAILURE);
    }

    printf("%s \n", md5_result);
=======
    //char * md5_result = NULL;
    //size_t md5_size = 0;
    
    //ssize_t md5_dim = getline(&md5_result, &md5_size, stdin);
    //if(md5_dim == -1){
    //    perror("getline");
    //    exit(EXIT_FAILURE);
    //}

    //printf("%s \n", md5_result);
>>>>>>> 1707c7a (prueba de cambios)
    

    exit(EXIT_SUCCESS);
}

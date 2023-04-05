
/*Lee los archivos a traves de un pipe (entrada estandar)*/

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

#define STD_IN 0
#define READ_FD 0
#define WRITE_FD 1
#define MAX_PATH_CHARACTERS 50
#define PIPE_ENTRIES 2
#define ERROR -1

int main(int argc, char const *argv[])
{
    //Leo el path de std_in
    char * file_path = NULL;
    size_t path_size = 0;
    
    ssize_t path_dim = getline(&file_path, &path_size, stdin);
    if(path_dim == -1){
        perror("getline");
        exit(EXIT_FAILURE);
    }

    // Elimino \n del final
    if (path_dim > 0 && file_path[path_dim - 1] == '\n') {
    file_path[path_dim - 1] = '\0';
    path_dim--;
    }
    
    //Creo un proceso hijo para que ejecute md5sum y me comunico con el mediante un pipe
    int fd[PIPE_ENTRIES];
    pid_t pid;
    int status;

    //Creo al pipep
    if (pipe(fd) == ERROR){
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    //Creo al hijo
    pid = fork();
    if(pid == ERROR){
        perror("Error al crear proceso hijo");
        exit(EXIT_FAILURE);
    }

    if(pid == 0){
        //Estamos dentro del proceso hijo
        //No vamos a leer del pipe
        close(fd[READ_FD]);
        //Reacomodamos los fds
        close(WRITE_FD);
        dup(fd[WRITE_FD]);
        close(fd[WRITE_FD]); 

        //Mediante execl pasamos a ejecutar md5sum
        char * const md5_path = "/usr/bin/md5sum";
        char * const paramList[] = {md5_path, file_path, NULL}; 
        execv("/usr/bin/md5sum", paramList);
        perror("Error al ejecutar md5sum");
        exit(EXIT_FAILURE);
    }

    //El padre solo va a leer del pipe
    close(fd[WRITE_FD]);
    //Reasignamos fds
    close(READ_FD);
    dup(fd[READ_FD]);
    close(fd[READ_FD]);

    //Espero a que el hijo termine
    wait(NULL);

    //Ahora quiero leer el resultado

    //Leo el path de std_in
    char * md5_result = NULL;
    size_t md5_size = 0;
    
    ssize_t md5_dim = getline(&md5_result, &md5_size, stdin);
    if(md5_dim == -1){
        perror("getline");
        exit(EXIT_FAILURE);
    }


    free(file_path);
    printf("Termino el slave\n");
    printf("%s \n", md5_result);
    free(md5_result);

    return 0;
}

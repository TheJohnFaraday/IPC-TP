
/*Lee los archivos a traves de un pipe (entrada estandar)*/

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>

#define STD_IN 0
#define READ_FD 0
#define WRITE_FD 1
#define STD_OUT 1
#define MAX_PATH_CHARACTERS 100
#define PIPE_ENTRIES 2
#define ERROR -1
#define SLAVE_PROCESS 0

int main(int argc, char const *argv[])
{
    
        //Creo un proceso hijo para que ejecute md5sum y me comunico con el mediante un pipe
        int fd[PIPE_ENTRIES];
        pid_t pid;
        int status;

        int first_process = 1;

        //Creo al pipep
        if (pipe(fd) == ERROR){
            perror("pipe");
            exit(EXIT_FAILURE);
        }


    while(1){

        //Leo el path de std_in
        //char * file_path = NULL;
        char file_path[MAX_PATH_CHARACTERS];
        memset(file_path, 0, MAX_PATH_CHARACTERS);
        ssize_t path_dim;    

        path_dim = read(STD_IN, file_path, MAX_PATH_CHARACTERS);

        if(path_dim == ERROR){
            perror("read");
            exit(EXIT_FAILURE);
        }
        //Creo al hijo
        pid = fork();
        if(pid == ERROR){
            perror("Error al crear proceso hijo");
            exit(EXIT_FAILURE);
        }


        if(pid == SLAVE_PROCESS){
            //Estamos dentro del proceso hijo
            //No vamos a leer del pipe
            if(first_process){
                close(fd[READ_FD]);
                //Reacomodamos los fds
                close(WRITE_FD);
                dup(fd[WRITE_FD]);
                close(fd[WRITE_FD]); 
            }

            //Mediante execl pasamos a ejecutar md5sum
            char * const md5_path = "/usr/bin/md5sum";
            char * const paramList[] = {md5_path, file_path, NULL}; 
            execv("/usr/bin/md5sum", paramList);
            perror("Error al ejecutar md5sum");
            exit(EXIT_FAILURE);
        }

        //El padre solo va a leer del pipe
        if(first_process){
            close(fd[WRITE_FD]);
            first_process = 0;
        }
    
        //Espero a que el hijo termine
        wait(NULL);

        //Ahora quiero leer el resultado

        //Leo el path de std_in
        char md5_result[MAX_PATH_CHARACTERS];
        memset(md5_result, 0, MAX_PATH_CHARACTERS);
        
        ssize_t md5_dim = read(fd[READ_FD], md5_result, MAX_PATH_CHARACTERS);
        if(md5_dim == ERROR){
            perror("read");
            exit(EXIT_FAILURE);
        }

        write(STD_OUT, md5_result, md5_dim);

    }
    
    exit(EXIT_SUCCESS);
}

// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*Lee los archivos a traves de un pipe (entrada estandar)*/


#include "../include/defs.h"

int main(int argc, char const *argv[])
{
    
        //Creo un proceso hijo para que ejecute md5sum y me comunico con el mediante un pipe
        int fd[PIPE_ENTRIES];
        pid_t pid;

        int first_process = 1;

        //Creo al pipep
        if (pipe(fd) == ERROR){
            perror("pipe");
            exit(EXIT_FAILURE);
        }


    while(UNTIL_MASTER_KILL_ME){

        char output [MAX_OUTPUT_CHARACTERS];
        memset(output, 0, MAX_OUTPUT_CHARACTERS);
        //Leo el path de std_in
        //char * file_path = NULL;
        char file_path[MAX_PATH_CHARACTERS];
        memset(file_path, 0, MAX_PATH_CHARACTERS);  

        ssize_t path_dim = read(STD_IN, file_path, MAX_PATH_CHARACTERS);

        if(path_dim == ERROR){
            perror("read");
            exit(EXIT_FAILURE);
        }

        int cant_paths = 2;

        //Preparamos los parametros para el md5sum
        char path1[MAX_PATH_CHARACTERS];
        char path2[MAX_PATH_CHARACTERS];
        memset(path1, 0, MAX_PATH_CHARACTERS);
        memset(path2, 0, MAX_PATH_CHARACTERS);

        //Procesamos los path leidos y preparamos los argumentos para el md5
        char * token = strtok(file_path, " ");
        if(token == NULL){
            perror("strtok");
            exit(EXIT_FAILURE);
        }
        
        strcpy(path1, token);
        token = strtok(NULL, " ");
        if (token != NULL){
            strcpy(path2, token);
        }
        else{
            cant_paths = 1;
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

            //Mediante execv pasamos a ejecutar md5sum
            char * const md5_path = "/usr/bin/md5sum";
            if (cant_paths == 1){
                char * const paramList[] = {md5_path, path1, NULL};
                execv("/usr/bin/md5sum", paramList);
                perror("Error al ejecutar md5sum");
                exit(EXIT_FAILURE);
            }
            else{
                char * const paramList[] = {md5_path, path1, path2, NULL};
                execv("/usr/bin/md5sum", paramList);
                perror("Error al ejecutar md5sum");
                exit(EXIT_FAILURE);
            }
            
            
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

        //Agrego el resultado al output
        strcat(output, md5_result);
    
        //Imprimo el resultado
        ssize_t output_dim = write(STD_OUT, output, strlen(output));
        if(output_dim == ERROR){
            perror("write");
            exit(EXIT_FAILURE);
        }

    }
    
    exit(EXIT_SUCCESS);
}

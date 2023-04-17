#include "shmManager.h"
#include "semManager.h"

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
#define PIPE_BUF 1024
#define WAITING_FOR_VIEW_PROCESS 5

#define MAX_SHM_SIZE 20
#define NO_PARAMETER 1
#define YES_PARAMETER 2

#define UNTIL_MASTER_KILL_ME 1
#define MAX_OUTPUT_CHARACTERS 260
#define MAX_FILES 2


//Par de pipes por cada relacion master-slave
typedef struct {
    int Path_pipe_fd[PIPE_ENTRIES];
    int Result_pipe_fd[PIPE_ENTRIES];
}slave_pipes;

void updateSelect(slave_pipes * pipes, int cant_slaves,  fd_set *__restrict __readfds, fd_set *__restrict __writefds);

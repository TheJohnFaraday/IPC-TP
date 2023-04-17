#include <semaphore.h>
#include <fcntl.h>           
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

#define SEM_NAME "/sem_name"
#define ERROR -1
#define INITIAL_STATE 0

sem_t * create_sem();
void post_sem(sem_t * sem);
void close_sem(sem_t * sem);
void unlink_sem();
sem_t * open_sem();
void wait_sem(sem_t * sem);
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define SHM_NAME "/shm_name"
#define PAGE_SIZE 4096
#define BUFFER_SIZE 256
#define ERROR -1
#define INITIAL_MEM_ADDRESS 0

int create_shm(size_t shm_size);
char * mmap_shm(int fd, size_t shm_size);
void close_shm(int shm_fd, char * shm_ptr, size_t shm_size);
char * ropen_shm(int * shm_fd, size_t shm_size);

void unlink_shm();
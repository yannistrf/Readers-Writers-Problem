#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>

#include "mem_util.h"

// Returns a pointer to the shared memory segment
void* get_memory_block(const char* filename, int* shmid, int block_size) {
    
    key_t key = ftok(filename, 0);
    printf("Key: %d\n", key);

    *shmid = shmget(key, block_size, IPC_CREAT | MEMORY_SEG_PERM);
    printf("Shmid: %d\n", *shmid);
    if (*shmid == -1) {
        fprintf(stderr, "Shmget failed (errno: %d).\n", errno);
        exit(EXIT_FAILURE);
    }

    void* block = block = shmat(*shmid, NULL, 0);
    if (block == (void*) -1) {
        fprintf(stderr, "Shmat failed (errno: %d).\n", errno);
        exit(EXIT_FAILURE);
    }

    return block;
}

void dettach_shared_memory(void* block) {

    if (shmdt(block) == -1) {
        fprintf(stderr, "Shmdt failed (errno: %d).\n", errno);
        exit(EXIT_FAILURE);
    }
}

void destroy_shared_memory(int shmid) {
    
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "Shmctl failed (errno: %d).\n", errno);
		exit(EXIT_FAILURE);
	}
}
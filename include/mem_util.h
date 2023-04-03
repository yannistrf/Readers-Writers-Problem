#include <semaphore.h>

#define MEMORY_SEG_PERM 0644

// Pointers to each part of the shared memory
typedef struct {
    char* text_block;           // File segment 
    int* curr_seg;              // Current segment in shared memory
    int* children_left;         // Keep track of alive children
    int* segment_readers;       // Array that tells us how many readers are waiting for a segment
    sem_t* access_mem;          // Semaphore for accessing the memory
    sem_t* ready_to_load_seg;   // Parent is ready to load a segment semaphore
    sem_t* loaded_seg;          // Parent loaded the segment semaphore
    sem_t* segment_sems;        // Array of semaphores, one for each segment
} MemoryInfo;


// Returns a pointer to the shared memory segment
void* get_memory_block(const char* filename, int* shmid, int block_size);

void dettach_shared_memory(void* block);

void destroy_shared_memory(int shmid);
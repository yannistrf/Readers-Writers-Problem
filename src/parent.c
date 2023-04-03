#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>

#include "mem_util.h"
#include "file_util.h"

// The function the children processes will execute
int child(MemoryInfo* info, int no_of_segments, int no_of_lines, int requests, int id);

int main(int argc, char** argv) {


    // Check command line arguments
    if (argc != 5) {
        fprintf(stderr, "Usage is %s <filename> <lines_per_seg> <no_of_children> <requests>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Extract the values from the arguments
    const char* filename = argv[1];
    int lines_per_seg = atoi(argv[2]);
    int no_of_children = atoi(argv[3]);
    int requests = atoi(argv[4]);

    printf("Filename: %s | Lines per segment: %d | Children processes: %d | Requests per child process: %d\n",\
            filename, lines_per_seg, no_of_children, requests);

    // Open file, find number of segments and their boundaries
    FILE* fp = fopen(filename, "r");
    int no_of_segments;
    // Don't forget to free the allocated memory
    long int* segment_bounds = file_get_segments(fp, lines_per_seg, &no_of_segments);

    // Calculate the necessary block size
    int text_size = LINESIZE * lines_per_seg;
    int curr_seg_size = sizeof(int);
    int readers_size = no_of_segments * sizeof(int);
    int sems_size = (no_of_segments+3) * sizeof(sem_t);
    int block_size = text_size + curr_seg_size + readers_size + sems_size;

    int shmid;
    void* data = get_memory_block(filename, &shmid, block_size);
    // This struct has the pointers to the beginning of each part at the shared memory
    MemoryInfo info;
    // First part is the text segment
    info.text_block = (char*) data;

    // Second part is an integer, the segment currently inside the memory
    info.curr_seg = (int*) (info.text_block + text_size);

    // Third part is an integer that indicates how many children are still alive
    info.children_left = info.curr_seg + 1;
    *info.children_left = no_of_children;

    // Fourth part is an integer array, tells us the reader waiting for each segment
    info.segment_readers = info.children_left + 1;
    for (int seg = 0; seg < no_of_segments; seg++)
        info.segment_readers[seg] = 0;

    // Fifth part is the semaphores segment
    sem_t* semaphores = (sem_t*) (info.segment_readers + no_of_segments);
    info.access_mem = semaphores;
    info.ready_to_load_seg = semaphores + 1;
    info.loaded_seg = semaphores + 2;
    info.segment_sems = semaphores + 3;

    // Initialize all semaphores
    sem_init(info.access_mem, 1, 1);
    sem_init(info.ready_to_load_seg, 1, 0);
    sem_init(info.loaded_seg, 1, 0);
    for (int seg = 0; seg < no_of_segments; seg++)
        sem_init(info.segment_sems+seg, 1, 1);

    
    // Create children
    for (int i = 0; i < no_of_children; i++) {
        int pid = fork();
        if (pid == -1) {
            fprintf(stderr, "Couldn't create process.\n");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            child(&info, no_of_segments, lines_per_seg, requests, i);
            free(segment_bounds);
            dettach_shared_memory(data);
            fclose(fp);
            exit(EXIT_SUCCESS);
        }
    }

    FILE* log = fopen("logs/parent_log.txt", "w");
    if (log == NULL) {
        fprintf(stderr, "Couldn't create/open parent log file.\n");
        exit(EXIT_FAILURE);
    }
    
    int next_segment = -1;
    while (1) {
    
        // Wait signal to load segment
        sem_wait(info.ready_to_load_seg);

        // If all children finished execution, exit
        if (*info.children_left == 0)
            break;

        // Do we need to load another segment ?
        if (next_segment != *info.curr_seg) {

            if (next_segment != -1) {
                // Keep our logs, segment exiting shared memory
                fprintf(log, "# Segment %d exited shared memory at ", next_segment);
                file_write_time(log);
            }
            // Read from the shared memory which segment to load next
            next_segment = *info.curr_seg;
            // Bring segment inside the shared memory
            file_copy_block(fp, segment_bounds[next_segment], segment_bounds[next_segment+1], info.text_block);
            // Keep our logs, segment entered shared memory
            fprintf(log, "# Segment %d entered shared memory at ", next_segment);
            file_write_time(log);
        }

        // Give signal that the segment is loaded
        sem_post(info.loaded_seg);
    }


    // Wait for children to return
    for (int i = 0; i < no_of_children; i++)
        wait(0);

    free(segment_bounds);
    fclose(fp);
    fclose(log);
    // Destroy all semaphores
    sem_destroy(info.access_mem);
    sem_destroy(info.ready_to_load_seg);
    sem_destroy(info.loaded_seg);
    for (int seg = 0; seg < no_of_segments; seg++)
        sem_destroy(info.segment_sems+seg);

    dettach_shared_memory(data);
    // Free the shared memory space we allocated
    destroy_shared_memory(shmid);
    return 0;
}
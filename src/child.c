#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#include "mem_util.h"
#include "file_util.h"

void choose_line(int no_of_segments, int prev_seg, int no_of_lines, int* segment, int* line);

int child(MemoryInfo* info, int no_of_segments, int no_of_lines, int requests, int id) {

    int prev_seg = -1;
    int next_segment;
    int next_line;
    char* line;
    char log_file[32];
    sprintf(log_file, "logs/child_log%d.txt", id);
    FILE* log = fopen(log_file, "w");

    if (log == NULL) {
        fprintf(stderr, "Couldn't create/open child log file.\n");
        exit(EXIT_FAILURE);
    }

    while (requests != 0) {
        choose_line(no_of_segments, prev_seg, no_of_lines, &next_segment, &next_line);
        // Write to log file
        fprintf(log, "Requested <segment %d line %d> at ", next_segment, next_line);
        file_write_time(log);
        // Wait for the segment semaphore
        sem_wait(&info->segment_sems[next_segment]);
        info->segment_readers[next_segment]++;

        // We are the first one to request this segment
        if (info->segment_readers[next_segment] == 1) {
            // Wait for memory access
            sem_wait(info->access_mem);
            // Update the segment to load value
            *info->curr_seg = next_segment;
            // Signal parent to load the new segment
            sem_post(info->ready_to_load_seg);
            // Wait for parent to load the segment
            sem_wait(info->loaded_seg);
        }
        // Write to log file
        fprintf(log, "Proceed to read <segment %d line %d> at ", next_segment, next_line);
        file_write_time(log);
        sem_post(&info->segment_sems[next_segment]);
        // Got the line we wanted
        line = &info->text_block[LINESIZE*next_line];
        // Write the line to the log file
        fprintf(log, "### %s\n", line);
        usleep(20000);

        sem_wait(&info->segment_sems[next_segment]);
        info->segment_readers[next_segment]--;

        // Last request
        if (requests == 1) {
            (*info->children_left)--;
        }
        // Last child alive, notify parent
        if (*info->children_left == 0)
            sem_post(info->ready_to_load_seg);

        // No more readers for this segment
        // Give up memory access
        if (info->segment_readers[next_segment] == 0)
            sem_post(info->access_mem);
        sem_post(&info->segment_sems[next_segment]);
        
        prev_seg = next_segment;
        requests--;
    }

    fclose(log);
    return 0;
}

// This function picks "randomly" a segment and a line
// randomly -> 70% chance to choose again the previous segment
// The segment and the line values are passed to the given pointers
void choose_line(int no_of_segments, int prev_seg, int no_of_lines, int* segment, int* line) {
    // For randomness purposes only
    static int i = 0;
    srand(time(NULL) + i + getpid());
    i++;
    int r = rand();
    // We'll choose this segment if we decide to don't
    // choose the previous one or if it's the first choice
    int possible_seg = r % no_of_segments;

    // Make the choice
    float p = (double) rand() / RAND_MAX;
    if (p > 0.7 || prev_seg == -1) {
        *segment = possible_seg;
    }
    else {
        *segment = prev_seg;
    }
    
    // Choose the line
    *line = rand() % no_of_lines;
}
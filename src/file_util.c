#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "file_util.h"

// Stores in no_of_segments the number of segments we will divide the file to
// Returns a pointer array with the beginning of every segment
// ***We need to free the allocated space of the segment_indexes array***
long int* file_get_segments(FILE* fp, int lines_per_seg, int* no_of_segments) {
    
    char buffer[LINESIZE];
    int line_count = 0;
    
    // Count number of lines of file
    while (fgets(buffer, LINESIZE, fp) != NULL)
        line_count++;

    // In how many segments will we divide the file ?
    int segments = line_count / lines_per_seg;
    // Last segment
    if (line_count % lines_per_seg != 0)
        segments++;
    
    *no_of_segments = segments;

    // Create array that indicates the beginning of every segment
    long int* segment_indexes = malloc((segments+1) * sizeof(long int));

    // Go back to the beginning of file
    fseek(fp, 0, SEEK_SET);
    // Store the beginning of each segment
    for (int i = 0; i < segments; i++) {
        segment_indexes[i] = ftell(fp);
        for (int j = 0; j < lines_per_seg; j++)
            fgets(buffer, LINESIZE, fp);
    }

    // Also mark EOF
    segment_indexes[segments] = ftell(fp);

    return segment_indexes;
}

// Copies a segment of a file inside the block pointer
int file_copy_block(FILE* fp, long int start_line, long int finish_line, char* block) {
    
    if (start_line > finish_line)
        return -1;

    // Set file pointer to the beginning of the segment
    fseek(fp, start_line, SEEK_SET);

    int i = 0;
    while (ftell(fp) != finish_line) {
        // Copy line
        fgets(&block[i*LINESIZE], LINESIZE-1, fp);
        i++;
    }

    return 0;
}

// Writes the current time inside the file
void file_write_time(FILE* fp) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t now = tv.tv_sec;
    struct tm* t = localtime(&now);

    int hour = t->tm_hour;
    int mins = t->tm_min;
    int secs = t->tm_sec;
    int msecs = tv.tv_usec;  // tv.tv_usec --> microseconds

    fprintf(fp, "%02d:%02d:%02d:%06d\n", hour, mins, secs, msecs);  
}

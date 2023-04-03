#define LINESIZE 1024

// Returns array with the beginning of each segment and
// the number of segments inside no_of_segments pointer
long int* file_get_segments(FILE* fp, int lines_per_seg, int* no_of_segments);

// Copies a segment of a file inside the block pointer
int file_copy_block(FILE* fp, long int start, long int finish, char* block);

// Writes the current time inside the file
void file_write_time(FILE* fp);
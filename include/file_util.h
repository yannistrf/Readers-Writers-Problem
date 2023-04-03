#define LINESIZE 1024

long int* file_get_segments(FILE* fp, int lines_per_seg, int* no_of_segments);

int file_copy_block(FILE* fp, long int start, long int finish, char* block);

void file_write_time(FILE* fp);
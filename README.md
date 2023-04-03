# Readers-Writers Problem
This project was done for my Operating Systems assignment at
the Department of Informatics and Telecommunications at NKUA.

## Description of the problem
The whole problem can be summed up to this: when we have multiple
processes (or threads) reading something, the ones that want to
write need to wait for them to finish reading. Else our program
would have non deterministic behaviour.

In this project we perform a simulation of that. We pass to the
parent process a big file which we divide into smaller segments.
Then the children processes are created and each of them wants
to read a specific line at a specific segment of the file. The
parent can load one segment each time inside the memory. Multiple
children can read the same segment simultaneously.
The ones that want a different segment need to wait with a FIFO
policy (implemented by the semaphores) for the parent to load
their segment. When a child finishes reading its line it proceeds
to its next request.

## Execution
To build the executable main run "make". If you also want to
execute the program with parameters defined inside the Makefile
run "make run". To delete binary and log files run "make clean"
and "make clean_logs". For illustrative purposes we created
the file.txt to pass to our program which counts from 1 to 1000.
Of course you can pass your own file. After execution you can
look inside the log files to check the results inside the log
directory.

## Implementation details

### Extra modules
mem_util    -> a small interface for handling the shared memory
file_util   -> performs certain actions on the argument file and
               helps the logging process

### Shared memory
We tried to allocate the memory dynamically, since we don't know
the size of the input file. The only admission is the line size
which is defined inside the file_util.h. The dynamic allocation
makes the structure a bit complicated though so before reading
the source code make sure you freshen up your pointer knowledge.
Inside the shared memory, besides the file segment, there is also
neccessary information for the exution of the program, like some
variables and some semaphores. Make sure to check the MemoryInfo
struct.

### Timing problem
The parent's work is simple. While there are alive children, wait
on the semaphore to load a segment, load it, then post the load
semaphore to notify the children.

The children's functionality is a bit more complicated. Every
segment has its own semaphore. So once the children has chosen
its line, it waits on the proper semaphore. If it passes that
part then it means that either it's the first child to request
this segment, or that someone else requested the same segment.
and is now loaded. If it's the first then it needs to notify
the parent to load the wanted segment. That's the part where
the FIFO queue is implied, waiting on the access_mem semaphore.
After that the rest of the childern wanting the same segment get
unblocked and they proceed to read. Once all of them are done,
the parent gets notified and loads the next segment.
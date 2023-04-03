# our compiler
CC = gcc

# where to look for header files
INCLUDE = ./include

# compiler flags
CFLAGS = -I$(INCLUDE) -Wall -Werror -pthread

# linker flags
LDFLAGS = -pthread

# command line arguments
# <filename> <lines_per_segment> <no_of_children> <requests_per_child>
ARGS = file.txt 200 12 15

# executable name
EXEC = main

# binary folder
BIN = ./bin

# source code folder
SRC = ./src

# all of our object files
OBJS = $(BIN)/parent.o $(BIN)/child.o $(BIN)/mem_util.o $(BIN)/file_util.o

# rule to build executable
$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(EXEC)

# rule to build our object files
$(BIN)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

# run executable
run: $(EXEC)
	./$(EXEC) $(ARGS)

# clean all binary files
clean:
	rm -f $(OBJS) $(EXEC)

# clean all log files
clean_logs:
	rm -f logs/*

# check for memory leaks
valgrind: $(EXEC)
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXEC) $(ARGS)

.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=mmap_write mmap_read
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)

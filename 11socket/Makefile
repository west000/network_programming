.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=echosrv echocli echocli2
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c &< -o $@
clean:
	rm -f *.o $(BIN)

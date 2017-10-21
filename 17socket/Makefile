.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=echosrv echocli nofile_limit conntest pollsrv
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c &< -o $@
clean:
	rm -f *.o $(BIN)

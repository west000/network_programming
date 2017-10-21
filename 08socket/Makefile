.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=p2psrv p2pcli
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c &< -o $@
clean:
	rm -f *.o $(BIN)

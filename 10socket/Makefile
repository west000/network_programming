.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=echosrv echocli getiplist
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c &< -o $@
clean:
	rm -f *.o $(BIN)

.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=echosrv echocli echosrv_nondeadlock echocli_nondeadlock
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)

.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=msg_get msg_rmid msg_stat msg_set
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)

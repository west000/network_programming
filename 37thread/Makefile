.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=01thread echosrv echocli 02pthread_atfork deadlock_test nondeadlock_test
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
01thread:01thread.o
	$(CC) $(CFLAGS) $^ -o $@ -lpthread
echosrv:echosrv.o
	$(CC) $(CFLAGS) $^ -o $@ -lpthread
02pthread_atfork:02pthread_atfork.o
	$(CC) $(CFLAGS) $^ -o $@ -lpthread
deadlock_test:deadlock_test.o
	$(CC) $(CFLAGS) $^ -o $@ -lpthread
nondeadlock_test:nondeadlock_test.o
	$(CC) $(CFLAGS) $^ -o $@ -lpthread
clean:
	rm -f *.o $(BIN)

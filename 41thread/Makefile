.PHONY:clean
CC=gcc
CFLAGS=-Wall -g
All=main
all:$(All)
OBJS=threadpool.o main.o condition.o
.c.o:
	$(CC) $(CFLAGS) -c $<

main:$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lrt

clean:
	rm -f $(All) *.o

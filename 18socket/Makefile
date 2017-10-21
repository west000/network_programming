.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=echosrv echocli nofile_limit conntest pollsrv epollsrv
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c &< -o $@

%.o:%.cpp
	g++ $(CFLAGS) -c &< -o $@

clean:
	rm -f *.o $(BIN)

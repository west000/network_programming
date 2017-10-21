#ifndef _SYSUTIL_H_
#define  _SYSUTIL_H_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        }while(0)

int read_timeout(int fd, unsigned int wait_seconds);

int write_timeout(int fd, unsigned int wait_seconds);

int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);

void activate_nonblock(int fd);

void deactivate_nonblock(int fd);

int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);

#endif //!_SYSUTIL_H_

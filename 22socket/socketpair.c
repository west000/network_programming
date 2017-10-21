#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define ERR_EXIT(m)			\
	do{				\
		perror(m);		\
		exit(EXIT_FAILURE);	\
	}while(0)

int main(void)
{
	int sockfds[2];
	/*socketpair建立一条全双工的管道，只能用于本机的通信，因此传送的数据不用转换为网络字节序*/
	/*同时只能用于父子进程或者亲缘进程间进行通信，而UNIX域套接字则可以在本机不同进程之间进行通信*/
	if(socketpair(PF_UNIX, SOCK_STREAM, 0, sockfds) < 0)
		ERR_EXIT("socketpair");

	pid_t pid;
	pid = fork();
	if(pid == -1)
		ERR_EXIT("fork");
	
	if(pid > 0)
	{
		int val = 0;
		close(sockfds[1]);
		while(1)
		{
			++val;
			printf("sending data:	%d\n", val);
			write(sockfds[0], &val, sizeof(val));	
			read(sockfds[0], &val, sizeof(val));
			printf("data received:	%d\n", val);
			sleep(1);
		}
	}	
	else if(pid == 0)
	{
		int val;
		close(sockfds[0]);
		while(1)
		{
			read(sockfds[1], &val, sizeof(val));
			++val;
			write(sockfds[1], &val, sizeof(val));
		}
	}

	return 0;
}

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <signal.h>
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

void handler(int sig)
{
	printf("recv a sig=%d\n");
	exit(EXIT_SUCCESS);
}

int main(void) 
{
	int sock;
	if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		ERR_EXIT("socket");
	
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9999);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(sock, (struct sockaddr*)&servaddr ,sizeof(servaddr)) < 0)
		ERR_EXIT("connect");

	pid_t pid;
	pid = fork();
	if(pid == -1)
		ERR_EXIT("fork");
	if(pid == 0) /*child process recv data from sock*/ 
	{
		signal(SIGUSR1, handler);
		char recvbuf[1024];
		while(1)
		{
			memset(recvbuf, 0, sizeof(recvbuf));
			int ret = read(sock, recvbuf, sizeof(recvbuf));
			if(ret == -1)
				ERR_EXIT("read");
			else if(ret == 0)
			{
				printf("peer close\n");
				break;
			}
			fputs(recvbuf, stdout);
			
		}
		printf("p2pcli child close\n");
		kill(getppid(), SIGUSR1);
		exit(EXIT_SUCCESS);
	}	
	else /*parent process get data from input and send to peer*/
	{
		char sendbuf[1024] = {0};
		while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) 
		{
			write(sock, sendbuf, strlen(sendbuf));
			memset(sendbuf, 0, sizeof(sendbuf));
		}
		printf("p2psrv parent close\n");
		exit(EXIT_SUCCESS);
	}
	close(sock);
	
	return 0;
}

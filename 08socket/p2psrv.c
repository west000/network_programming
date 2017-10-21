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
	printf("recv a sig=%d\n", sig);
	exit(EXIT_SUCCESS);
}

int main(void) 
{
	int listenfd;
	if((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
/*	if((listenfd = socket(PF_INET, SOCK_STREAM, )) < 0) */
		ERR_EXIT("socket");
	
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9999);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	/*servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");*/
	/*inet_aton("127.0.0.1", &servaddr.sin_addr);*/

	int on = 1;
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		ERR_EXIT("setsockopt");

	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("bind");
	if(listen(listenfd, SOMAXCONN) < 0)
		ERR_EXIT("listen");

	struct sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);
	int conn;
	if((conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen)) < 0)
		ERR_EXIT("accpet");

	pid_t pid;
	pid = fork();
	if(pid == -1)
		ERR_EXIT("fork");

	if(pid == 0)	/*child process gets data from stdin*/
	{
		signal(SIGUSR1, handler);
		char sendbuf[1024] = {0};
		while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
		{
			write(conn, sendbuf, strlen(sendbuf));
			memset(sendbuf, 0, sizeof(sendbuf));
		}
		printf("p2psrv child close\n");
		exit(EXIT_SUCCESS);
	}
	else	/*parent process recv data from sock and write to stdout*/
	{
		char recvbuf[1024];
		while(1)
		{
			memset(recvbuf, 0, sizeof(recvbuf));
			int ret = read(conn, recvbuf, sizeof(recvbuf));	
			if(ret == -1)
				ERR_EXIT("read");
			else if(ret == 0)
			{
				printf("peer close\n");
				break;
			}
			fputs(recvbuf, stdout);
		}
		printf("p2psrv parent close\n");
		kill(pid, SIGUSR1);
		exit(EXIT_SUCCESS);
	}

	close(listenfd);
	
	return 0;
}

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define ERR_EXIT(m) \
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	}while(0)

void do_service(int conn) 
{
	char recvbuf[1024];
        while(1)
        {
                memset(recvbuf, 0, sizeof(recvbuf));
                int ret = read(conn, recvbuf, sizeof(recvbuf));
                if(ret == 0)
		{
			close(conn);
			printf("client close\n");
			break;
		}
		else if(ret == -1)
			ERR_EXIT("read");
		fputs(recvbuf, stdout);
                write(conn, recvbuf, ret);
        }
        close(conn);
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
	
	pid_t pid;
	while(1)
	{
		if((conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen)) < 0)
                	ERR_EXIT("accpet");
		
		pid = fork();
		if(pid == -1)
			ERR_EXIT("fork");
		if(pid == 0)
		{
			close(listenfd);
			do_service(conn);
			exit(EXIT_SUCCESS);
		}
		else 
			close(conn);
	}

	char recvbuf[1024];
	while(1)
	{
		memset(recvbuf, 0, sizeof(recvbuf));
		int ret = read(conn, recvbuf, sizeof(recvbuf));
		fputs(recvbuf, stdout);
		write(conn, recvbuf, ret);
	}
	close(conn);
	close(listenfd);
	
	return 0;
}

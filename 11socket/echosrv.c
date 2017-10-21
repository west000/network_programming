#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

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

ssize_t readn(int fd, void *buf, size_t count)
{
        size_t nleft = count;
        size_t nread;
        char *bufp = (char*)buf;

        while(nleft > 0)
        {
                if((nread = read(fd, bufp, nleft)) < 0)
                {
                        if(errno == EINTR)      /*interrupted*/
                                continue;
                        return -1;              /*error*/
                }
                else if(nread == 0)             /*EOF*/
                        return count - nleft;

                bufp += nread;
                nleft -= nread;
        }
        return count;
}

ssize_t writen(int fd, const void *buf, size_t count)
{
        size_t nleft = count;
        size_t nwritten;
        char *bufp = (char*)buf;

        while(nleft > 0)
        {
                if((nwritten = write(fd, bufp,  nleft)) < 0)
                {
                        if(errno == EINTR)
                                continue;
                        return -1;
                }
                else if(nwritten == 0)
                        continue;

                bufp += nwritten;
                nleft -= nwritten;
        }
	return count;
}

/*
 * recv与read的不同：
 * 1. recv用于套接字，read用于所有IO
 * 2. recv的flags可以指定为MSG_PEEK，表示从套接字缓冲区读取完数据后，不将缓冲区清空
 * 3. recv只要窥探到有数据，就可以返回，因此不一定会返回len个数据
 *    read会阻塞，直到读到len字节或读到EOF或出错
 * */
ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
	while(1)
	{
		int ret = recv(sockfd, buf, len, MSG_PEEK);
		if(ret == -1 && errno == EINTR)
			continue;
		return ret;
	}
}

/*使用上面的recv_peek实现readline，因此只能用于套接字*/
ssize_t readline(int sockfd, void *buf, size_t maxline)
{
	int ret;
	int nread;
	char *bufp = buf;
	int nleft = maxline;
	while(1)
	{
		ret = recv_peek(sockfd, bufp, nleft);  /*先偷看一下有多少个字节，并保存在bufp中，注意此处读取，不会将数据从套接字缓冲区删除！*/
		if(ret < 0)
			return ret;
		else if(ret == 0)
			return ret;

		nread = ret;
		int i;
		for(i=0; i<nread; i++)
		{
			if(bufp[i] == '\n')	/*如果读到换行符，则将数据冲缓冲区搬走*/
			{
				ret = readn(sockfd, bufp, i+1);	
				if(ret != i+1)	/*如果读不到i+1个字符，表明出错*/
					exit(EXIT_FAILURE);
				return ret;
			}
		}

		if(nread > nleft)
			exit(EXIT_FAILURE);
		
		nleft -= nread;
		ret = readn(sockfd, bufp, nread);
		if(ret != nread) /*之前在recv_peek已经读到nread个字符了，如果现在读不到，证明有问题*/
			exit(EXIT_FAILURE);
		
		bufp += nread;
	}

	return -1;
}

void echo_srv(int conn) 
{
	char recvbuf[1024];
        while(1)
        {
                memset(recvbuf, 0, sizeof(recvbuf));
                int ret = readline(conn, recvbuf, sizeof(recvbuf));
		
		if(ret == -1)
			ERR_EXIT("readline");
		else if(ret == 0)
		{
			printf("client close\n");
			break;
		}

		fputs(recvbuf, stdout);
                writen(conn, recvbuf, strlen(recvbuf));
        }
}

void handle_sigchld(int sig)
{
/*	wait(NULL);*/
	while(waitpid(-1, NULL, WNOHANG) > 0)
		;
}

int main(void) 
{
	/*signal(SIGCHLD, SIG_IGN);*/
	signal(SIGCHLD, handle_sigchld);
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
		printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
			
		pid = fork();
		if(pid == -1)
			ERR_EXIT("fork");
		if(pid == 0)
		{
			close(listenfd);
			echo_srv(conn);
			exit(EXIT_SUCCESS);
		}
		else 
			close(conn);
	}

	return 0;
}

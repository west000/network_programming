#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

void do_service(int conn) 
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
        close(conn);
}

void handle_sigpipe(int sig)
{
	printf("recv a sig=%d\n", sig);
}

int main(void) 
{
	signal(SIGPIPE, handle_sigpipe);
	
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

	int conn;
	struct sockaddr_in peeraddr;
	socklen_t peerlen;

	int i, maxi = 0;
	int client[FD_SETSIZE];
	for(i=0; i<FD_SETSIZE; i++)
		client[i] = -1;

	fd_set rset;
	fd_set allset;
	FD_ZERO(&rset);
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);	

	int maxfd = listenfd;	
	int nready;
	
	int count = 0;
	while(1)
	{
		rset = allset;
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if(nready == -1)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("select");
		}
		if(nready == 0)
			continue;
			
		if(FD_ISSET(listenfd, &rset))	/*监听套接字可读，有新连接建立完成*/
		{
			peerlen = sizeof(peeraddr);
			if((conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen)) < 0)
                        	ERR_EXIT("accpet");
			
			for(i=0; i<FD_SETSIZE; i++)	/*找一个空闲的位置*/
			{
				if(client[i] < 0)
				{
					client[i] = conn;
					if(i > maxi)	/*维护一个最大的非空闲位置*/
						maxi = i;
					break;
				}
			}

			if(i == FD_SETSIZE) /*如果空间满了，直接结束服务器程序*/
			{
				fprintf(stderr, "too many clients\n");
				exit(EXIT_FAILURE);
			}
                	printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
			printf("count=%d\n", ++count);
			FD_SET(conn, &allset);	/*加入监听集合中*/
			if(conn > maxfd)	/*维护一个最大的fd*/
				maxfd = conn;

			if(--nready <= 0)	/*如果没有可读描述符了，就直接进入下一select轮询*/
				continue;
		}

		for(i=0; i<=maxi; i++)
		{
			conn = client[i];
			if(conn == -1)
				continue;

			if(FD_ISSET(conn, &rset))	/*可读*/
			{
				char recvbuf[1024] = {0};
				int ret = readline(conn, recvbuf, sizeof(recvbuf));
                		if(ret == -1)
                        		ERR_EXIT("readline");
           			else if(ret == 0)	/*对方关闭连接*/
                		{
                        		printf("client close\n");
					FD_CLR(conn, &allset);	/*将套接字从监听集合中删除*/
					client[i] = -1;	/*对应的，client[i]变为空闲*/
					close(conn);	/*同时需要关闭连接，使客户端接收到FIN*/
                		}

                		fputs(recvbuf, stdout);	/*输出到标准输出*/
                		writen(conn, recvbuf, strlen(recvbuf));	/*回显回去*/

				if(--nready <= 0) /*没有可读的了，跳出循环，进入下一select轮询*/
					break;
			}
		}
	}

	return 0;
}

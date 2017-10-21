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
 *  * recv与read的不同：
 *   * 1. recv用于套接字，read用于所有IO
 *    * 2. recv的flags可以指定为MSG_PEEK，表示从套接字缓冲区读取完数据后，不将缓冲区清空
 *     * 3. recv只要窥探到有数据，就可以返回，因此不一定会返回len个数据
 *      *    read会阻塞，直到读到len字节或读到EOF或出错
 *       * */
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
                ret = recv_peek(sockfd, bufp, nleft);  /*先偷看一下有多少个字节，并保存在bufp中，注意此
处读取，不会将数据从套接字缓冲区删除！*/
                if(ret < 0)
                        return ret;
                else if(ret == 0)
                        return ret;

                nread = ret;
                int i;
                for(i=0; i<nread; i++)
                {
                        if(bufp[i] == '\n')     /*如果读到换行符，则将数据冲缓冲区搬走*/
                        {
                                ret = readn(sockfd, bufp, i+1);
                                if(ret != i+1)  /*如果读不到i+1个字符，表明出错*/
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

void echo_cli(int sock)
{
/*	char sendbuf[1024] = {0};
        char recvbuf[1024] = {0};
        while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
        {
              	writen(sock, sendbuf, strlen(sendbuf));

                int ret = readline(sock, recvbuf, sizeof(recvbuf));
                if(ret == -1)
                        ERR_EXIT("readline");
                else if(ret == 0)
                {
                        printf("srv close\n");
                        break;
                }

                fputs(recvbuf, stdout);
                memset(recvbuf, 0, sizeof(recvbuf));
                memset(sendbuf, 0, sizeof(sendbuf));
        }
        close(sock);
*/

	fd_set rset;
	FD_ZERO(&rset);
	
	int nready;
	int fd_stdin = fileno(stdin); 	/*这里不用STDOUT这个宏，因为这个宏等于0，而我们不能保证标准输入不会被重定向，虽然我们这里不会被重定向，但是这是一个好的编程习惯*/
	int maxfd = fd_stdin > sock ? fd_stdin : sock;
	
	char sendbuf[1024] = {0};
        char recvbuf[1024] = {0};
	while(1)
	{
		FD_SET(fd_stdin, &rset);
		FD_SET(sock, &rset);
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if(nready == -1) /*错误返回-1*/
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("select");
		}
		if(nready == 0)	/*超时返回0，上面没有设置超时，实际上这一句可以不用*/
			continue;

		if(FD_ISSET(sock, &rset))
		{
			int ret = readline(sock, recvbuf, sizeof(recvbuf));
			if(ret == -1)
				ERR_EXIT("readline");
                	else if(ret == 0)
                	{
                        	printf("srv close\n");
                        	break;
               		}

                	fputs(recvbuf, stdout);
                	memset(recvbuf, 0, sizeof(recvbuf));
		}
		if(FD_ISSET(fd_stdin, &rset))
		{
			if(fgets(sendbuf, sizeof(sendbuf), stdin) == NULL) /*标准输入读到结束符*/
				break;
			writen(sock, sendbuf, strlen(sendbuf));
			memset(sendbuf, 0, sizeof(sendbuf));
		}
	}
	close(sock);
}

void handle_sigpipe(int sig)
{
	/*默认情况下，SIGPIPE会终止进程，但是这里我们不进行终止，打印一句话即可*/
	printf("recv a sig=%d\n", sig);
}

int main(void) 
{
	signal(SIGPIPE, handle_sigpipe);
	/*signal(SIGPIPE, SIG_IGN);*/
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

	struct sockaddr_in localaddr;
	socklen_t addrlen = sizeof(localaddr);	
	if(getsockname(sock, (struct sockaddr*)&localaddr, &addrlen) < 0)
		ERR_EXIT("getsockname");	
	printf("ip=%s port=%d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));	

	struct sockaddr_in peeraddr;
	addrlen = sizeof(peeraddr);
	if(getpeername(sock, (struct sockaddr*)&peeraddr, &addrlen) < 0)
		ERR_EXIT("getsockname");
	printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

	echo_cli(sock);
	
	return 0;
}

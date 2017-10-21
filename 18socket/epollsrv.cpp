#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/epoll.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <vector>
#include <algorithm>

typedef std::vector<struct epoll_event> EventList;

#define ERR_EXIT(m) \
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	}while(0)

void activate_nonblock(int fd)
{
        int ret;
        int flags = fcntl(fd, F_GETFL);
        if(flags == -1)
                ERR_EXIT("fcntl");

        flags |= O_NONBLOCK;
        ret = fcntl(fd, F_SETFL, flags);
        if(ret == -1)
                ERR_EXIT("fcntl");
}

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
	char *bufp = (char*)buf;
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
		ERR_EXIT("socket");
	
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9999);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int on = 1;
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		ERR_EXIT("setsockopt");

	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("bind");
	if(listen(listenfd, SOMAXCONN) < 0)
		ERR_EXIT("listen");

	std::vector<int> clients;  /*保存客户端已连接套接字，暂时没有太大作用*/
	int epollfd;
	epollfd = epoll_create1(EPOLL_CLOEXEC);	/*创建epoll句柄，epoll_create1底层使用红黑树存放套接字；epoll_create底层使用哈希表存放套接字，参数size表示哈希表的大小*/

	struct epoll_event event;
	event.data.fd = listenfd;
	event.events = EPOLLIN | EPOLLET; /*默认情况下是EPOLLLT模式*/
	epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event); /*将监听描述符添加到epoll实例中进行管理*/
	
	EventList events(16); /*用于存放返回的感兴趣描述符以及其事件*/
	struct sockaddr_in peeraddr;
	socklen_t peerlen;
	int conn;
	int i;

	int count = 0;
	int nready;
	while(1)
	{
		nready = epoll_wait(epollfd, &*events.begin(), static_cast<int>(events.size()), -1);
		if(nready == -1)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("epoll_wait");
		}
		if(nready == 0)
			continue;
			
		/*如果返回的描述符数目达到events的容量，则扩容*/
		if((size_t)nready == events.size())
			events.resize(events.size()*2);

		/*遍历返回的描述符*/
		for(i=0; i<nready; i++)
		{
			/*有新的连接建立完成*/
			if(events[i].data.fd == listenfd)
			{
				peerlen = sizeof(peeraddr);
				if((conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen)) < 0)
                                	ERR_EXIT("accpet");
				printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
				printf("count=%d\n", ++count);
				
				clients.push_back(conn); /*将描述符加入到列表中*/
				
				activate_nonblock(conn); /*将描述符设置为非阻塞模式*/
				event.data.fd = conn;
				event.events = EPOLLIN | EPOLLET;
				epoll_ctl(epollfd, EPOLL_CTL_ADD, conn, &event); /*加入到epollfd中进行管理*/
			}
			/*已连接套接字可读*/
			else if(events[i].events & EPOLLIN)
			{
				conn = events[i].data.fd;
				if(conn < 0)
					continue;
				
				char recvbuf[1024] = {0};
				int ret = readline(conn, recvbuf, sizeof(recvbuf));
				if(ret == -1)
					ERR_EXIT("readline");
				if(ret == 0)
				{
					printf("client close\n");
					close(conn);
					
					event = events[i];
					epoll_ctl(epollfd, EPOLL_CTL_DEL, conn, &event); /*从epollfd所管理的套接字中删除*/
					clients.erase(std::remove(clients.begin(), clients.end(), conn), clients.end()); /*从列表中删除*/
				}
				
				fputs(recvbuf, stdout);
				writen(conn, recvbuf, strlen(recvbuf));

			}
		}
	}
	
	return 0;
}

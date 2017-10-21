#include "sysutil.h"

/*测试样例
int ret;
ret = read_timeout(fd, 5);
if(ret == 0)
{
	read(fd, ...);
}
else if(ret == -1 && errno == ETIMEDOUT)
{
	timeout...
}
else 
{
	ERR_EXIT("read_timeout");
}
*/

/*
 * read_timeout - 读超时检测函数，不含读操作
 * @fd:文件描述符
 * @wait_seconds:等到超时秒数，如果为0表示不检测超时
 * 成功（未超时）返回0；失败返回-1；超时返回-1并且errno=ETIMEDOUT
 * */
int read_timeout(int fd, unsigned int wait_seconds)
{
	int ret = 0;
	if(wait_seconds > 0) 
	{
		fd_set read_fdset;
		struct timeval timeout;
		
		FD_ZERO(&read_fdset);
		FD_SET(fd, &read_fdset);
		
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do
		{
			ret = select(fd+1, &read_fdset, NULL, NULL, &timeout);
		}while(ret < 0 && errno == EINTR);

		if(ret == 0) /*超时*/
		{
			ret = -1;
			errno = ETIMEDOUT;
		}
		else if(ret == 1) /*可读*/
			return 0;
	}

	return ret;
}

/*
 * write_timeout - 写超时检测函数，不含写操作
 * @fd:文件描述符
 * @wait_seconds:等到超时秒数，如果为0表示不检测超时
 * 成功（未超时）返回0；失败返回-1；超时返回-1并且errno=ETIMEDOUT
 * */
int write_timeout(int fd, unsigned int wait_seconds)
{
        int ret = 0;
        if(wait_seconds > 0)
        {
                fd_set write_fdset;
                struct timeval timeout;

                FD_ZERO(&write_fdset);
                FD_SET(fd, &write_fdset);

                timeout.tv_sec = wait_seconds;
                timeout.tv_usec = 0;
                do
                {
                        ret = select(fd+1, NULL, &write_fdset, NULL, &timeout);
                }while(ret < 0 && errno == EINTR);

                if(ret == 0) /*超时*/
                {
                        ret = -1;
                        errno = ETIMEDOUT;
                }
                else if(ret == 1) /*可写*/
                        return 0;
        }

        return ret;
}

/*
 * accept_timeout - 带超时的accept
 * @fd:套接字
 * @addr:输出参数，返回对方地址
 * @wait_seconds:等待超时秒数，如果为0表示正常模式
 * 成功（未超时）返回已连接套接字；失败返回-1；超时返回-1并且errno=ETIMEDOUT
 * */
int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
	int ret = 0;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	if(wait_seconds > 0)
	{
		fd_set accept_fdset;
		struct timeval timeout;
		FD_ZERO(&accept_fdset);
		FD_SET(fd, &accept_fdset);
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do
		{
			ret = select(fd+1, &accept_fdset, NULL, NULL, &timeout);
		}while(ret < 0 && errno == EINTR);

		if(ret == -1) /*出错*/
			return -1;
		else if(ret == 0) /*超时*/
		{
			errno = ETIMEDOUT;
			return -1;
		}
	}

	/*成功*/
	if(addr != NULL)
		ret = accept(fd, (struct sockaddr*)addr, &addrlen);
	else
		ret = accept(fd, NULL, NULL);
	if(ret == -1)
		ERR_EXIT("accept");

	return ret;
}

/**
 * activate_nonblock - 设置I/O为非阻塞模式
 * @fd:文件描述符
 * */
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

/**
 * deactivate_nonblock - 设置I/O为阻塞模式
 * @fd:文件描述符
 * */
void deactivate_nonblock(int fd)
{
        int ret;
        int flags = fcntl(fd, F_GETFL);
        if(flags == -1)
                ERR_EXIT("fcntl");

        flags &= ~O_NONBLOCK;
        ret = fcntl(fd, F_SETFL, flags);
        if(ret == -1)
                ERR_EXIT("fcntl");
}

/**
 * connect_timeout - 带超时的connect
 * @fd:套接字
 * @addr:要连接的对方地址
 * @wait_seconds:等待超时秒数，如果为0表示正常模式
 * 成功（未超时）返回已连接套接字；失败返回-1；超时返回-1并且errno=ETIMEDOUT
 */
int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
	int ret = 0;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	
	if(wait_seconds > 0)
		activate_nonblock(fd);	/*将套接字设置为非阻塞模式*/
	
	ret = connect(fd, (struct sockaddr*)addr, addrlen);
	if(ret < 0 && errno == EINPROGRESS) /*connect返回失败，连接正在处理中*/
	{
		fd_set connect_fdset;
		struct timeval timeout;
		FD_ZERO(&connect_fdset);
		FD_SET(fd, &connect_fdset);
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do
		{
			/*一旦连接建立，套接字就可写*/
			ret = select(fd+1, NULL, &connect_fdset, NULL, &timeout);
		}while(ret < 0 && errno == EINTR);
		
		if(ret == 0) /*超时，需要将套接字改为阻塞模式*/
		{
			errno = ETIMEDOUT;
			ret = -1;
		}
		else if(ret < 0) /*出错*/
		{
			return -1;
		}
		else if(ret == 1) /*产生可读可写事件，需要将套接字改为阻塞模式*/
		{
			/*ret=1时，可能有两种情况*/
			/*一种是连接成功*/
			/*一种是套接字产生错误，此时错误信息不回保存在errno变量中，因此需要使用getsockopt来获取*/
			int err;	
			socklen_t socklen = sizeof(err);
			int sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen);
			if(sockoptret == -1)
				return -1;
			
			if(err == 0) /*套接字连接成功*/
				ret = 0;
			else /*套接字产生错误*/
			{
				errno = err;
				ret = -1;
			}
		}
	}
	
	/*connect返回成功，连接成功*/
	if(wait_seconds > 0)
	{
		deactivate_nonblock(fd);
	}
	return ret;
}

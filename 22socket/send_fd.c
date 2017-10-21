#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define ERR_EXIT(m)			\
	do{				\
		perror(m);		\
		exit(EXIT_FAILURE);	\
	}while(0)

/*使用sendmsg来发送描述符给父进程*/
void send_fd(int sock_fd, int send_fd)
{
	int ret;
	struct msghdr msg;
	struct cmsghdr *p_cmsg;
	struct iovec vec;
	char cmsgbuf[CMSG_SPACE(sizeof(send_fd))];	/*CMSG_SPACE返回的是cmsg_len、cmsg_level、cmsg_type、数据（包括补齐的填充字节）的字节数，这块缓冲区用于发送控制信息*/
	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof(cmsgbuf);
	p_cmsg = CMSG_FIRSTHDR(&msg);		/*获得第一块控制信息块的地址，然后在上面填充数据*/
	p_cmsg->cmsg_level = SOL_SOCKET;	
	p_cmsg->cmsg_type = SCM_RIGHTS;
	p_cmsg->cmsg_len = CMSG_LEN(sizeof(send_fd));	/*CMSG_LEN返回的是cmsg_len、cmsg_level、cmsg_t
ype、数据（不包括补齐的填充字节）的字节数*/
	int *p_fds = (int*)CMSG_DATA(p_cmsg);	/*CMSG_DATA返回控制块数据部分的指针*/
	*p_fds = send_fd;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &vec;	/*正常数据为一个数组，但这里数组长度为1*/
	msg.msg_iovlen = 1;
	msg.msg_flags = 0;

	/*这里的目的时候为了发送套接字，而不是正常数据，因此正常数据填充一个字节即可，减少传输的字节*/
	char sendchar = 0;
	vec.iov_base = &sendchar;	/*正常数据为1个字节*/
	vec.iov_len = sizeof(sendchar); 

	ret = sendmsg(sock_fd, &msg, 0);
	if(ret != 1)
		ERR_EXIT("sendmsg");
}

/*使用recvmsg来接收描述符*/
int recv_fd(const int sock_fd)
{
	int ret;
	struct msghdr msg;
	struct iovec vec;
	int recv_fd;
	char cmsgbuf[CMSG_SPACE(sizeof(recv_fd))];

	char recvchar;	
	vec.iov_base = &recvchar;
	vec.iov_len = sizeof(recvchar);

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &vec;
	msg.msg_iovlen = 1;
	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof(cmsgbuf);
	msg.msg_flags = 0;

	/*实现填充控制信息的数据部分（这里的数据是描述符），如果接收到的数据没有覆盖此部分，则表明对方没有发送控制信息块*/	
	int *p_fd = (int*)CMSG_DATA(CMSG_FIRSTHDR(&msg));
	*p_fd = -1;	/*控制信息的描述符先填充为-1*/
	ret = recvmsg(sock_fd, &msg, 0);
	if(ret != 1)	/*由于对方发送的正常数据的长度为1字节，因此recvmsg会返回1*/
		ERR_EXIT("recvmsg");

	struct cmsghdr *p_cmsg = CMSG_FIRSTHDR(&msg);
	if(p_cmsg == NULL)
		ERR_EXIT("no passed fd");
	
	p_fd = (int*)CMSG_DATA(p_cmsg);
	recv_fd = *p_fd;
	if(recv_fd == -1)
		ERR_EXIT("no passed fd");

	return recv_fd;
}

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
		close(sockfds[1]);
		int fd = recv_fd(sockfds[0]);
		char buf[1024] = {0};
		read(fd, buf, sizeof(buf));
		printf("buf=[%s]\n", buf);
	}	
	else if(pid == 0)
	{
		/*子进程打开一个文件，并将文件描述符fd通过sendmsg传送给父进程*/
		/*使用这种方式，父进程接收到文件描述符之后，在父进程也会打开文件描述fd*/
		/*如果单纯使用write将一个fd传送给父进程，父进程是不会打开该fd的！！！*/
		close(sockfds[0]);
		int fd;
		fd = open("test.txt", O_RDONLY); 
		if(fd == -1)
			ERR_EXIT("open");
		send_fd(sockfds[1], fd); 
	}

	return 0;
}

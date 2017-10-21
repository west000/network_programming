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

void echo_srv(int sock)
{
	char recvbuf[1024];
	int n;
	while(1)
	{
		memset(recvbuf, 0, sizeof(recvbuf));
		n = read(sock, recvbuf, sizeof(recvbuf));
		if(n == -1)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("read");
		}
		else if(n == 0)
		{
			printf("client close\n");
			break;
		}
	
		fputs(recvbuf, stdout);
		write(sock, recvbuf, strlen(recvbuf));
	}

	close(sock);
}

int main(void)
{
	int listenfd;
	if((listenfd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
		ERR_EXIT("socket");

	unlink("/tmp/test_socket");
	struct sockaddr_un servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sun_family = AF_UNIX;
	strcpy(servaddr.sun_path, "/tmp/test_socket"); 

	/*每次使用bind绑定listenfd的时候，会生成/tmp/test_socket文件*/
	/*如果之前的/tmp/test_socket文件不删除的话，会出现错误：Address already in use*/
	/*这是不是设置地址重复使用，而是通过unlink删除该文件*/
	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("bind");

	if(listen(listenfd, SOMAXCONN) < 0)
		ERR_EXIT("listen");
	
	int conn;
	pid_t pid;
	while(1)
	{
		if((conn = accept(listenfd, NULL, NULL)) < 0)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("accept");
		}	
		
		/*连接成功，创建一个子进程与客户端进行通信*/
		pid = fork();
		if(pid == 0) /*子进程执行的地方*/
		{	
			close(listenfd);
			echo_srv(conn);
			exit(EXIT_SUCCESS);
		}

		/*父进程执行的地方*/
		close(conn);
	}

	return 0;
}

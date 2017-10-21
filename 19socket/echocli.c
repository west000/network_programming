#include <unistd.h>
#include <sys/types.h>      
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
	do { \
		perror(m); \
		exit(EXIT_FAILURE); \
	}while(0)

void echo_cli(int sock)
{
	struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(9999);
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	/*通常情况下，客户端可以不用bind地址*/

	char sendbuf[1024] = {0};
	char recvbuf[1024] = {0};
	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
	{
		/*如果不使用bind绑定地址，则会在使用sendto的时候绑定地址*/
		/*这里做个实验，如果服务端未打开，然后调用sendto向sock中写数据，这是会写成功*/
		/*因为sendto返回成功不代表数据发送成功，只要sock的缓冲区有足够空间，就将sendbuf的内容拷贝到sock的缓存区中，然后返回*/
		sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
		/*这时候，由于向一个未打开的服务端口写数据，会收到ICPM目的端口不可达错误报文，但是ICMP报告不会通知给未连接套接口sock*/
		/*因此这时recvfrom会阻塞*/
		int ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);

		if(ret == -1)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("recvfrom");	/*如果ICMP错误报文能通知给未连接套接口sock，那么会触发这条语句，但是实验证明这条语句并没有被触发*/
		}

		fputs(recvbuf, stdout);
		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));
	}
	close(sock);
}

int main(void)
{
	int sock;
	if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
/*	if((sock = socket(PE_INET, SOCK_DGRAM, 0)) < 0)*/
		ERR_EXIT("socket");
	
	echo_cli(sock);
		
	return 0;
}

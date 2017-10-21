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

int main(void)
{
	int sock;
	if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
/*	if((sock = socket(PE_INET, SOCK_DGRAM, 0)) < 0)*/
		ERR_EXIT("socket");
	
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9999);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("bind");

	/*这里做个实验，将服务端和客户端写在同一个程序中*/
	/*发送4个字节，但接收缓冲区只有1个字节，分4次读取，发现只能读到1个字节，后面3个字节读不到了*/
	sendto(sock, "ABCD", 4, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
	
	char recvbuf[1];
	int i;
	for(i=0; i<4; i++)
	{
		int n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
		if(n == -1)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("recvfrom");
		}	
		else if(n > 0)
			printf("n=%d %c\n", n, recvbuf[0]);
	}

	return 0;
}

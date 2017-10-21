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

	/*这里做个实验，为sock绑定一个远端地址*/
	connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr));

	char sendbuf[1024] = {0};
	char recvbuf[1024] = {0};
	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
	{
		/*sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));*/
		/*由于使用connect为sock绑定了远端地址，因此发送的时候可以不用指定地址*/
		/*sendto(sock, sendbuf, strlen(sendbuf), 0, NULL, 0);*/
		send(sock, sendbuf, strlen(sendbuf), 0);
		int ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);

		if(ret == -1)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("recvfrom");	/*如果ICMP错误报文能通知给未连接套接口sock，那么会触发这条语句，实验证明：由于使用了connect为sock绑定而来远端地址，因此这里可以触发该语句！recvfrom:Connection Refused*/
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

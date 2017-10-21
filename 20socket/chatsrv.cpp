#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "pub.h"

// 聊天室成员列表
USER_LIST client_list;

void do_login(MESSAGE &msg, int sock, struct sockaddr_in *cliaddr);
void do_logout(MESSAGE &msg, int sock, struct sockaddr_in *cliaddr);
void do_sendlist(int sock, struct sockaddr_in *cliaddr);

void char_srv(int sock)
{
	struct sockaddr_in cliaddr;
	socklen_t clilen;
	int n;
	MESSAGE msg;
	while(1)
	{
		memset(&msg, 0, sizeof(msg));
		clilen = sizeof(cliaddr);
		n = recvfrom(sock, &msg, sizeof(msg), 0, (struct sockaddr*)&cliaddr, &clilen);
		if(n < 0)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("recvfrom");
		}

		int cmd = ntohl(msg.cmd);
		switch(cmd)
		{
		case C2S_LOGIN:
			do_login(msg, sock, &cliaddr);
			break;
		case C2S_LOGOUT:
			do_logout(msg, sock, &cliaddr);
			break;
		case C2S_ONLINE_USER:
			do_sendlist(sock, &cliaddr);
			break;
		default:
			break;
		}
	}
}

void do_login(MESSAGE &msg, int sock, struct sockaddr_in *cliaddr)
{
	USER_INFO user;
	strcpy(user.username, msg.body); /*登录信息，body里面存放用户名*/
	user.ip = cliaddr->sin_addr.s_addr;
	user.port = cliaddr->sin_port;

	/*查找用户*/
	USER_LIST::iterator it;
	for(it=client_list.begin(); it != client_list.end(); ++it)
	{
		if(strcmp(it->username, msg.body) == 0)
			break;
	}

	if(it == client_list.end())	/*没有找到用户，登录成功*/
	{
		printf("has a user login : %s <-> %s:%d\n", msg.body, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));
		client_list.push_back(user);

		// 登录成功应答，此时msg.body为空
		MESSAGE reply_msg;
		memset(&reply_msg, 0, sizeof(reply_msg));
		reply_msg.cmd = htonl(S2C_LOGIN_OK);
		sendto(sock, &reply_msg, sizeof(reply_msg), 0, (struct sockaddr*)cliaddr, sizeof(*cliaddr));

		// 发送在线人数
		int count = htonl((int)client_list.size());
		sendto(sock, &count, sizeof(int), 0, (struct sockaddr*)cliaddr, sizeof(*cliaddr));

		// 发送在线列表
		printf("sending user list information to: %s <-> %s:%d\n", msg.body, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));
		for(it=client_list.begin(); it!=client_list.end(); ++it)
		{
			sendto(sock, &*it, sizeof(USER_INFO), 0, (struct sockaddr*)cliaddr, sizeof(*cliaddr));
		}

		// 向其他用户通知有新用户登录
		for(it=client_list.begin(); it!=client_list.end(); ++it)
		{
			// 不发送给当前客户端连接
			if(strcmp(it->username, msg.body) == 0)
				continue;
			
			struct sockaddr_in peeraddr;
			peeraddr.sin_family = AF_INET;
			peeraddr.sin_port = it->port;
			peeraddr.sin_addr.s_addr = it->ip;
		
			msg.cmd = htonl(S2C_SOMEONE_LOGIN);
			memcpy(msg.body, &user, sizeof(user));

			if(sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr*)&peeraddr, sizeof(peeraddr)) < 0)
				ERR_EXIT("sendto");
		}
	}
	else /*有同名用户已经登录*/ 
	{
		printf("uesr %s has already logined\n", msg.body);

		MESSAGE reply_msg;
		memset(&reply_msg, 0, sizeof(reply_msg));
		reply_msg.cmd = htonl(S2C_ALREADY_LOGINED);
		sendto(sock, &reply_msg, sizeof(reply_msg), 0, (struct sockaddr*)cliaddr, sizeof(*cliaddr));
	}
}

void do_logout(MESSAGE &msg, int sock, struct sockaddr_in *cliaddr)
{
	printf("has a user logout : %s <-> %s:%d\n", msg.body, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));

	USER_LIST::iterator it;
	for(it=client_list.begin(); it!=client_list.end(); ++it)
	{
		if(strcmp(it->username, msg.body) == 0)
			break;
	}

	if(it != client_list.end())
		client_list.erase(it);

	// 向其他用户通知有用户登出，msg.body为空
	for(it=client_list.begin(); it!=client_list.end(); ++it)
        {
                if(strcmp(it->username, msg.body) == 0)
                        continue;
		
		struct sockaddr_in peeraddr;
                peeraddr.sin_family = AF_INET;
                peeraddr.sin_port = it->port;
                peeraddr.sin_addr.s_addr = it->ip;

                msg.cmd = htonl(S2C_SOMEONE_LOGOUT);

                if(sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr*)&peeraddr, sizeof(peeraddr)) < 0)
			ERR_EXIT("sendto");
        }    
}

void do_sendlist(int sock, struct sockaddr_in *cliaddr)
{
	MESSAGE msg;
	msg.cmd = htonl(S2C_ONLINE_USER);
	sendto(sock, (const char*)&msg, sizeof(msg), 0, (struct sockaddr*)cliaddr, sizeof(*cliaddr));
	
	int count = htonl((int)client_list.size());
	/*发送在线用户数*/
	sendto(sock, &count, sizeof(int), 0, (struct sockaddr*)cliaddr, sizeof(*cliaddr));
	/*发送在线用户列表*/
	for(USER_LIST::iterator it=client_list.begin(); it!=client_list.end(); ++it)
	{
		sendto(sock, &*it, sizeof(USER_INFO), 0, (struct sockaddr*)cliaddr, sizeof(*cliaddr));
	}
}

int main(void)
{
	int sock;
	if((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		ERR_EXIT("socket");

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5188);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("bind");

	char_srv(sock);
	return 0;
}

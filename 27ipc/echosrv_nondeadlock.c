#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m)			\
	do {				\
		perror(m);		\
		exit(EXIT_FAILURE);	\
	}while(0)


#define MSGMAX 8192
struct msgbuf {
	long mtype;       /* message type, must be > 0 */
	char mtext[MSGMAX];    /* message data */
};

void echo_srv(int msgid)
{
	int n;
	struct msgbuf msg;
	while(1)
	{
		memset(&msg, 0, sizeof(msg));
		/*服务端从众所周知的消息队列接收数据*/
		if((n = msgrcv(msgid, &msg, MSGMAX, 1, 0)) < 0)
			ERR_EXIT("msgrcv");
		
		/*获取客户端的进程号*/
		int pid = *((int*)msg.mtext);
		/*获取客户端的私有消息队列标识号*/
		int cli_msgid = *((int*)(msg.mtext+4));
 
		printf("pid=%d pri_msgid=%d\n", pid, cli_msgid);

		fputs(msg.mtext+8, stdout);
		msg.mtype = pid;
		/*往客户端的私有消息队列中写数据*/
		msgsnd(cli_msgid, &msg, n, 0);
	}	
}

int main(int argc, char *argv[])
{
	int msgid;
	msgid = msgget(1234, IPC_CREAT | 0666);
	if(msgid == -1)
		ERR_EXIT("msgget");

	echo_srv(msgid);
	
	return 0;
}

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

void echo_cli(int msgid)
{
	/*客户端创建一个私有消息队列*/
	int pri_msgid;
	if((pri_msgid = msgget(IPC_PRIVATE, 0666)) < 0)
		ERR_EXIT("msgget");

	int n;
	int pid;
	pid = getpid();
	struct msgbuf msg;
	*((int*)msg.mtext) = pid;
	*((int*)(msg.mtext+4)) = pri_msgid;

	while(fgets(msg.mtext+8, MSGMAX-8, stdin) != NULL)
	{
		msg.mtype = 1;
		/*往众所周知的消息队列写数据*/
		if(msgsnd(msgid, &msg, 8+strlen(msg.mtext+8), 0) < 0)
			ERR_EXIT("msgsnd");
		printf("pid=%d, mtype=%ld pri_msgid=%d\n", pid, msg.mtype, pri_msgid);
	
		memset(msg.mtext, 0, MSGMAX);
		/*从自己的私有队列收数据*/
		if((n = msgrcv(pri_msgid, &msg, MSGMAX, pid, 0)) < 0)
			ERR_EXIT("msgrcv");

		fputs(msg.mtext+8, stdout);
		printf("pid=%d, mtype=%ld pri_msgid=%d\n", pid, msg.mtype, pri_msgid);
		memset(msg.mtext+8, 0, MSGMAX-8);
	}
}

int main(int argc, char *argv[])
{

	int msgid;
	msgid = msgget(1234, 0);	// 打开消息队列
	if(msgid == -1)
		ERR_EXIT("msgget");

	echo_cli(msgid);
		
	return 0;
}

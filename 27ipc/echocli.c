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
	int n;
	int pid;
	pid = getpid();
	struct msgbuf msg;
	*((int*)msg.mtext) = pid;
	while(fgets(msg.mtext+4, MSGMAX-4, stdin) != NULL)
	{
		msg.mtype = 1;
		if(msgsnd(msgid, &msg, 4+strlen(msg.mtext+4), 0) < 0)
			ERR_EXIT("msgsnd");
		printf("pid=%d, mtype=%ld\n", pid, msg.mtype);
	
		memset(msg.mtext, 0, MSGMAX);
		if((n = msgrcv(msgid, &msg, MSGMAX, pid, 0)) < 0)
			ERR_EXIT("msgrcv");

		fputs(msg.mtext+4, stdout);
		printf("pid=%d, mtype=%ld\n", pid, msg.mtype);
		memset(msg.mtext+4, 0, MSGMAX-4);
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MSGMAX 8192

#define ERR_EXIT(m)			\
	do {				\
		perror(m);		\
		exit(EXIT_FAILURE);	\
	}while(0)


struct msgbuf {
	long mtype;       /* message type, must be > 0 */
	char mtext[1];    /* message data */
};

int main(int argc, char *argv[])
{
	int flag = 0;
	int type = 0;
	int opt;
	
	while(1)
	{
		opt = getopt(argc, argv, "nt:");
		if(opt == '?')	// 有没法解析的参数
			exit(EXIT_FAILURE);
		
		if(opt == -1)  // 解析完了
			break;
		
		switch(opt)
		{
		case 'n':
			flag |= IPC_NOWAIT;
			break;
		case 't':
			type = atoi(optarg);
			break;
		}
	}

	int msgid;
	msgid = msgget(1234, 0);	// 打开消息队列
	if(msgid == -1)
		ERR_EXIT("msgget");

	struct msgbuf *ptr;
	ptr = (struct msgbuf*)malloc(sizeof(long) + MSGMAX);
	ptr->mtype = type;
	int n = 0;
	if((n = msgrcv(msgid, ptr, MSGMAX, type, flag)) < 0)
		ERR_EXIT("msgrcv");
		
	printf("read %d bytes type=%ld\n", n, ptr->mtype);

	return 0;
}

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define ERR_EXIT(m)			\
	do {				\
		perror(m);		\
		exit(EXIT_FAILURE);	\
	}while(0)

int main(void)
{
	int msgid;
	msgid = msgget(1234, 0);	// 打开消息队列
	if(msgid == -1)
		ERR_EXIT("msgget");
	
	printf("msgget succ\n");
	printf("msgid = %d\n", msgid);

	if(msgctl(msgid, IPC_RMID, NULL) < 0)
		ERR_EXIT("msgctl");
	return 0;
}

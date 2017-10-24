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
	//msgid = msgget(1234, 0666);		 // 如果消息队列1234不存在，这样打开会出现ENOENT错误
	//msgid = msgget(1234, 0666 | IPC_CREAT); // 如果消息队列1234不存在，创建并打开它
	//msgid = msgget(1234, 0666 | IPC_CREAT | IPC_EXCL); // 如果消息队列1234已经存在，这样会导致EEXIST错误
	//msgid = msgget(IPC_PRIVATE, 0666);	//宏IPC_PRIVATE实际上等于0，每调用一次，都会创建一个新的消息队列，无论之前该消息队列是否存在	
						// 通常，使用IPC_PRIVATE创建的消息队列是不能被其他进程共享的（父子/亲缘进程除外），如果要共享，就必须将对应的msgid传送给其他进程
	
	// msgid = msgget(1234, 0400);		// 权限合法
	// msgid = msgget(1234, 0777);		// 权限不合法，出现EACCES错误
	// msgid = msgget(1234, 0);		// 使用原来的访问权限打开

	msgid = msgget(1234, 0666 | IPC_CREAT);
	if(msgid == -1)
		ERR_EXIT("msgget");
	
	printf("msgget succ\n");
	printf("msgid = %d\n", msgid);
	return 0;
}

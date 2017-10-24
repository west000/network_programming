#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m)			\
	do {				\
		perror(m);		\
		exit(EXIT_FAILURE);	\
	}while(0)

typedef struct stu
{
	char name[32];
	int age;
}STU;

int main(int argc, char *argv[])
{
	int shmid;
	shmid = shmget(1234, sizeof(STU), IPC_CREAT | 0666);	// 创建共享内存段
	if(shmid == -1)
		ERR_EXIT("shmget");	 

	STU *p;
	p = shmat(shmid, NULL, 0);	// 将共享内存连接到当前进程的地址空间
	if(p == (void*)-1)
		ERR_EXIT("shmat");

	// 往共享内存中写
	strcpy(p->name, "test");
	p->age = 20;

	// 实验：假设对等方从共享内存段中读取后，往前4个字节中写入quit
	//       这里读取到quit之后，删除共享内存段
	while(1)
	{
		if(memcmp(p, "quit", 4) == 0)
			break;
	}
	shmdt(p);	// 解除连接	
	shmctl(shmid, IPC_RMID, 0);
	return 0;
}

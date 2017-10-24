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
	shmid = shmget(1234, 0, 0);	// 打开共享内存
	if(shmid == -1)
		ERR_EXIT("shmget");	 

	STU *p;
	p = shmat(shmid, NULL, 0);	// 将共享内存连接到当前进程的地址空间
	if(p == (void*)-1)
		ERR_EXIT("shmat");

	// 从共享内存中读
	printf("name = %s age = %d\n", p->name, p->age);	
	memcpy(p, "quit", 4);		// 前四个字节置为quit
	
	shmdt(p);	// 解除连接	
	return 0;
}

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m)			\
	do {				\
		perror(m);		\
		exit(EXIT_FAILURE);	\
	}while(0)

// semctl的第四个参数，是个联合体，需要手工给出
union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                    (Linux-specific) */
};

// 创建信号量集的封装，这个集合里面只有一个信号量
int sem_create(key_t key)
{
	int semid;
	semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
	if(semid == -1)
		ERR_EXIT("semget");

	return semid;
}

// 打开一个已存在的信号量集
int sem_open(key_t key)
{
	int semid;
	semid = semget(key, 0, 0);
	if(semid == -1)
		ERR_EXIT("semget");

	return semid;
}

// 设置信号集中信号量的值
int sem_setval(int semid,  int val)
{
	union semun su;
	su.val = val;
	int ret;
	ret = semctl(semid, 0, SETVAL, su);	// 对第0个信号量进行设置
	if(ret == -1)
		ERR_EXIT("sem_setval");

	return ret;
}

// 获取信号集中信号量的值
int sem_getval(int semid)
{
	int ret;
	ret = semctl(semid, 0, GETVAL, 0);	// 获取第0个信号量
	if(ret == -1)
		ERR_EXIT("sem_getval");
	return ret;
}

// 删除信号量集，注意：无法删除信号集中的一个信号量，只能将信号集整个删除
int sem_d(int semid)
{
	int ret;
	ret = semctl(semid, 0, IPC_RMID, 0);
	if(ret == -1)
		ERR_EXIT("sem_d");

	return ret;
}

// P操作
int sem_p(int semid)
{
	struct sembuf sb = {0, -1, 0};	// 对第0个信号量，做-1操作，第三个参数不关心
	int ret;
	ret = semop(semid, &sb, 1);	
	if(ret == -1)
		ERR_EXIT("semop");
	
	return ret;
}

// V操作
int sem_v(int semid)
{
	struct sembuf sb = {0, 1, 0};
	int ret;
	ret = semop(semid, &sb, 1);	
	if(ret == -1)
		ERR_EXIT("semop");
	
	return ret;
}

int semid;

void print(char op_char)
{
	int pause_time;
	srand(getpid());	// 以进程号作为随机终止
	int i;
	for(i=0; i<10; i++)
	{
		sem_p(semid);
		printf("%c", op_char);		// 打印
		fflush(stdout);			// 立即刷新到标准输出
		pause_time = rand() % 3;	
		sleep(pause_time);		// 睡眠一段随机时间
		printf("%c", op_char);		// 再次打印
		fflush(stdout);			// 这里也要立即刷新到标准输出
		sem_v(semid);	

		pause_time = rand() % 2;	
		sleep(pause_time);	
	}
}

int main(int argc, char *argv[])
{
	semid = sem_create(IPC_PRIVATE); // 本例使用信号量在父子进程间实现互斥，因此可以使用私有的信号量集
	sem_setval(semid, 0);	// 初始值设置为0
	
	pid_t pid;
	pid = fork();
	if(pid == -1)
		ERR_EXIT("fork");

	if(pid > 0)
	{	
		sem_setval(semid, 1);	// 父进程先打印
		print('O');
		wait(NULL);	// 父进程等待子进程结束
				// 如果不怎么做，可能会产生父进程已经结束了，而子进程未结束，会在shell主机名显示之后，打印剩余内容
		sem_d(semid);
	}	
	else
	{
		print('X');
	}
	return 0;
}

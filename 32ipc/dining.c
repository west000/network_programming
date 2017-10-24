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

#define DELAY ((rand()%5)+1)
int semid;

void wait_for_2fork(int no)
{
	int left = no;
	int right = (no+1)%5;
	
	struct sembuf buf[2] = {
		{left, -1, 0},
		{right, -1, 0}
	};

	semop(semid, buf, 2);	// 一次性拿两个叉子，只要有一个拿不到就等待
				// 这样，要么都拿到，要么都拿不到，这样就不会产生死锁

	// 如果每次只拿一个叉子，则可能会发生死锁
	// 如下所示
	// struct sembuf buf_left = {left, -1, 0};
	// semop(semid, &buf_left, 1);
	// struct sembuf buf_right = {right, -1, 0};
	// semop(semid, &buf_right, 1);
	//
	// 由于上面分开拿两个叉子的时间非常短暂，因此产生死锁的概率比较小
	// 为了让死锁发生的概率大一些，可以在两次拿叉子之间sleep一段时间
	// 如下所示
	// struct sembuf buf_left = {left, -1, 0};
	// semop(semid, &buf_left, 1);
	// sleep(DELAY);
	// struct sembuf buf_right = {right, -1, 0};
	// semop(semid, &buf_right, 1);
}

void free_2fork(int no)
{
	int left = no;
	int right = (no+1)%5;
	
	struct sembuf buf[2] = {
		{left, 1, 0},
		{right, 1, 0}
	};

	semop(semid, buf, 2);
}

void philosophere(int no)
{
	srand(getpid());
	for(;;)
	{
		printf("%d is thinking\n", no);
		sleep(DELAY);
		printf("%d is hungry\n", no);
		wait_for_2fork(no);
		printf("%d is eating\n", no);
		sleep(DELAY);
		free_2fork(no);
	}
}

int main(int argc, char *argv[])
{
	semid = semget(IPC_PRIVATE, 5, IPC_CREAT | 0666);
	if(semid == -1)
		ERR_EXIT("semget");
	
	union semun su;
	su.val = 1;
	int i;
	for(i=0; i<5; i++)
	{
		semctl(semid, i, SETVAL, su);
	}

	int no = 0;
	pid_t pid;
	for(i=1; i<5; i++)
	{
		pid = fork();
		if(pid == -1)
			ERR_EXIT("fork");
		
		if(pid == 0)
		{
			no = i;	
			break;	// 子进程设置no之后，跳出循环！
		}
	}	

	philosophere(no);
	return 0;
}

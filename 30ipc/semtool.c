#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>

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

	printf("key=0x%x, semid=%d\n", key, semid);
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

	printf("value updated...\n");
	return ret;
}

// 获取信号集中信号量的值
int sem_getval(int semid)
{
	int ret;
	ret = semctl(semid, 0, GETVAL, 0);	// 获取第0个信号量
	if(ret == -1)
		ERR_EXIT("sem_getval");
	
	printf("current val is %d\n", ret);
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
	// struct sembuf sb = {0, -1, 0};  // 对第0个信号量，做-1操作，第三个参数不关心
	// struct sembuf sb = {0, -1, IPC_NOWAIT};  // 计数值<=0时，报错，返回-1，errno=EAGAIN，Resource temporarily unavailable
	struct sembuf sb = {0, -1, SEM_UNDO};  // 进程终止时，撤销P操作
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

int sem_getmode(int semid)
{
	union semun su;
	struct semid_ds sem;
	su.buf = &sem;
	int ret = semctl(semid, 0, IPC_STAT, su);
	if(ret == -1)
		ERR_EXIT("semctl");
	
	printf("current permissions is %o\n", su.buf->sem_perm.mode);
	return ret;
}

int sem_setmode(int semid, char *mode)
{
	union semun su;
	struct semid_ds sem;
	su.buf = &sem;
	
	// 先获取
	int ret = semctl(semid, 0, IPC_STAT, su);
	if(ret == -1)
		ERR_EXIT("semctl");
	
	printf("current permissions is %o\n", su.buf->sem_perm.mode);
	sscanf(mode, "%o", (unsigned int*)&su.buf->sem_perm.mode);
	
	// 再设置
	ret = semctl(semid, 0, IPC_SET, su);
	if(ret == -1)
		ERR_EXIT("semctl");

	printf("permissions updated...\n");

	return ret;
}

void usage(void)
{
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "semtool -c\n");
	fprintf(stderr, "semtool -d\n");
	fprintf(stderr, "semtool -p\n");
	fprintf(stderr, "semtool -v\n");
	fprintf(stderr, "semtool -s <val>\n");
	fprintf(stderr, "semtool -g\n");
	fprintf(stderr, "semtool -f\n");
	fprintf(stderr, "semtool -m <mode>\n");
}

int main(int argc, char *argv[])
{
	int opt;
	opt = getopt(argc, argv, "cdpvs:gfm:");
	if(opt == '?')
		exit(EXIT_FAILURE);

	if(opt == -1)
	{
		usage();
		exit(EXIT_FAILURE);
	}
	
	// ftok根据一个路径（该路径必须存在），和一个整数（低8位不能为0）
	// 生成一个唯一的整数值
	key_t key = ftok(".", 's');
	int semid;
	switch(opt)
	{
	case 'c':
		sem_create(key);
		break;
	case 'p':
		semid = sem_open(key);
		sem_p(semid);
		sem_getval(semid);
		break;
	case 'v':
		semid = sem_open(key);
		sem_v(semid);
		sem_getval(semid);
		break;
	case 'd':
		semid = sem_open(key);
		sem_d(semid);
		break;
	case 's':
		semid = sem_open(key);
		sem_setval(semid, atoi(optarg));
		break;
	case 'g':
		semid = sem_open(key);
		sem_getval(semid);
		break;
	case 'f':
		semid = sem_open(key);
		sem_getmode(semid);
		break;
	case 'm':
		semid = sem_open(key);
		sem_setmode(semid, argv[2]);
		break;
	}

	return 0;
}

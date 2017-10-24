#include "ipc.h"

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

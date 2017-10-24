#ifndef _IPC_H_
#define _IPC_H_

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
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

// semctl的第四个参数，是个联合体，需要手工给出
union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                    (Linux-specific) */
};

// 创建信号量集的封装，这个集合里面只有一个信号量
int sem_create(key_t key);

// 打开一个已存在的信号量集
int sem_open(key_t key);

// 设置信号集中信号量的值
int sem_setval(int semid,  int val);

// 获取信号集中信号量的值
int sem_getval(int semid);

// 删除信号量集，注意：无法删除信号集中的一个信号量，只能将信号集整个删除
int sem_d(int semid);

// P操作
int sem_p(int semid);

// V操作
int sem_v(int semid);

#endif /* _IPC_H_ */

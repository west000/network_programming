# System V 信号量
System V的信号量是由信号量集实现的

信号量集在内核中的数据结构
```c
man 2 semctl

struct semid_ds
{
    struct ipc_perm sem_perm;  /* Ownership and permissions */
    time_t          sem_otime; /* Last semop time */
    time_t          sem_ctime; /* Last change time */
    unsigned short  sem_nsems; /* No. of semaphores in set */
};
```

# 信号量集常用的函数
```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int semget(key_t key, int nsems, int semflg);
int semctl(int semid, int semnum, int cmd, ...);
int semop(int semid, struct sembuf *sops, unsigned nsops);
```

## semget
- 创建和访问一个信号集
- int semget(key_t key, int nsems, int semflg);
- @key:信号集的名字
  @nsems:信号集中信号量的个数
  @semflg:由9个权限标志构成，用法与创建文件时使用的mode模式标志一样
- return：成功返回一个非负整数，表示该信号集的标识码；失败返回-1

## semctl
- 用于控制信号集
- int semctl(int semid, int semnum, int cmd, ...);
- @semid:有semget返回的信号集标识码
  @semnum:信号集中信号量的序号
  @cmd:将要采取的动作（有三个可选值）
  @最后一个参数根据命令不同而不同
- return: 成功为0，失败为-1

主要命令如下：
命令 | 说明
---|---
SETVAL | 设置信号量集中的信号量的计数值
GETVAL | 获取信号量集中的信号量的计数值
IPC_STAT | 把semid_ds结构中的字段设置为信号集的当前关联值
IPC_SET | 在进程有足够权限的前提下，把信号集的当前关联值设置为semid_ds数据结构中给出的值
IPC_RMID | 删除信号集

## semop
- 对信号集进行操作，即P、V操作
- int semop(int semid, struct sembuf *sops, unsigned nsops);
- @semid:由semget返回的信号集标识码
  @sops:指向一个结构数组的指针
  @nsops:结构数组中信号量的个数
- return:成功为0，失败为-1

```c
struct sembuf{
	short sem_num;	// 信号量在信号集中的编号，从0开始
	short sem_op;	// -n表示P操作，等待信号量变得可用；
			// +n表示V操作，发出信号量已经变为可用
	short sem_flg;	// 两个取值：
			// IPC_NOWAIT，如果是-n操作，并且当前计数值小于n，则出错，返回-1，errno=EAGAIN
			// SEM_UNDO，当进程结束的时候，对信号量的P、V操作会被撤销
			// 不关心可填0
};
``` 

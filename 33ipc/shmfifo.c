#include "shmfifo.h"
#include <assert.h>

shmfifo_t* shmfifo_init(int key, int blksize, int blocks)
{
	shmfifo_t *fifo = (shmfifo_t*)malloc(sizeof(shmfifo_t));
	assert(fifo != NULL);
	memset(fifo, 0, sizeof(shmhead_t));

	// 打开或创建一块共享内存
	int shmid;
	shmid = shmget(key, 0, 0);	// 尝试打开共享内存
	int size = sizeof(shmhead_t) + blksize*blocks;
	if(shmid == -1)			// 共享内存不存在
	{
		// 创建共享内存
		fifo->shmid = shmget(key, size, IPC_CREAT | 0666);
		if(fifo->shmid == -1)
			ERR_EXIT("shmget");

		// 连接共享内存到当前地址空间
		fifo->p_shm = (shmhead_t*)shmat(fifo->shmid, NULL, 0);
		if(fifo->p_shm == (shmhead_t*)-1)
			ERR_EXIT("shmat");

		// 填充共享内存的头部状态信息
		fifo->p_payload = (char*)(fifo->p_shm + 1);
		fifo->p_shm->blksize = blksize;
		fifo->p_shm->blocks = blocks;
		fifo->p_shm->rd_index = 0;
		fifo->p_shm->wr_index = 0;

		// 共享内存和信号集的key可以相同，没有影响
		// 这里创建三个信号集，每个信号集只有一个信号量
		fifo->sem_mutex = sem_create(key);
		fifo->sem_full = sem_create(key+1);
		fifo->sem_empty = sem_create(key+2);

		sem_setval(fifo->sem_mutex, 1);
		sem_setval(fifo->sem_full, blocks);
		sem_setval(fifo->sem_empty, 0);
	}
	else	// 共享内存已存在
	{
		fifo->shmid = shmid;

		// 连接共享内存到当前地址空间
		fifo->p_shm = (shmhead_t*)shmat(fifo->shmid, NULL, 0);
		if(fifo->p_shm == (shmhead_t*)-1)
			ERR_EXIT("shmat");

		fifo->p_payload = (char*)(fifo->p_shm + 1);
		// 之前已经有对应的头部状态信息，因此这里不需要填充头部

		// 打开信号集
		fifo->sem_mutex = sem_open(key);
		fifo->sem_full = sem_open(key+1);
		fifo->sem_empty = sem_open(key+2);
	}
	
	return fifo;
}

void shmfifo_put(shmfifo_t *fifo, const void *buf)
{
	sem_p(fifo->sem_full);
	sem_p(fifo->sem_mutex);

	memcpy( fifo->p_payload + fifo->p_shm->blksize*fifo->p_shm->wr_index,
		buf,
		fifo->p_shm->blksize );
	fifo->p_shm->wr_index = (fifo->p_shm->wr_index + 1) % fifo->p_shm->blocks;

	sem_v(fifo->sem_mutex);
	sem_v(fifo->sem_empty);
}

void shmfifo_get(shmfifo_t *fifo, void *buf)
{
	sem_p(fifo->sem_empty);
	sem_p(fifo->sem_mutex);

	memcpy(	buf,
		fifo->p_payload + fifo->p_shm->blksize*fifo->p_shm->rd_index,
		fifo->p_shm->blksize);
	fifo->p_shm->rd_index = (fifo->p_shm->rd_index + 1) % fifo->p_shm->blocks;	

	sem_v(fifo->sem_mutex);
	sem_v(fifo->sem_full);
}

void shmfifo_destroy(shmfifo_t *fifo)
{
	// 删除信号量
	sem_d(fifo->sem_mutex);
	sem_d(fifo->sem_full);
	sem_d(fifo->sem_empty);

	// 删除共享内存
	shmdt(fifo->p_shm);
	shmctl(fifo->shmid, IPC_RMID, 0);
	free(fifo);	
}

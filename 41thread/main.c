#include "threadpool.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// 实际执行任务的工作函数
void* mytask(void *arg)
{
	int num = *(int*)arg;
	free(arg);
	printf("task %d is working on thread 0x%x\n", num, (unsigned int)pthread_self());
	sleep(1);	// 模拟任务执行耗时
	return NULL;
}

int main(void)
{
	threadpool_t pool;
	threadpool_init(&pool, 3);
	
	int i;
	for(i=0; i<10; i++)
	{
		int *arg = (int *)malloc(sizeof(int));
		*arg = i;
		threadpool_add_task(&pool, mytask, arg);
	}

	threadpool_destroy(&pool);
	return 0;
}

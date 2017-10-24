/* 使用条件变量解决生产者和消费者问题
 * 这里的缓冲区是无界的
 * 因而采用条件变量是一种比较理想的方案
 * */
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m)			\
	do {				\
		perror(m);		\
		exit(EXIT_FAILURE);	\
	} while(0)

#define CONSUMERS_COUNT 2
#define PRODUCERS_COUNT 4

pthread_mutex_t g_mutex;
pthread_cond_t g_cond;

// 生产者和消费者的线程ID
pthread_t g_thread[CONSUMERS_COUNT+PRODUCERS_COUNT];

// 当前产品的个数
int nready = 0;

// 消费者线程入口函数
void* consume(void *arg)
{
	int num = *((int*)arg);
	free(arg);
	while(1)
	{
		pthread_mutex_lock(&g_mutex);
		// 这里用while而不用if的原因是：
		// pthread_cond_wait可能会被虚假地唤醒（即不是因为条件变为真，收到通知而返回）
		// 这里使用循环能够在pthread_cond_wait返回后判断条件是否变为真
		// 非真即是pthread_cond_wait被虚假唤醒，需要继续调用pthread_cond_wait
		while(nready == 0)	
		{
			printf("%d begin wait a condition ...\n", num);
			// pthread_cond_wait完成了三件事
			// 1. 对g_mutex解锁
			// 2. 等待条件，直到有线程向它发起通知
			// 3. 重新对g_mutex加锁
			pthread_cond_wait(&g_cond, &g_mutex);
			printf("%d end wait a condition ...\n", num);
		}
		printf("%d begin consume product ...\n", num);
		--nready;	// 消费产品：这里是简化版，并没有对缓冲区进行操作
		printf("%d end consume product ...\n", num);
		pthread_mutex_unlock(&g_mutex);
		sleep(1);
	}
	return NULL;
}

// 生产者线程入口函数
void* produce(void *arg)
{
	int num = *((int*)arg);
	free(arg);
	while(1)
	{
		pthread_mutex_lock(&g_mutex);
		printf("%d begin produce product ... _\n", num);
		++nready;
		printf("%d end produce product ...   _|\n", num);
		if(nready > 0)	// 这里是比较谨慎的操作，其实可以不用判断nready>0，因为nready的初始值为0，在临界区中经过自增操作之后，肯定大于0
			printf("%d signal ...\n", num);
		// 向第一个等待的线程发起通知
		pthread_cond_signal(&g_cond);
		pthread_mutex_unlock(&g_mutex);
		sleep(1);
	}
	return NULL;
}

int main(void)
{
	int i;

	pthread_mutex_init(&g_mutex, NULL);
	pthread_cond_init(&g_cond, NULL);	

	// 创建生产者和消费者线程
	int *p;
	for(i=0; i<CONSUMERS_COUNT; i++)
	{
		p = (int*)malloc(sizeof(int));
		*p = i;
		pthread_create(&g_thread[i], NULL, consume, p);
	}

	sleep(1);
	
	for(i=0; i<PRODUCERS_COUNT; i++)
	{
		p = (int*)malloc(sizeof(int));
		*p = i;
		pthread_create(&g_thread[i+CONSUMERS_COUNT], NULL, produce, p);
	}

	// 等待生产者和消费者线程结束
	for(i=0; i<CONSUMERS_COUNT+PRODUCERS_COUNT; i++)
		pthread_join(g_thread[i], NULL);

	// 销毁信号量和互斥锁
	pthread_mutex_destroy(&g_mutex); 
	pthread_cond_destroy(&g_cond);	
	return 0;
}

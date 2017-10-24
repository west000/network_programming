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

#define CONSUMERS_COUNT 5 
#define PRODUCERS_COUNT 1
#define BUFFSIZE 10

int g_buffer[BUFFSIZE];

unsigned int in = 0;
unsigned int out = 0;
unsigned int produce_id = 0;	// 生产产品的ID
unsigned int consume_id = 0;	// 消费产品的ID

sem_t g_sem_full;
sem_t g_sem_empty;
pthread_mutex_t g_mutex;

// 生产者和消费者的线程ID
pthread_t g_thread[CONSUMERS_COUNT+PRODUCERS_COUNT];

// 消费者线程入口函数
void* consume(void *arg)
{
	int num = *((int*)arg);
	free(arg);
	int i;
	while(1)
	{
		printf("%d wait buffer not empty\n", num);	
		sem_wait(&g_sem_empty);
		pthread_mutex_lock(&g_mutex);

		// 在消费产品之前，先打印仓库的状态
		for(i=0; i<BUFFSIZE; i++)
		{
			printf("%02d ", i);
			if(g_buffer[i] == -1)		// 当前仓库是否为空
				printf("null");
			else
				printf("%d", g_buffer[i]);

			if(i == out)			// 要消费的仓库
				printf("\t<--consume");

			printf("\n");
		}

		// 消费产品
		consume_id = g_buffer[out];		// 取出产品
		printf("%d begin consume product %d\n", num, consume_id);
		g_buffer[out] = -1;
		out = (out + 1) % BUFFSIZE;
		printf("%d end consume product %d\n", num, consume_id);

		pthread_mutex_unlock(&g_mutex);
		sem_post(&g_sem_full);
		sleep(5);
	}
	return NULL;
}

// 生产者线程入口函数
void* produce(void *arg)
{
	int num = *((int*)arg);
	free(arg);
	int i;
	while(1)
	{
		printf("%d wait buffer not full\n", num);
		sem_wait(&g_sem_full);
		pthread_mutex_lock(&g_mutex);

		// 在生产产品之前，先打印仓库的状态
		for(i=0; i<BUFFSIZE; i++)
		{
			printf("%02d ", i);
			if(g_buffer[i] == -1)		// 当前仓库是否为空
				printf("null");
			else
				printf("%d", g_buffer[i]);

			if(i == in)			// 要生产的仓库
				printf("\t<--produce");

			printf("\n");
		}

		// 生产产品
		printf("%d begin produce product %d\n", num, produce_id);
		g_buffer[in] = produce_id;
		in = (in + 1) % BUFFSIZE;
		printf("%d end produce product %d\n", num, produce_id++);

		pthread_mutex_unlock(&g_mutex);
		sem_post(&g_sem_empty);
		sleep(1);
	}
	return NULL;
}

int main(void)
{
	int i;
	// 初始化仓库
	for(i=0; i<BUFFSIZE; i++)
		g_buffer[i] = -1;

	// 初始化一个无名的POSIX信号量，只用于当前进程的多个线程间的通信
	sem_init(&g_sem_full, 0, BUFFSIZE);
	sem_init(&g_sem_empty, 0, 0);

	// 初始化一个无名的互斥锁，只用于当前进程的多个线程间的通信
	// 采用默认属性，attr=NULL
	pthread_mutex_init(&g_mutex, NULL);

	// 创建生产者和消费者线程
	int *p;
	for(i=0; i<CONSUMERS_COUNT; i++)
	{
		p = (int*)malloc(sizeof(int));
		*p = i;
		pthread_create(&g_thread[i], NULL, consume, p);
	}
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
	sem_destroy(&g_sem_full);
	sem_destroy(&g_sem_empty);
	pthread_mutex_destroy(&g_mutex); 
	
	return 0;
}

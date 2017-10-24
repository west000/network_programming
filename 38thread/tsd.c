/*这个程序用来演示如何使用线程特定数据，不进行错误处理*/
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m)			\
	do {				\
		perror(m);		\
		exit(EXIT_FAILURE);	\
	} while(0)

typedef struct tsd
{
	pthread_t tid;
	char *str;
}tsd_t;

pthread_key_t key_tsd;	// 一个全局的key，相当于每个线程都有这样一个全局变量
			// 指向每个线程特有的实际数据
pthread_once_t once_control = PTHREAD_ONCE_INIT;

// 销毁key指向的特定数据
void destroy_routine(void *value)
{
	printf("destroy ...\n");
	// 释放实际的数据
	free(value);
}

// 仅被调用一次的初始化函数
void once_routine(void)
{
	// 创建一个与线程关联的全局key
	pthread_key_create(&key_tsd, destroy_routine);
	printf("create key ...\n");
}

void* thread_routine(void *arg)
{
	// pthread_once指定once_routine仅被调用一次
	// 由第一个进入的线程调用
	pthread_once(&once_control, once_routine);
	tsd_t *value = (tsd_t*)malloc(sizeof(tsd_t));
	value->tid = pthread_self();
	value->str = (char*)arg;

	// 给线程设定特定数据，由value指向
	pthread_setspecific(key_tsd, value);
	printf("%s setspecific %p\n", (char*)arg, value);

	// 获取key_tsd指向的特定数据
	value = pthread_getspecific(key_tsd);
	printf("tid=0x%x str=%s\n", (unsigned int)value->tid, value->str);
	sleep(2);
	value = pthread_getspecific(key_tsd);
	printf("tid=0x%x str=%s\n", (unsigned int)value->tid, value->str);
	return NULL;
}

int main(void)
{
	// 在主线程中，创建一个线程特定数据的key
	// pthread_key_create(&key_tsd, destroy_routine);

	pthread_t tid1;
	pthread_t tid2;
	pthread_create(&tid1, NULL, thread_routine, "thread1");
	pthread_create(&tid2, NULL, thread_routine, "thread2");
	
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);

	// 销毁key，实际上是调用destroy_routine函数进行销毁
	// 由于上面创建两个线程，并且每个线程都用key_tsd指定各自的特定数据
	// 因此有两个全局私有变量，destroy_routine会被调用两次
	pthread_key_delete(key_tsd);
	return 0;
}

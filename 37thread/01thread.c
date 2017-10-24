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

void* thread_routine(void *arg)
{
	int i;
	for(i=0; i<20; i++)
	{
		printf("B");
		fflush(stdout);
		usleep(20);
/*
		if(i==3)
			pthread_exit("ABC");
*/
	}
	
	// 在这里故意睡眠3秒钟，主线程任然需要等待对等线程结束后才退出
	sleep(3);
	return "DEF";	
}

int main(void)
{
	pthread_t tid;
	int ret;	
	if((ret = pthread_create(&tid, NULL, thread_routine, NULL)) != 0)
	{
		// 注意这里的错误处理
		// 不采用ERR_EXIT，因为perror检查的是全局的errno变量，开销比通过返回值判定错误要大
		fprintf(stderr, "pthread_create:%s\n", strerror(ret));
		exit(EXIT_FAILURE);
	} 

	int i;
	for(i=0; i<20; i++)
	{
		printf("A");
		fflush(stdout);
		usleep(20);
	}
	
	void *value;
	// 等待创建的线程结束
	if((ret = pthread_join(tid, &value)) != 0)
	{
		fprintf(stderr, "pthread_create:%s\n", strerror(ret));
		exit(EXIT_FAILURE);
	}
	printf("\n");
	printf("return msg=%s\n", (char*)value);
	return 0;
}

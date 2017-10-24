#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include <stdio.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* doit(void *arg)
{
	printf("pid = %d begin doit ...\n", (int)getpid());
	pthread_mutex_lock(&mutex);
	struct timespec ts = {2, 0};
	nanosleep(&ts, NULL);
	pthread_mutex_unlock(&mutex);
	printf("pid = %d end doit ...\n", (int)getpid());
	
	return NULL;
}

int main(void)
{
	printf("pid = %d Entering main ...\n", (int)getpid());
	pthread_t tid;
	// 父进程在此处创建一个对等线程，线程入口函数为doit
	pthread_create(&tid, NULL, doit, NULL);

	struct timespec ts = {1, 0};
	nanosleep(&ts, NULL);		// 等待1秒钟，等待期间对等线程能够运行（对mutex加锁），这样在fork只有，子进程等到的mutex也是上锁状态

	// 父进程在其主线程中，调用fork创建一个子进程
	if(fork() == 0)
	{
		// 子进程中，只继承父进程的一个执行序列（调用fork的主线程）, 而继承另外一个对等线程
		// 注意：子进程会复制父进程的状态，包括mutex的状态（已上锁）
		//       这时在子进程中执行doit，会产生死锁！子进程一直阻塞在获取mutex上
		doit(NULL);
	}
	pthread_join(tid, NULL);
	printf("pid = %d Exiting main ...\n", (int)getpid());
	return 0;
}

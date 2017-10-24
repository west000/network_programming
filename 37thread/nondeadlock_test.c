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
	nanosleep(&ts, NULL);	// 等上2秒钟，保证对等线程不会在fork之前执行完毕，导致mutex解锁
	pthread_mutex_unlock(&mutex);
	printf("pid = %d end doit ...\n", (int)getpid());
	
	return NULL;
}

void prepare(void)
{
	// fork时，由于父进程创建的对等线程已经执行，且未执行完毕，这时mutex是上锁状态
	// 父进程在创建子进程之前，先对mutex解锁
	// 这样，创建的子进程，复制到的mutex就是解锁状态！
	pthread_mutex_unlock(&mutex);
}

void parent(void)
{
	// 父进程成功创建子进程之后，由于对等线程还未执行完毕，需要重新对mutex加锁
	// 这样，对等线程的pthread_mutex_unlock才不会出错
	pthread_mutex_lock(&mutex);
}

int main(void)
{
	pthread_atfork(&prepare, &parent, NULL);
	printf("pid = %d Entering main ...\n", (int)getpid());
	pthread_t tid;
	pthread_create(&tid, NULL, doit, NULL);
	struct timespec ts = {1, 0};
	nanosleep(&ts, NULL);	// 等上1秒钟，保证对等线程能够执行，对mutex上锁
	if(fork() == 0)
	{
		doit(NULL);
	}
	pthread_join(tid, NULL);
	printf("pid = %d Exiting main ...\n", (int)getpid());
	return 0;
}

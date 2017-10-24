/*为了节约开发时间，此处不做过多的错误处理*/
#include "threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// 从任务队列中取出任务，并执行任务（消费者）
void* thread_routine(void *arg)
{
	struct timespec abstime;
	int timeout;
	printf("thread 0x%x is starting\n", (unsigned int)pthread_self());
	threadpool_t *pool = (threadpool_t *)arg;
	while(1)
	{
		timeout = 0;	// 超时标志设置为0

		condition_lock(&pool->ready);		// <-----------进入临界区
		
		// 消费者线程能够获取到互斥锁，表明有新的空闲线程
		pool->idle++;	

		// 等待队列中有任务到来，或者线程池销毁的通知
		while(pool->first == NULL && !pool->quit)
		{
			printf("thread 0x%x is waiting\n", (unsigned int)pthread_self());
			// 为了实现线程的动态销毁，即等待一段时间后，没有任务到达，自动销毁
			// 这里采用了pthread_cond_timedwait函数

			clock_gettime(CLOCK_REALTIME, &abstime);
			abstime.tv_sec += 2;	// 等待两秒钟
			int status = condition_timedwait(&pool->ready, &abstime);
			if(status == ETIMEDOUT)
			{
				printf("thread 0x%x wait timed out\n", (unsigned int)pthread_self());
				timeout = 1;
				break;
			}

			// condition_wait(&pool->ready);
			// 这里有个比较隐晦的点：
			// pthread_cond_wait完成了三件事：
			// 对互斥锁解锁 -> 等待条件 -> 重新对互斥锁加锁
			// 可见，当条件满足，跳出循环时，互斥锁是上锁的！
		}

		// 当跳出上面循环时
		// 线程要么去执行任务（等到新任务），要么退出并自动销毁（等到线程池销毁通知，或等到超时）
		// 因此减少一个空闲线程
		pool->idle--;

		// 注意：有可能两个条件同时满足！！
		// 等待到任务到来
		if(pool->first != NULL)
		{
			// 从队头取出任务
			task_t *t = pool->first;
			pool->first = t->next;
			if(t->next == NULL)
				pool->last = NULL;

			// 运行任务
			// 注意：由于任务在临界区中运行，执行任务需要一段时间（主因）
			//       且执行任务时不对临界资源(threadpool的字段)进行操作（次因）
			//       因此在运行任务前需要对互斥锁进行解锁
			//
			//       如果不解锁，那么其他消费者线程就无法进入临界区等待任务
			//       并且生产者线程无法进入临界区添加任务
			condition_unlock(&pool->ready);	// -----------> 执行任务前解锁
			t->run(t->arg);
			free(t);
			condition_lock(&pool->ready);	// <----------- 任务退出后重新加锁
		}

		// 等待到线程池销毁通知（由销毁线程池函数设置quit=1，然后发出通知），且任务都执行完毕
		// 该线程退出，自动销毁
		if(pool->quit && pool->first == NULL)
		{
			pool->counter--;
			// 当前线程数为0，向threadpool_destroy发起通知，
			// 使其跳出`while(pool->counter > 0) condition_wait(pool->ready);`循环
			// 从而使调用threadpool_destroy的线程(主线程)能够结束
			if(pool->counter == 0)
				condition_signal(&pool->ready);
			
			// 跳出循环之前，要记得解锁
			condition_unlock(&pool->ready);
			break;
		}

		// 线程等待任务超时，且任务都执行完毕
		// 该线程退出，自动销毁
		if(timeout && pool->first == NULL)
		{
			pool->counter--;	// 线程数减1
			// 跳出循环之前，要记得解锁
			condition_unlock(&pool->ready);
			break;
		}

		condition_unlock(&pool->ready);		// ----------->退出临界区
	}

	printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
	return NULL;
}

// 初始化线程池
void threadpool_init(threadpool_t *pool,  int threads)
{
	// 对线程中的各个字段进行初始化
	condition_init(&pool->ready);
	pool->first = NULL;
	pool->last = NULL;
	pool->counter = 0;
	pool->idle = 0;
	pool->max_threads = threads;
	pool->quit = 0;	
}

// 往线程池中添加任务（生产者）
void threadpool_add_task(threadpool_t *pool, void *(*run)(void *arg), void *arg)
{
	// 生成新任务
	task_t *newtask = (task_t *)malloc(sizeof(task_t));
	newtask->run = run;
	newtask->arg = arg;
	newtask->next = NULL;	

	/* *
	 * 对threadpool的字段进行操作，都属于临界区
	 * */
	condition_lock(&pool->ready);		// <-----------进入临界区

	// 将任务添加到队列中
	if(pool->first == NULL)	// 往空队列添加任务
		pool->first = newtask;
	else 
		pool->last->next = newtask;
	pool->last = newtask;

	// 添加任务之后
	if(pool->idle > 0)
	{
		// 如果有等待线程，则唤醒其中一个，让其执行任务
		condition_signal(&pool->ready);	
	}
	else if(pool->counter < pool->max_threads)
	{
		// 没有等待线程，并且当前线程数不超过最大线程数，则创建一个新线程
		pthread_t tid;
		pthread_create(&tid, NULL, thread_routine, pool);
		pool->counter++;
	}
		// 否则，仅仅将任务插入队列中，等消费者线程有空闲时自己从队列中获取任务
		// 可见，前面两种是主动地让消费者线程处理任务
		// 而这里是被动地让消费者线程处理任务

	condition_unlock(&pool->ready);		// ----------->退出临界区
}

// 销毁线程池
void threadpool_destroy(threadpool_t *pool)
{
	if(pool->quit)
		return;
	
	condition_lock(&pool->ready);
	pool->quit = 1;		// 设置销毁线程池标志
	if(pool->counter > 0)
	{
		// 有等待线程，则向它们广播通知quit已被设置为1
		if(pool->idle > 0)
			condition_broadcast(&pool->ready);
		
		// 处于执行任务状态中的线程，不会收到广播
		// 这里需要等待`处于执行任务状态的线程`全部退出
		while(pool->counter > 0)
			condition_wait(&pool->ready);
	}
	condition_unlock(&pool->ready);
	condition_destroy(&pool->ready);
}
 

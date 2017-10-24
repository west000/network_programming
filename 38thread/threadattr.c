/*这个程序只是测试一些线程属性函数，不做错误处理*/
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

int main(void)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	// 分离状态
	int state;
	pthread_attr_getdetachstate(&attr, &state);
	if(state == PTHREAD_CREATE_JOINABLE)
		printf("detachstate: PTHREAD_CREATE_JOINABLE\n");
	else if(state == PTHREAD_CREATE_DETACHED)
		printf("detachstate: PTHREAD_CREATE_DETACHED\n");
	
	// 栈大小
	size_t size;
	pthread_attr_getstacksize(&attr, &size);
	printf("stacksize: %zu\n", size);

	// 栈溢出保护区大小
	pthread_attr_getguardsize(&attr, &size);	
	printf("guardsize: %zu\n", size);

	// 线程竞争范围
	int scope;
	pthread_attr_getscope(&attr, &scope);
	if(scope == PTHREAD_SCOPE_PROCESS)	// 进程范围内的竞争 
		printf("scope: PTHREAD_SCOPE_PROCESS\n");
	else if(scope == PTHREAD_SCOPE_SYSTEM)	// 系统范围内的竞争
		printf("scope: PTHREAD_SCOPE_SYSTEM\n");

	// 调度策略
	int policy;
	pthread_attr_getschedpolicy(&attr, &policy);
	if(policy == SCHED_FIFO)	// 如果线程具有相同优先级，按先进先出原则进行调度
		printf("policy: SCHED_FIFO\n");
	else if(policy == SCHED_RR)	// 轮转，基于时间片的调度
		printf("policy: SCHED_RR\n");
	else if(policy == SCHED_OTHER)	// 其他情况
		printf("policy: SCHED_OTHER\n");

	// 继承调度策略：是否继承调用者线程的属性
	int inheritsched;
	pthread_attr_getinheritsched(&attr, &inheritsched);
	if(inheritsched == PTHREAD_INHERIT_SCHED)	// 继承调用者线程的属性
		printf("inheritsched: PTHREAD_INHERIT_SCHED\n");
	else if(inheritsched == PTHREAD_EXPLICIT_SCHED)	// 需要自己显示设置新线程的尚需经
		printf("inheritsched: PTHREAD_EXPLICIT_SCHED\n");

	// 调度参数：通常只关注调度的优先级
	struct sched_param param;
	pthread_attr_getschedparam(&attr, &param);
	printf("sched priority: %d\n", param.sched_priority);
	
	// 销毁线程属性变量
	pthread_attr_destroy(&attr);

	// 
	int level;
	level = pthread_getconcurrency();
	printf("level: %d\n", level);	

	return 0;
}

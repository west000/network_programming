#ifndef _CONDITION_H_
#define _CONDITION_H_

#include <pthread.h>

// 条件变量结构体
// 由于条件变量总是与一个互斥锁一起使用，因此该结构体包含了mutex和cond
typedef struct condition
{
	pthread_mutex_t pmutex;
	pthread_cond_t	pcond;
} condition_t;

int condition_init(condition_t *cond);
int condition_lock(condition_t *cond);
int condition_unlock(condition_t *cond);
int condition_wait(condition_t *cond);
int condition_timedwait(condition_t *cond, const struct timespec *abstime);
int condition_signal(condition_t *cond);
int condition_broadcast(condition_t *cond);
int condition_destroy(condition_t *cond);

#endif /* _CONDITION_H_  */

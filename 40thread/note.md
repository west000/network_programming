# POSIX条件变量


# 条件变量常用的函数
```c
#include <pthread.h>

int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr);
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// 等待条件变为真
int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex);
// 向第一个等待线程发起通知
int pthread_cond_signal(pthread_cond_t *cond);
// 向所有等待线程发起通知		  
int pthread_cond_broadcast(pthread_cond_t *cond);
```

# 条件变量的使用规范
- 条件变量需要配合互斥锁使用，因为一个条件变量通常被多个线程访问，所以要对条件变量进行保护

## 等待条件代码
```c
pthread_mutex_lock(&mutex);
while(条件为假)	----------------------> 这里的条件是个临界区，因此需要用到互斥锁
    pthread_cond_wait(cond, mutex);
修改条件
pthread_mutex_unlock(&mutex);
```

pthread_cond_wait实际上完成了三件事：
- 对mutex进行解锁：a.为了能够让其他线程有机会进入临界区，修改条件使其为真，因此需要解锁；b.使得其他线程有机会进入该临界区，等待同一条件（比如多个消费者线程进入临界区，等待缓冲区非空）
- 等待条件，直到有线程向它发起通知
- 重新对mutex进行加锁：这样才能与下面的pthread_mutex_unlock匹配

注意点：
- 在调用pthread_cond_wait之前需要对mutex进行加锁，否则pthread_cond_wait调用失败
- 上面使用while而不是if，原因是pthread_cond_wait可能被虚假地唤醒（即函数返回，并不是条件变为真而收到其他线程的信号后返回），因此需要在pthread_cond_wait返回的时候，通过循环判断条件是否为假
- pthread_cond_wait可能会被信号中断，这些信号分为两类：a. 信号中断pthread_cond_wait后，pthread_cond_wait能够自动重启，就好像中断没有发生过一样；b. 信号中断导致虚假唤醒pthread_cond_wait


## 给条件发送信号代码
```c
pthread_mutex_lock(&mutex);
设置条件为真
pthread_cond_signal(cond);
pthread_mutex_unlock(&mutex);
```

pthread_cond_signal完成的事：
- 向第一个等待条件的线程发起通知
- 如果没有任何一个线程处于等待条件的状态，这个通知将被忽略

# 使用条件变量解决生产者消费者问题
- 使用条件变量实现无界缓冲区是比较理想的方案


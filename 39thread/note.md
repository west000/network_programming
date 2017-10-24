# POSIX信号量和互斥锁

## 有名信号量
```c
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

sem_t *sem_open(const char *name, int oflag);
sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
int sem_close(sem_t *sem);
int sem_unlink(const char *name);
int sem_wait(sem_t *sem);
int sem_post(sem_t *sem);
```

## 无名信号量
注意点：
- 无名信号量依然可以用于不同进程的不同线程之间的通信！
- 有两个前提：sem要存放在共享内存中；pshared不能为0

```c
#include <semaphore.h>

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_post(sem_t *sem);
```

## 互斥锁
注意点：
- 互斥锁是无名的，但是它也能应用于不同进程的多个线程间的通信
- 有两个前提：mutex是否在共享内存中；并且attr需要指定互斥锁为共享的
```c
#include <pthread.h>

int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

# 生产者消费者问题
```
条件: 缓冲区大小为10
      sem_full(10)      当前缓冲区的空闲数
      sem_empty(0)      当前缓冲区的产品数
      mutex             操作缓冲区的互斥锁

生产者                          消费者
P(sem_full)   -<-               P(sem_empty)  -<-
  P(mutex)      |                 P(mutex)      |
  生产产品      |                 消费产品      |
  V(mutex)      |                 V(mutex)      |
V(sem_empty)  ->-               V(sem_full)   ->-

由上可见，sem_full和sem_empty是用来实现同步的；mutex是用来实现互斥的
```

# POSIX自旋锁和读写锁
## 自旋锁
- 自旋锁类似于互斥锁，它的性能比互斥锁更高
- 自旋锁和互斥锁很重要的一个区别在于：线程在申请自旋锁的时候，线程不会被挂起，它处于忙等状态（不断地申请）
- 因此自旋锁适用于等待时间比较短的应用，如实时性要求高的应用
- 不适用等待时间长的应用，因为此时CPU处于空转获取锁的状态，浪费CPU资源
```c
#include <pthread.h>

int pthread_spin_init(pthread_spinlock_t *lock, int pshared);
int pthread_spin_destroy(pthread_spinlock_t *lock);
int pthread_spin_lock(pthread_spinlock_t *lock);
int pthread_spin_trylock(pthread_spinlock_t *lock);
int pthread_spin_unlock(pthread_spinlock_t *lock);
```

## 读写锁
读写锁的规范如下：
- 只要没有线程持有给定的读写锁用于写，那么任意数目的线程可以持有读写锁用于读
- 仅当没有线程持有某个给定的读写锁用于读或用于写时，才能分配读写锁用于写
- 读写锁用于读称为共享锁，读写锁用于写称为排它锁

与互斥锁的区别：
- 读写锁控制线程的互斥更加有针对性，当一个线程施加共享锁时，还其他线程允许施加共享锁；而对于互斥锁，只要上锁，其他线程均无法进入临界区，直到线程释放互斥锁为止

```c
#include <pthread.h>

int pthread_rwlock_init(pthread_rwlock_t *restrict rwlock, const pthread_rwlockattr_t *restrict attr);
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
```


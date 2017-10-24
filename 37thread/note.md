# POSIX线程库
使用方法如下：
- 与线程相关的函数构成了一个完整的系列，绝大部分函数的名字以'pthread_'开头
- 使用这些函数，需要引入<pthread.h>
- 链接这些线程函数库时，要使用'-lpthread'选项

# 进程和线程函数的对比
对比项目 | 进程 | 线程
---|---|---
ID类型 | pid_t | thread_t
创建 | fork | pthread_create
等待子进程/创建的线程结束 | waitpid(子进程的退出状态可以通过status参数返回, int *status) | pthread_join(对等线程退出的返回内容可以通过value参数返回，void**value)
中途退出 | exit | pthread_exit
运行完退出 | 在main函数中调用return | 在线程入口函数中调用return
僵尸进程/线程 | 子进程先结束，父进程未结束，这时子进程会保留一个状态，直到父进程调用wait/waitpid，才撤销僵尸状态 | 如果新创建的线程结束了，而主线程没有调用pthread_join，这时线程就处于僵尸状态，可以使用pthread_detach函数将线程分离，这样线程结束之后交由内核回收，不会产生僵尸状态
杀死其他进程/线程 | kill | pthread_cancel

线程结束的方式：
- 自杀：调用pthread_exit；在线程入口函数中调用return
- 他杀：其他线程调用pthread_cancel
 

# 常用函数
```c
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, 
		   void *(*start_routine) (void *), void *arg);
int pthread_join(pthread_t thread, void **retval);
pthread_t pthread_self(void);
void pthread_exit(void *retval);
int pthread_cancel(pthread_t thread);
int pthread_detach(pthread_t thread);
```

## pthread_create
- 创建一个新线程
- int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
- @thread:返回线程ID
  @attr:设置线程属性，attr为NULL表示使用默认属性
  @start_routine:线程启动后要执行的函数的地址
  @arg:传给线程启动函数的参数
- return:成功返回0；失败返回错误码

多线程函数的错误处理方式：
- 传统的函数：成功返回0，失败返回-1，并且对全局变量errno赋值以指示错误
- pthreads函数：出错时，不会设置全局变量errno，而是将错误代码通过返回值返回
- pthreads同样也提供了线程内的errno变量，以支持其他使用errno的代码。对于pthreads函数的错误，建议通过返回值做判定，因为读取返回值要比读取线程内的errno变量开销更小


## pthread_join
- 等待线程结束
- int pthread_join(pthread_t thread, void **retval);
- @thread:线程ID
  @retval:指向一个指针，后者指向线程的返回值
- return:成功返回0；失败返回错误码

## pthread_self
- 返回线程ID
- pthread_t pthread_self(void);
- return:这个函数调用总是成功，返回线程ID

## pthread_exit
- 线程终止
- void pthread_exit(void *retval);
- @retval:retval不要指向一个局部变量
- return:无返回值，跟进程一样，线程结束的时候无法返回到它的调用者（自身）

## pthread_cancel
- 取消一个执行中的线程
- int pthread_cancel(pthread_t thread);
- @thread:线程ID
- return:成功返回0；失败返回错误码

## pthread_detach
- 将一个线程分离
- int pthread_detach(pthread_t thread);
- @thread:线程ID
- return:成功返回0；失败返回错误码



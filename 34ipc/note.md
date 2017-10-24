# IPC
## System V IPC
- 消息队列
- 共享内存
- 信号量

## POSIX IPC
- 消息队列
- 共享内存
- 信号量
- 互斥锁
- 条件变量
- 读写锁
- 自旋锁
- 文件锁

# POSIX消息队列

# 常用函数
```c
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>

mqd_t mq_open(const char *name, int oflag);
mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
int mq_close(mqd_t mqdes);
int mq_unlink(const char *name);
int mq_getattr(mqd_t mqdes, struct mq_attr *attr);
int mq_setattr(mqd_t mqdes, struct mq_attr *newattr, struct mq_attr *oldattr);
int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio);
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio);
int mq_notify(mqd_t mqdes, const struct sigevent *sevp);
```

## mq_open
- 打开一个消息队列
- mqd_t mq_open(const char *name, int oflag);
- @name:消息队列的名字
  @oflag:与open函数类似，可以是O_RDONLY、O_WRONLY、ORDWR，还可以按位或O_CREAT、O_EXCL、O_NONBLOCK等
- return:成功返回消息队列文件描述符；失败返回-1

- 创建或打开一个消息队列
- mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
- @name:
  @oflag:
  @mode:如果oflag指定了O_CREAT，需要设置mode
  @attr:消息队列的属性
- return:

注意点：
- 在生成可执行文件的时候，需要连接rt运行时库，即链接时加上`-lrt`
- 不同于System V IPC，POSIX IPC无法通过IPC查看
- 当成功创建一个消息队列后，该消息队列实际上存在于虚拟文件系统中，但这个文件系统需要挂载到某一个目录上才可以用
```sh
man 7 mq_overview

# mkdir /dev/mqueue	# 创建一个挂载点
# mount -t mqueue none /dev/mqueue	# 将虚拟文件系统挂载到/dev/mqueue目录下
# umount /dev/mqueue	# 将虚拟文件系统卸载
# rm -rf mqueue		# 如果没有卸载，会提示Device or resource busy
```

POSIX IPC名字限定：
- 必须以'/'开头，并且后续不能有其他'/'，形如/somename
- 长度不能超过NAME_MAX

## mq_close
- 关闭消息队列
- int mq_close(mqd_t mqdes);
- @mqdesL:mq_open返回的消息队列描述符
- return:成功为0；失败为-1

注意：
- 关闭消息队列不代表删除一个消息队列，就好像关闭一个文件不是删除一个文件

## mq_unlink
- 连接数减1，直到连接数为0时，删除消息队列
- int mq_unlink(const char *name);
- @name:消息队列的名字
- return:成功为0；失败为-1

## mq_getattr
- 获取消息队列属性
- int mq_getattr(mqd_t mqdes, struct mq_attr *attr);
- @mqdes:mq_open返回的消息队列描述符
  @attr:属性
- return:成功为0；失败为-1

## mq_setattr
- 设置消息队列属性
- int mq_setattr(mqd_t mqdes, struct mq_attr *newattr, struct mq_attr *oldattr);
- @mqdes:mq_open返回的消息队列描述符
  @newaddr:新属性
  @oldattr:旧属性
- return:成功为0；失败为-1

## mq_send
- 发送消息
- int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio);
- @mqdes:mq_open返回的消息队列描述符
  @msg_ptr:指向消息的指针
  @msg_len:消息长度
  @msg_prio:消息优先级
- return:成功为0；失败为-1

## mq_receive
- 接收消息
- ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio);
- @mqdes:mq_open返回的消息队列描述符
  @msg_ptr:返回接收到的消息
  @msg_len:消息长度，必须等于当前系统一条消息的最大长度
  @msg_prio:返回接收到的消息优先级
- return:成功返回接收到的消息字节数；失败返回-1

注意：
- 返回指定消息队列中最高优先级的最早消息
- 接收的长度(msg_len)必须等于每条消息的长度

## mq_notify
- 建立或删除消息到达通知事件
- int mq_notify(mqd_t mqdes, const struct sigevent *sevp);
- @mqdes:mq_open返回的消息队列描述符
  @notification: a.不为NULL：当消息到达且消息队列先前为空（即从空变为不空），进程将得到通知；b. 为NULL：撤销已注册的通知
- return:成功为0；失败为-1

通知方式有两种：
- 产生一个信号
- 创建一个线程执行一个指定的函数

```c
man 7 sigevent

union sigval {          /* Data passed with notification */
	int     sival_int;         /* Integer value */
	void   *sival_ptr;         /* Pointer value */
};

struct sigevent {
	int          sigev_notify; /* Notification method */
	int          sigev_signo;  /* Notification signal */
	union sigval sigev_value;  /* Data passed with notification */
	void         (*sigev_notify_function) (union sigval);
                            /* Function used for thread notification (SIGEV_THREAD) */
	void        *sigev_notify_attributes;
                            /* Attributes for notification thread (SIGEV_THREAD) */
	pid_t        sigev_notify_thread_id;
                            /* ID of thread to signal (SIGEV_THREAD_ID) */
};

```

使用mq_notify需要注意的：
- 任何时刻只能有一个进程可以被注册为接收某个给定消息队列的通知
- 当有一个消息到达某个`先前为空`的队列，并且已有一个进程被注册为接收该队列的通知，同时没有任何线程阻塞在该队列的mq_receive调用，这时通知才会发出
- 当通知被发送给它的注册进程时，其注册将被撤销。如果需要的话，进程必须再次调用mq_notify以重新注册，`重新注册要放在读取消息之前`，而不能在之后，否则可能在mq_receive 和 mq_notify之间，有消息到达，导致消息队列不为空，从而无法再次通知


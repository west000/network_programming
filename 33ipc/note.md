# 将共享内存和信号量结合
- 用信号量解决生产者消费者问题
- 用信号量和共享内存，实现一个先进先出的共享环形缓冲区shmfifo

# 生产者消费者问题

生产者和消费者实质是一个`同步与互斥`相结合的例子
- 同步：缓冲区为空，生产者才能往里面生成产品；缓冲区不为空，消费者才可以从里面获取产品
- 互斥：生产者和消费者都有多个，同时对缓冲区的操作，需要实现生产者间的互斥，消费者间的互斥，以及生产者-消费者间的互斥

```
条件: 缓冲区大小为10
      sem_full(10)	当前缓冲区的空闲数
      sem_empty(0)      当前缓冲区的产品数
      sem_mutex(1)	操作缓冲区的互斥锁

生产者                          消费者
P(sem_full)   -<-               P(sem_empty)  -<-
  P(sem_mutex)  |                 P(sem_mutex)  |
  生产产品      |                 消费产品      |
  V(sem_mutex)  |                 V(sem_mutex)  |
V(sem_empty)  ->-               V(sem_full)   ->-

由上可见，sem_full和sem_empty是用来实现同步的；sem_mutex是用来实现互斥的
```

# 先进先出的共享环形缓冲区shmfifo
开辟一块定长的共享内存，分为两部分：
- 头部：存放当前缓冲区的使用状态(p_shm指向)
- 负载：实际存放数据的缓冲区(p_payload指向)
```
-------------------------------------------
|块|总|读|写|     |     |     |     |     |
|大|块|索|索|  0  |  1  |  2  |  3  | ··· |
|小|数|引|引|     |     |     |     |     |
-------------------------------------------
^           ^
|           |
p_shm       p_payload
```

```c
struct shmhead
{
	unsigned int blksize;	// 块大小
	unsigned int blocks;	// 总块数
	unsigned int rd_index;	// 读索引
	unsigned int wr_index;	// 写索引
};
```

```c
struct shmfifo
{
	shmhead_t *p_shm;	// 共享内存头部指针 
	char *p_payload;	// 有效负载的起始地址

	int shmid;		// 共享内存ID
	int sem_mutex;		// 用来互斥用的信号量
	int sem_full;		// 用来控制共享内存是否满的信号量
	int sem_empty;		// 用来控制共享内存是否空的信号量
};
```



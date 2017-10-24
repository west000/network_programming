# 线程池
- 线程池拥有若干个线程
- 用于执行大量的、相对短暂的任务
- 根据任务类型的不同，将线程池中线程的最佳个数也不同，通常：
  a. 计算密集型任务：线程个数 = CPU个数
  b. I/O密集型任务：线程个数 > CPU个数


# 实现一个线程池

##该线程池的特点（两个基本的要求）
- 当任务增加的时候，能够动态的增加线程池中线程的数量，直到达到一个阀值（要求1）
- 当任务执行完毕的时候，能够动态的销毁线程池中的线程（要求2）
- 用于执行大量相对短暂的任务
- 该线程池的实现本质上是生产者和消费者模型的应用。生产者线程向`任务队列`中添加任务。
- 一旦队列有任务到来，如果有等待线程就将其唤醒，用来执行任务；如果没有等待线程，同时如果线程数量没有达到阀值，就创建线程来执行任务，如果线程数量达到阀值，那新任务就等待，直到有空闲线程

## 用到的数据结构
```c
// 任务结构体，将任务放入队列，由线程池中的线程来执行
typedef struct task
{
	void *(*run)(void *arg);	// 任务回调函数
	void *arg;			// 回调函数参数
	struct task *next;		// 下一任务结点
} task_t;

// 线程池结构体
typedef struct threadpool
{
	condition_t ready;	// 任务准备就绪，或线程池销毁通知
	task_t *first;		// 任务队列头指针
	task_t *last;		// 任务队列尾指针
	int counter;		// 线程池中当前线程数
	int idle;		// 线程池中当前正在等待任务的线程数
	int max_threads;	// 线程池中最大允许的线程数
	int quit;		// 销毁线程池的时候置为1
} threadpool_t;

// 初始化线程池
void threadpool_init(threadpool_t *pool,  int threads);
// 在线程池中添加任务
void threadpool_add_task(threadpool_t *pool, void *(*run)(void *arg), void *arg);
// 销毁线程池
void threadpool_destroy(threadpool_t *pool);
```

辅助的数据结构
```c
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
```

说明：
- 线程池中的任务构成了一个队列，采用单链表实现
- condition结构体包含mutex和cond，并提供8个操纵函数 

## 设计的思路
- 由于实现一个线程池比较复杂，因此采用测试驱动开发的编程思想，即先写编写测试代码，然后让这些代码能够正常工作
- 以下是以测试驱动开发的设计过程：
  a. 主线程（生产者）:调用threadpool_add_task向任务队列中加入任务，当任务全部处理完毕后，销毁线程池
  b. 多个消费者线程:等待任务队列中的任务——等到任务则处理之，等到销毁线程池通知则主动退出线程，等待超时则主动退出线程
```c
                     -------     -------     -------
                |-->| 任务n-|-->| 任务2-|-->| 任务1 |
                |    -------     -------     -------
                |                             ^ ^ ^
                |                             | | |
                |                             | | |
                |                             | | |
主线程(生产者)  |                             | | |           
 ------------   |                   消费者1---| | |---消费者n   
| 往任务队列 |__|____________                   |
| 中添加任务 |              |                   |
|------------|              |                 消费者2
| 调用销毁   |____          |                   |
| 线程池函数 |   |          |                   |
|------------|   |          |              消费者线程
| 退出进程   |   |          |              ------------
 ------------    |          |             | 在任务队列 |
                 |      -------------     | 上等待任务 |
     ------------|      |有空闲线程, |    |------------|
    | 置销毁标志 |      | 通知它执行 |    | 等到任务， |
    |------------|      |   任务     |    | 就执行之   |
    | 有等待线程,|      |------------|    |------------|
    | 则广播通知 |      | 无空闲线程 |    | 等到销毁线 |
    |它们销毁线程|      | 且未达到线 |    | 程池的通知 |
    |------------|      | 程池线程数 |    |通知销毁函数|
    | 有处于执行 |      | 量阀值，则 |    | 并退出线程 |
    | 任务状态的 |      |  创建线程  |    |------------|
    | 线程，则等 |      |------------|    |  等待超时  |
    | 待任务结束 |      |  否则，让  |    | 则退出线程 |
     ------------       | 消费者线程 |     ------------
                        | 自己主动从 |
                        | 队列中获取 |
                        |    任务    |
                         ------------
```



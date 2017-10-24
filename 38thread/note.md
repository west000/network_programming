# POSIX线程的其他内容
- 线程属性函数
- 线程并发级别
- 线程特定数据

# 线程属性函数
## 初始化和销毁属性
- int pthread_attr_init(pthread_attr_t *attr);
- int pthread_attr_destroy(pthread_attr_t *attr);


## 获取与设置分离属性
- int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
- int pthread_attr_getdetachstate(pthread_attr_t *attr, int *detachstate);


## 获取与设置栈大小
- int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
- int pthread_attr_getstacksize(pthread_attr_t *attr, size_t *stacksize);


## 获取与设置栈溢出保护区大小
- int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize);
- int pthread_attr_getguardsize(pthread_attr_t *attr, size_t *guardsize);

## 获取与设置线程竞争范围
- int pthread_attr_setscope(pthread_attr_t *attr, int scope);
- int pthread_attr_getscope(pthread_attr_t *attr, int *scope);

## 获取与设置调度策略
- int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
- int pthread_attr_getschedpolicy(pthread_attr_t *attr, int *policy);

## 获取与设置继承的调度策略
- int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
- int pthread_attr_getinheritsched(pthread_attr_t *attr, int *inheritsched);

## 获取与设置调度参数
- int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);
- int pthread_attr_getschedparam(pthread_attr_t *attr, struct sched_param *param);

# 线程并发级别
## 获取与设置并发级别
- int pthread_setconcurrency(int new_level);
- int pthread_getconcurrency(void);

- @new_level:并发级别

注意点：
- 仅在N:M线程模型中有效，设置并发级别，给内核一个提示：表示提供给定级别数量的核心线程来映射用户线程

# 线程特定数据
- 在单线程程序中，经常用到'全局变量'以实现多个函数间共享数据
- 在多线程环境中，由于数据空间是共享的，因此全局变量也是所有线程共有的
- 有时候程序有必要提供'线程私有的全局变量'，仅在某个线程中有效，但却可以跨多个函数访问
- POSIX线程库通过维护TSD(Thread-specific Data)来解决此问题

对特定数据的使用：
- 每个线程都可以有自己的特定数据，总共有128项，以key-value的形式组织
- 要使用一个特定数据，首先需要创建一个key，只要其中给一个线程创建一个key，那么其他线程都会拥有这个key。如下图，线程0创建了一个key，存放在pkey[1]的位置，线程1->线程n都有这个pkey[1],但是它们各自指向的内存都不同。
- 这就好比一个线程私有全局变量a，每个线程都有这个变量a，但是各个线程对a进行操作都不会影响到其他线程

```
            线程0                              线程n
           --------                           --------
          |  其他  |                         |  其他  |
          |  线程  |                         |  线程  | 
          |  信息  |                         |  信息  |
           --------                           --------
pkey[0]   |  指针  |空             pkey[0]   |  指针  |空
           --------                           --------
pkey[1]   |  指针 -|------         pkey[1]   |  指针 -|------
           --------      |                    --------      |
          |        |     |                   |        |     |
  ···     |  ····  |     |           ···     |  ····  |     |
          |        |     |                   |        |     |
           --------      |                    --------      |
pkey[127] |  指针  |空   |         pkey[127] |  指针  |空   |
           --------      |                    --------      |
                         |                                  |
系统数据结构             |                                  |
-------------------------|----------------------------------|-----
线程分配的内存区域       |                                  |
                         |                                  |
                         |                                  |
              --------   |                       --------   |
             |实际数据|<--                      |实际数据|<--
              --------                           --------
```

## 相关函数
### pthread_key_create
- int pthread_key_create(pthread_key_t *key, void (*destructor)(void*));
- 创建一个全局的key，指定销毁实际数据的函数 
- @key:返回key
  @destructor:用来删除key指向的实际数据
- return:成功返回0；失败返回错误码

### pthread_key_delete
- int pthread_key_delete(pthread_key_t key);
- 当线程退出的时候，销毁一个全局的key，并调用pthread_key_create指定的销毁函数销毁实际数据
- @key:pthread_key_create返回的key
- return:成功返回0；失败返回错误码

### pthread_setspecific
- int pthread_setspecific(pthread_key_t key, const void *value);
- 为线程设置与给定key关联的特定数据
- @key:将要与特定数据关联的key
  @value:指向特定数据的指针
- return:成功返回0；失败返回错误码

### pthread_getspecific
- void *pthread_getspecific(pthread_key_t key);
- 获取线程特定数据
- @key:与特定数据关联的key
- return:若key有特定数据关联，则返回指向特定数据的指针；没有数据关联，则返回NULL

### pthread_once
- pthread_once_t once_control = PTHREAD_ONCE_INIT;
- int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));
- 只调用init_routine函数一次，由第一个进入的线程调用init_routine函数
- @once_control:设置为PTHREAD_ONCE_INIT
  @init_routine:仅被调用一次的函数，由第一个进入的线程调用
- return:成功返回0；失败返回错误码


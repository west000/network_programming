# POSIX 共享内存

# 常用函数
```c
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

int shm_open(const char *name, int oflag, mode_t mode);
int ftruncate(int fd, off_t length);
int fstat(int fd, struct stat *buf);
int shm_unlink(const char *name);
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
```

## shm_open
- 创建或打开一个共享内存对象
- int shm_open(const char *name, int oflag, mode_t mode);
- @name:共享内存对象的名字
  @oflag:与open函数类似，可以是O_RDONLY、O_RDWR，还可以按位或上O_CREAT、O_EXCL、O_TRUNC等
  @mode:此参数总是需要设置，如果oflag没有指定O_CREAT，可以指定为0
- return:成功返回非负整数文件描述符；失败返回-1

注意点：
- 与mq_open创建的消息队列显示，shm_open创建的共享内存存在于虚拟文件系统中，需要挂载到某个目录中才能看到创建的共享内存。只不过这个挂载过程已经有系统完成了
- 挂载的目录为/dev/shm

## ftruncate
- 修改共享内存对象/文件的大小
- int ftruncate(int fd, off_t length);
- @fd:文件描述符
  @length:长度
- return:成功为0；失败为-1

## fstat
- 获取共享内存对象/文件的信息
- int fstat(int fd, struct stat *buf);
- @fd:文件描述符
  @buf:返回的共享内存状态
- return:成功为0；失败为-1

## shm_unlink
- 减少连接数，当连接数为0时，删除共享内存对象
- int shm_unlink(const char *name);
- @name:共享内存对象的名字
- return:成功为0；失败为-1

## mmap
- 将共享内存对象映射到进程地址空间
- void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
- @addr:要映射的起始地址，通常为NULL，让内核自动选择
  @length:映射到进程地址空间的字节数
  @prot:映射区保护方式
  @flags:标志
  @fd:文件描述符
  @offset:从文件开始的偏移量
- return:成功返回映射到的内存区的起始地址；失败返回-1



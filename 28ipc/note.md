# 内存映射
使用的注意点：
- 映射无法改变文件的大小
- 可用于进程间通信的有效地址空间不完全受限于被映射文件的大小
- 文件一旦被映射后，所有对映射区域的访问实际上是对内存区域的访问。映射区域内容写回文件时，所写内容不能超过文件的大小

# 共享内存常用的函数
```c
#include <sys/mman.h>
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
int msync(void *addr, size_t length, int flags);
```

## mmap
- 将文件或者设备空间映射到共享内存区
- void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
- @addr:要映射的起始地址，通常为NULL，让内核自动选择
  @len:映射到进程空间的字节数
  @prot:映射区保护模式
  @flags:标志
  @fd:文件描述符
  @offset:从文件头开始的偏移量
- return：成功返回映射到的内存区的起始地址（页面的整数倍），失败返回-1

prot | 说明
---|---
PROT_READ | 页面可读
PROT_WRITE | 页面可写
PROT_EXEC | 页面可执行
PROT_NONE | 页面不可访问

flags | 说明
---|---
MAP_SHARED | 变动是共享的
MAP_PRIVATE | 变动是私有的
MAP_FIXED | 准确解释addr参数（使addr对齐到页面的低端地址，这个选项一般不使用）
MAP_ANONYMOUS | 建立匿名映射区（不涉及文件，指定该选项之后忽略fd、offset参数）

## munmap
- 取消mmap函数建立的映射
- int munmap(void *addr, size_t length);
- @addr:映射的内存起始地址
  @len:映射到进程地址空间的字节数
- return：成功0，失败-1

## msync
- 对映射的共享内存执行同步操作
- int msync(void *addr, size_t length, int flags);
- @addr:映射的内存起始地址
  @len:字节数
  @flags:选项
- return：成功0，失败-1

flags | 说明
MS_ASYNC | 执行异步写
MS_SYNC | 执行同步写
MS_INVLIDATE | 使高速缓存的数据失效

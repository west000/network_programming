# System V共享内存
随内核持续

```c
#include <sys/ipc.h>
#include <sys/shm.h>

int shmget(key_t key, size_t size, int shmflg);
void *shmat(int shmid, const void *shmaddr, int shmflg);
int shmdt(const void *shmaddr);
int shmctl(int shmid, int cmd, struct shmid_ds *buf);
```

## shmget
- 创建共享内存
- int shmget(key_t key, size_t size, int shmflg);
- @key:共享内存段的名字
  @size:共享内存大小
  @shmflg:由9个权限标志构成，用法与创建文件时使用的mode模式标志一样
- return：成功返回一个非负整数，表示该共享内存段的标识码；失败返回-1

## shmat
- 将共享内存段连接到进程的地址空间
- void *shmat(int shmid, const void *shmaddr, int shmflg);
- @shmid:shmget返回的共享内存标识
  @shmaddr:指定连接的地址
  @shmflg:连接的方式，取值可以是SHM_RND和SHM_RDONLY
- return:成功返回一个指针，指向共享内存第一个节；失败返回-1

连接地址的规定：
```c
if(shmaddr == NULL)
	内核自动选择一个地址
else if(shmflg 没有设置SHM_RND标识)
	以shmaddr为连接地址
else if(shmflg 设置SHM_RND标识)
	自动向下调整为SHMLBA(4KB，即4096B)的整数倍，公式为：shmaddr-(shmaddr%SHMLBA)

if(shmflg == SHM_RDONLY)
	连接操作用来只读共享内存

通常情况下，shmflg指定为0，表示共享内存段可读可写
```

## shmdt
- 将共享内存段与当前进程脱离
- int shmdt(const void *shmaddr);
- @shmaddr:shmat返回的指针
- return:成功为0，失败为-1
- 注意：将共享内存段与当前进程脱离不等于删除共享内存段


## shmctl
- 用来创建和访问一个共享内存
- int shmctl(int shmid, int cmd, struct shmid_ds *buf);
- @shmid:由shmget返回的共享内存标识码
  @cmd:将要采取的动作（主要有三个）
  @buf:指向一个保存着共享内存模式状态和访问权限的数据结构
- return:成功为0，失败为-1

命令 | 说明
---|---
IPC_STAT | 把shmid_ds结构中的数据设置为共享内存的当前关联值
IPC_SET	| 在进程有足够权限的前提下，把共享内存的当前关联值设置为shmid_ds数据结构中给出的值
IPC_RMID | 删除共享内存段


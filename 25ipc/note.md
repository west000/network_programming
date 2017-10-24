# System V 消息队列
- 消息队列提供了从一个进程向另一个进程发送一块数据的方法，同样管道也是可以实现此功能，但是管道是基于字节流的，数据之间是没有边界的
- 与管道不同，消息队列的每个数据块都被认为有一个类型，接收者进程接收的数据块可以有不同的类型值，数据之间是有边界的；另一个区别是，消息接收的顺序不必按照发送的顺序，而管道所传送的数据一定是按照先进先出的原则来进行接收的
- 与管道相同，消息队列也有同样的不足：每个消息的最大长度是有限制的（MSGMAX），每个消息队列的总字节数也是有上限的（MSGMNB），系统上消息队列的总数也有上限（MSGMNI）

```
$ cat /proc/sys/kernel/msgmax 
8192
$ cat /proc/sys/kernel/msgmnb
16384
$ cat /proc/sys/kernel/msgmni	# 应该跟内存大小有关
5662
```

# IPC对象的数据结构
System V有三种IPC：消息队列、共享内存、信号量
内核为每个IPC对象维护一个数据结构，如下所示：
```c
$ man 2 msgctl

struct ipc_perm {
	key_t          __key;       /* Key supplied to msgget(2) */
	uid_t          uid;         /* Effective UID of owner */
	gid_t          gid;         /* Effective GID of owner */
	uid_t          cuid;        /* Effective UID of creator */
	gid_t          cgid;        /* Effective GID of creator */	
	unsigned short mode;        /* Permissions */
	unsigned short __seq;       /* Sequence number */
};
```

# 消息队列结构
```c
$ man 2 msgctl

struct msqid_ds {
	struct ipc_perm msg_perm;     /* Ownership and permissions */
	time_t          msg_stime;    /* Time of last msgsnd(2) */
	time_t          msg_rtime;    /* Time of last msgrcv(2) */
	time_t          msg_ctime;    /* Time of last change */
	unsigned long   __msg_cbytes; /* Current number of bytes in queue (nonstandard) */ 消息队列当前字节数
	msgqnum_t       msg_qnum;     /* Current number of messages in queue */  消息队列当前消息数
	msglen_t        msg_qbytes;   /* Maximum number of bytes allowed in queue */ 等于MSGMNB
	pid_t           msg_lspid;    /* PID of last msgsnd(2) */
	pid_t           msg_lrpid;    /* PID of last msgrcv(2) */
};
```

# 消息队列在内核中的表示
- 消息队列中，每个消息是以链表的形式链接起来的
```
nextmsg ----> nextmsg ----> NULL
type=1        type=3        type=2
len=1         len=2         len=3
数据          数据          数据
              数据          数据
                            数据
```

# 消息队列的常用函数
```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int msgget(key_t key, int msgflg);
int msgctl(int msqid, int cmd, struct msqid_ds *buf);
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
```

## msgget
- 用来创建和访问一个消息队列
- int msgget(key_t key, int msgflg);
- @key：某个消息队列的名字
  @msgflg：由9个权限标志构成，用法与创建文件是使用的mode模式标志一样
- return：成功返回该消息队列的标识码（非负整数）；失败返回-1

相关命令：
```sh
$ ipcs # 可以查看系统中所有的消息队列
$ ipcrm -q msqid # 删除标识号为msqid的消息队列
$ ipcrm -Q key   # 如果不是使用IPC_PRIVATE创建的消息队列，可以根据key进行删除
```

内核创建或打开一个IPC对象的逻辑：
```
if(key == IPC_PRIVATE)
{
	if(系统表格满) // 如果是消息队列，则达到MSGMNI
		errno = ENOSPC;
	else
		成功，创建新对象，返回标识符
}
else 
{
	if(key已经存在)
	{
		if(IPC_CREAT和IPC_EXCL都设置了)
			errno = EEXIST;
		else if(访问权限允许)
			成功，返回标识符
		else
			errno = EACCES;
	}
	else
	{
		if(设置IPC_CREAT)
		{
			if(系统表格满) // 如果是消息队列，则达到MSGMNI
                		errno = ENOSPC;
        		else
                		成功，创建新对象，返回标识符
		}
		else 
			errno = ENOENT;
	}	
}

```


## msgctl
- 消息队列的控制函数
- int msgctl(int msqid, int cmd, struct msqid_ds *buf);
- @msqid：有msgget函数返回的消息队列标识符
  @cmd：要采取的动作（常用的有IPC_STAT、IPC_SET、IPC_RMID）
- return：成功0，失败-1

## msgsnd

## msgrcv


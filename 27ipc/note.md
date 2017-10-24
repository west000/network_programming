# echosrv和echocli
实现了一个简单的本机回射服务器

## echosrv
- 创建一个众所周知的消息队列，key=1234
- 从消息队列接收的数据
  msg.mtype = 1
  msg.mtext = [pid + data]
- 向消息队列中发送的数据
  msg.mtype = pid
  msg.mtext = [pid + data]

## echocli
- 打开一个众所周知的消息队列，key=1234
- 发送给服务端的数据
  msg.mtype = 1
  msg.mtext = [pid + data]
- 从服务端接收的数据
  msg.mtype = pid
  msg.mtext = [pid + data]

## 存在的问题——可能造成死锁
- 由于客户端和服务器都同一个消息队列中读写数据
- 考虑一种情况，如果有很多个客户端同时向消息队列写数据，导致消息队列写满了，这时候服务端无法再向消息队列中写数据，因此服务端阻塞在msgsnd，无法执行msgrcv；而客户端阻塞在msgrcv中。从而导致死锁

# echosrv_nondeadlock 和 echocli_nondeadlock
- 实现一个不会产生死锁的本机回射服务器
- 前面的回射服务器和回射客户端共用同一个消息队列，可能导致死锁
- 而这里每个客户端echocli_nondeadlock各自创建一个私有消息队列，并把自己的私有消息队列标识号发送给服务端，服务端接收到对应客户端的数据之后，解析出pid+msgid，然后往这个msgid回射数据

# echocli_nondeadlock
- 创建一个私有消息队列，标识号为private_msgid
- 打开众所周知的消息队列，key=1234
- 发送给服务端的数据
  msg.mtype = 1
  msg.mtext = [pid + private_msgid + data]
- 从消息队列private_msgid接收的数据
  msg.mtype = pid
  msg.mtext = [pid + private_msgid + data]

# echocli_nondeadlock
- 创建一个众所周知的消息队列，key = 1234
- 从消息队列中接收数据：
  msg.mtype = 1
  msg.mtext = [pid + private_msgid + data]
- 向消息队列private_msgid中发送的数据
  msg.mtype = 1
  msg.mtext = [pid + private_msgid + data]




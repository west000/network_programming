#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>	/* For O_* constants */
#include <sys/stat.h>	/* For mode costants */
#include <mqueue.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m)			\
	do {				\
		perror(m);		\
		exit(EXIT_FAILURE);	\
	}while(0)

typedef struct stu
{
	char name[32];
	int age;
} STU;

int main(int argc, char *argv[])
{
	mqd_t mqid;
	mqid = mq_open("/abc", O_CREAT | O_RDWR, 0666, NULL);
	if(mqid == (mqd_t)-1)
		ERR_EXIT("mq_open");

	struct mq_attr attr;
        mq_getattr(mqid, &attr);
	size_t size = attr.mq_msgsize;	

	STU stu;
	unsigned prio;

	// 接收的长度必须等于每条消息的最大长度
	if(mq_receive(mqid, (char*)&stu, size, &prio) == (mqd_t)-1)
		ERR_EXIT("mq_receive");

	printf("name=%s age=%d prio=%d\n", stu.name, stu.age, prio);

	mq_close(mqid);
	return 0;
}

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

int main(void)
{
	mqd_t mqid;
	mqid = mq_open("/abc", O_CREAT | O_RDWR, 0666, NULL);
	if(mqid == (mqd_t)-1)
		ERR_EXIT("mq_open");

	printf("mq_open succ\n");	
	
	struct mq_attr attr;
	mq_getattr(mqid, &attr);
	printf("max #msg=%ld max #bytes/msg=%ld #currently on queue=%ld\n",
		attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
	
	mq_close(mqid);
	return 0;
}

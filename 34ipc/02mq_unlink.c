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
	mq_unlink("/abc");
	return 0;
}

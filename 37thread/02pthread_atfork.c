#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include <stdio.h>

void prepare(void)
{
	printf("pid = %d prepare ...\n", (int)getpid());
}

void parent(void)
{
	printf("pid = %d parent ...\n", (int)getpid());
}

void child(void)
{
	printf("pid = %d child ...\n", (int)getpid());
}

int main(void)
{
	printf("pid = %d Entering main ...\n", (int)getpid());

	pthread_atfork(prepare, parent, child);

	fork();
	
	printf("pid = %d Exiting main ...\n", (int)getpid());

	return 0;
}

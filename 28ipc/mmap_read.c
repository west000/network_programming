#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>

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
	char name[4];
	int age;
}STU;

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "Usage: %s <file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int fd;
	fd = open(argv[1], O_RDWR);
	if(fd == -1)
		ERR_EXIT("open");

	/*打开一个文件，将文件映射到共享内存区*/	
	STU *p;
	p = (STU*)mmap(NULL, sizeof(STU)*5, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(p == NULL)
		ERR_EXIT("mmap");

	printf("Mapping address:%p\n", p);

	/*从共享内存区读取数据*/
	int i;
	for(i=0; i<5; i++)
	{
		printf("name = %s age = %d\n", (p+i)->name, (p+i)->age);
	}

	sleep(10);
	/*解除映射*/
	munmap(p, sizeof(STU)*5);
	printf("exiting...\n");
	return 0;
}

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

	/*创建/打开一个文件*/
	int fd;
	fd = open(argv[1], O_CREAT | O_RDWR | O_TRUNC, 0666);
	if(fd == -1)
		ERR_EXIT("open");
	
	/*定位到第39字节出，写入一个空字符串*/
	lseek(fd, sizeof(STU)*5-1, SEEK_SET);
	write(fd, "", 1);

	/*将文件映射到共享内存区*/
	STU *p;
	p = (STU*)mmap(NULL, sizeof(STU)*5, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(p == NULL)
		ERR_EXIT("mmap");

	printf("Mapping address:%p\n", p);

	/*往共享内存区写入五个学生数据*/
	char ch = 'a';
	int i;
	for(i=0; i<5; i++)
	{
		memcpy((p+i)->name, &ch, 1);	
		(p+i)->age = 20 + i;
		ch++;
	}

	
	sleep(10);
	/*解除映射*/
	printf("initialize over\n");
	munmap(p, sizeof(STU)*5);
	printf("exiting...\n");
	return 0;
}

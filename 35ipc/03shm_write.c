#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m)			\
	do {				\
		perror(m);		\
		exit(EXIT_FAILURE);	\
	} while(0)

typedef struct stu
{
	char name[32];
	int age;
} STU;

int main(void)
{
	int shmid;
	// 由于下面mmap的prot=PROT_WRITE，同时flags=MAP_SHARED
	// 因此shm_open的oflag需要指定为O_RDWR，否则会产生EACCESS错误
	shmid = shm_open("/xyz", O_RDWR, 0);
	if(shmid == -1)
		ERR_EXIT("shm_open");

	printf("shm_open succ\n");

	// 获取共享内存的状态
	struct stat buf;
	if(fstat(shmid, &buf) == -1)
		ERR_EXIT("fstat");
	
	STU *p;
	p = mmap(NULL, buf.st_size, PROT_WRITE, MAP_SHARED, shmid, 0);
	//if(p == (void*)-1) 
	if(p == MAP_FAILED)
		ERR_EXIT("mmap");
		
	// 往共享内存写入一些数据
	strcpy(p->name, "test");
	p->age = 20;

	// 使用od -c xyz可以查看写入的内容
	close(shmid);
	return 0;
}

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
	// 与System V的创建共享内存不同，shm_open无法为共享内存指定大小
	// 创建的时候，大小为0
	// 可以用ftruncate修改共享内存的大小
	shmid = shm_open("/xyz", O_CREAT | O_RDWR, 0666);
	if(shmid == -1)
		ERR_EXIT("shm_open");

	printf("shm_open succ\n");

	// 修改共享内存的大小
	if(ftruncate(shmid, sizeof(STU)) == -1)
		ERR_EXIT("ftruncate");

	// 获取共享内存的状态
	struct stat buf;
	if(fstat(shmid, &buf) == -1)
		ERR_EXIT("fstat");
	printf("size=%ld mode=%o nlink=%ld\n", buf.st_size, buf.st_mode & 0777, buf.st_nlink);
	
	close(shmid);
	return 0;
}

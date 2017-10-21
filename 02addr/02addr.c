#include <stdio.h>
#include <arpa/inet.h>

int main()
{
	unsigned long addr = inet_addr("198.162.0.1");
	printf("addr=%u\n", ntohl(addr));
	
	struct in_addr ipaddr;
	ipaddr.s_addr = addr;
	printf("%s\n", inet_ntoa(ipaddr));
	return 0;
}

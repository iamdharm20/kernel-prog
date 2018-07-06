/*  This userspace program maps 3 MB (uncached) SDRAM and writes to it. */
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
 
int main(void)
{
	char* address = NULL;

	int fd = open("/dev/mychar0", O_RDWR);
	if (fd < 0) {
		printf("open call failed\n");
		return -1;
	}
	address = mmap(0, 0x00300000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0xc0000000);
	if (address == MAP_FAILED) {
		printf("mmap operation failed");
		close(fd);
		return -1;
	}
	close(fd);
	address[100] = 3;

	munmap(data, 0x00300000);
	return 0;
}

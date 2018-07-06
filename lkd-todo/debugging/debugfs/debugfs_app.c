#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include "../include/dfs_types.h"

int open_cpu_dev(const char *dfs_path)
{
	char dev_path[128];

	sprintf(dev_path, "%s/%s/%s", 
		dfs_path, dfs_module_name, dfs_dev_cpu_name);
	
	return open(dev_path, O_RDONLY);
}

int open_mem_dev(const char *dfs_path)
{
	char dev_path[128];

	sprintf(dev_path, "%s/%s/%s", 
		dfs_path, dfs_module_name, dfs_dev_mem_name);
	
	return open(dev_path, O_RDWR);
}


void close_dev(int cpu_fd, int mem_fd)
{
	close(cpu_fd);
	close(mem_fd);
}

int read_cpu_dev(int fd)
{
	int ret;
	char buf[14];				//strlen(hello world!!)
	memset(buf, 0, 14);

	ret = read(fd, buf, 14);
	if(ret < 0) {
		perror("read failed");
		return -1;
	}
	
	printf("%s\n", buf);
	return 0; 
}

int read_mem_dev(int fd)
{
	int ret;
	struct dfs_pack pack;

	ret = read(fd, &pack, sizeof(struct dfs_pack));
	if(ret < 0) {
		perror("read failed");
		return -1;
	}

	if(ret != sizeof(struct dfs_pack)) {
		return -EAGAIN;
	}

	printf("sym: %c, type: %d, status: %d\n", pack.symbol, pack.type, pack.status);
	
	return 0;
}

int write_mem_dev(int fd)
{
	int ret;
	unsigned char data;

	srand(time(NULL));
	data = rand()%50;

	ret = write(fd, &data, 1);
	if(ret < 0) {
		perror("write failed");
		return -1;
	}

	printf("send data %u to kernel\n", data);

	return 0;
}

int main()
{
	int cpu_fd, mem_fd;
	const char *dfs_path = "/sys/kernel/debug";


	cpu_fd = open_cpu_dev(dfs_path);
	if(cpu_fd < 0) {
		perror("error: could not open cpu dev");
		return -errno;
	}


	mem_fd = open_mem_dev(dfs_path);
	if(mem_fd < 0) {
		perror("error: could not open mem dev");
		close(cpu_fd);
		return -errno;
	}

	read_cpu_dev(cpu_fd);
	read_mem_dev(mem_fd);
	write_mem_dev(mem_fd);

	close_dev(cpu_fd, mem_fd);

	return 0;
}

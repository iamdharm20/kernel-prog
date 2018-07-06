#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
 
int main()
{
    int fd;
    int m=100;
    char *map;
 
    fd=open("/dev/mydev",O_RDWR);
 
    map = mmap(0, 8192, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
 
    while(1)
    {
        puts(map);
        sleep(4);
    }
 
    return 0;
}


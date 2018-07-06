#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
 
#define SIZE 64
 
struct mydevicestatus
{
    int r1;
    int r2;
    int r3;
};
 
int userfn(int v)
{
    return v+100;
}
 
int main()
{
    char buf[100];
    struct mydevicestatus status1={33,44,55};
    int fd;
    int m=100;
    int *map;
 
    fd=open("/dev/mydev",O_RDWR);
 
    //read(fd,buf,100);
 
    ioctl(fd,10,&m);
 
    ioctl(fd,20,userfn);
 
    ioctl(fd,30,&status1);
 
    map = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
 
    printf("RTC=%d\n",map[0]);
    sleep(3);
    printf("RTC=%d\n",map[0]);
 
    return 1;
}

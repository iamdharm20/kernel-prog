#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <sys/select.h>
#include <poll.h>
#include <libaio.h>
#include <errno.h>
#include <assert.h>
 
int main()
{
    int fd,er;
    unsigned nr_events=1;
    char buf[100];
    io_context_t ctxp = NULL;
    struct iocb *arr[1];
    struct iocb iocbr;
    memset(&iocbr,0,sizeof(struct iocb));
    fd=open("/dev/simpdev",O_RDWR);
    er=io_setup(nr_events, &ctxp);
    if(er!=0)
        printf("error:%d\n",er);
    io_prep_pwrite(&iocbr,fd,buf,100,0);
 
    arr[0]=&iocbr;
    io_submit( ctxp, 1, arr);
    return 1;
}

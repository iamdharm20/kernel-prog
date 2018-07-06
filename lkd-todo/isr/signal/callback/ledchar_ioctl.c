#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <signal.h>
 
#include "ledchar_ioctl.h"

#define SIG_TEST 44 /* we define our own signal, hard coded since SIGRTMIN is different in user and in kernel space */ 

struct sigaction act;

/* signal handler definition goes here */
void sig_handler(int signo, siginfo_t *sinfo, void *ucontext)
{
        printf("Got alarm signal %d\n",signo);
	printf("received value %i\n", sinfo->si_int);
        /* do the required stuff here */
}

void send_pid(int pid, int fd)
{
     
	if (ioctl(fd, CD_SIGNAL_GEN, pid) == -1) {
		perror("ledchar: ioctl failed");
	}
}
 
int main(int argc, char *argv[])
{
	char *file_name = "/dev/ledchar";
	int fd;
	pid_t pid = 0;

	fd = open(file_name, O_RDWR);
	if (fd < 0) {
		perror("ledchar: open failed\n");
		return fd;
	}
 
	pid = getpid();


        act.sa_sigaction = sig_handler;
        act.sa_flags = SA_SIGINFO;

        /* register signal handler */
        sigaction(SIG_TEST, &act, NULL);

	/* kernel needs to know our pid to be able to send us a signal */
	send_pid(pid, fd);

        /* wait for any signal from kernel */
        pause();

        /* after signal handler execution */
        printf("back to main\n");
	
	close (fd);
 
	return 0;
}

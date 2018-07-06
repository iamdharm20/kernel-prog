#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define N 100
#define MEGEXTRA 10000

struct private_data {
	int *len;
	char *skbuff;
	int thread_status;
};

pthread_attr_t attr;

void * thread_rutine ( void *arg)
{
	
	int i = 0;
	size_t mystacksize;

	struct private_data *targ = (struct private_data *)arg;

	pthread_attr_getstacksize (&attr, &mystacksize);
	printf("Thread stack size = %li bytes \n", mystacksize);
	
	printf("thread pid:%d : tgid: %ld\n", getpid(), pthread_self());
	printf("len: %d\n", *(targ->len) );
	for (i =0 ; i < *(targ->len); i++)
		printf("%c", *(targ->skbuff + i));

	printf("\n");

	targ->thread_status = getpid();

	pthread_exit((void *)&targ->thread_status);
}

int main()
{
	int ret = 0;
	void *status;

	size_t stacksize;

	pthread_t thread;

	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_getstacksize (&attr, &stacksize);

	
	char skbuff[] = "thread private data";
	int len = strlen(skbuff);

	printf("process id: %d\n", getpid());

	struct private_data *arg = (struct private_data *)malloc(sizeof(struct private_data));
	arg->len = &len;
	arg->skbuff = skbuff;
	
	printf("Default stack size = %li bytes [ulimit -a]\n", stacksize);

	stacksize = ( 2 * 1024 * 1024 );

	printf("Amount of stack needed per thread = %li\n",stacksize);
	pthread_attr_setstacksize (&attr, stacksize);
	printf("Creating threads with stack size = %li bytes\n",stacksize);
		
	ret = pthread_create(&thread, &attr, &thread_rutine, (void *)arg);
	if(ret) {
		perror("thread creation failed: \n");
		free(arg);
		return -1;
		//exit(-1);
	}
	/* subroutine blocks the calling thread until the specified threadid thread terminates */
	ret = pthread_join(thread, &status);
	if(ret) {
		printf("ERROR; return code from pthread_join() is %d\n", ret);
		free(arg);
		exit(-1);
	}
	printf("Main: completed join with thread having a status of %d\n", *((int *)status));

	/* Free attribute and wait for the other threads */
	pthread_attr_destroy(&attr);

	free(arg);
	
	return 0;
}

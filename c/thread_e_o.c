#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

/*a mutex with the condition. */
pthread_cond_t cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int num = 0;

void *thread_efun()
{
	/* Lock mutex and then wait for signal to relase mutex */
	pthread_mutex_lock(&mutex);	
	do {
		if( num%2 == 0 ) {
			printf("%d ", num);
			num++;
		} else
			pthread_mutex_unlock(&mutex);
		
	}while(num <= 100);

	pthread_exit(NULL);
}

void *thread_ofun()
{
	pthread_mutex_lock(&mutex);	
	do{	
		if( (num%2 != 0) ) {
			printf("%d ", num);
			num++;
		} else
			pthread_mutex_unlock(&mutex);
		
	}while(num <= 100);
	
	pthread_exit(NULL);

}
int main()
{
	int ret = 0;
	pthread_t thread[2];
	
	ret = pthread_create(&thread[0], NULL, &thread_efun, NULL);
        if(ret) {
                perror("pthread_create failed: \n");
                return -1;
        }

	ret = pthread_create(&thread[1], NULL, &thread_ofun, NULL);
        if(ret) {
                perror("pthread_create failed: \n");
                return -1;
        }

        ret = pthread_join(thread[0], NULL);
        if(ret) {
                perror("pthread_join failed: \n");
                return -1;
        }
        ret = pthread_join(thread[1], NULL);
        if(ret) {
                perror("pthread_join failed: \n");
                return -1;
        }

	return 0;
}


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
struct {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
    int ready; 
} var = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0};

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void *A(void *arg);
void *B(void *arg);
int main(int argc,char **argv)
{
    pthread_t tid_A,tid_B;
    int fd;	
    fd = open("./txt",O_RDWR | O_CREAT | O_APPEND,0664);
    if(fd < 0)
    {
        printf("open failed!");
        return -1;
    }
	pthread_create(&tid_A,NULL,A,&fd); 
	pthread_create(&tid_B,NULL,B,&fd);
	pthread_join(tid_A,NULL);
	pthread_join(tid_B,NULL);
    close(fd);

	return 0;
}

void *A(void *arg)
{
	int count = 0;
    int fd = *((int*)arg);	
	for(;;)
	{
		pthread_mutex_lock(&var.mutex);
		write(fd,"A",1);
        count++;
		if(var.ready == 0) 
		{
     		var.ready = 1;
			pthread_cond_signal(&var.cond);
		}			
		pthread_mutex_unlock(&var.mutex);

	    if(count == 3)
        {
            pthread_exit(0);
        }
        sleep(1);
	
	}
	return NULL;
}

void *B(void *arg)
{
	int fd = *((int*)arg);
	int count = 0;
	for(;;)
	{
		pthread_mutex_lock(&var.mutex);
		while(var.ready == 0)
		{
			pthread_cond_wait(&var.cond,&var.mutex);
		}
		
    	write(fd,"B",1);
		count++;
		var.ready = 0;
		pthread_mutex_unlock(&var.mutex);

        if(count == 3)
        {
            pthread_exit(0);
        }
	}
	return NULL;
}


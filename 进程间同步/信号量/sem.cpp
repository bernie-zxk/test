#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

#define N 20

struct {
    sem_t mutex;
    sem_t nempty;
    sem_t nstored; 
} shared;

typedef struct queue{
    char data[N];
    int front,rear;
}*circular_queue,circular_queue1;
circular_queue create();
int is_empty(circular_queue L);
int is_full(circular_queue L);
int enqueue(circular_queue L,char data);
char dequeue(circular_queue L);
int pf(circular_queue L);
int is_right(char ch);
void *A(void *arg);
void *B(void *arg);
void *C(void *arg);
int main(int argc,char **argv)
{
    pthread_t tid_A,tid_B,tid_C;
    
    circular_queue L = create();  
    enqueue(L,'p');
    sem_init(&shared.mutex,0,1);
    sem_init(&shared.nempty,0,20);
    sem_init(&shared.nstored,0,0);
    
	pthread_create(&tid_A,NULL,A,L); 
	pthread_create(&tid_B,NULL,B,L);
	pthread_create(&tid_C,NULL,C,L);
	pthread_join(tid_A,NULL);
	pthread_join(tid_B,NULL);
	pthread_join(tid_C,NULL);
 
    sem_destroy(&shared.mutex);
    sem_destroy(&shared.nempty);
    sem_destroy(&shared.nstored);

	return 0;
}


void *A(void *arg)
{
    char ch = 's';
    circular_queue L = (circular_queue)arg;
	while(1)
	{
        sem_wait(&shared.nempty);
        sem_wait(&shared.mutex);
        if(is_full(L))
        {
            sem_post(&shared.mutex);
            sem_post(&shared.nempty);
        }
        else
        {
        enqueue(L,ch);
        sem_post(&shared.mutex);
        sem_post(&shared.nstored);
        }
        sleep(1);
	}
	return NULL;
}

void *B(void *arg)
{
    char ch;
	circular_queue L = (circular_queue)arg;

    while(1)
    {
        ch = getchar();
        getchar();
        sem_wait(&shared.nempty);
        sem_wait(&shared.mutex);
        if(is_full(L))
        {
            sem_post(&shared.mutex);
            sem_post(&shared.nempty);
        }
        else
        {
        enqueue(L,ch);
        sem_post(&shared.mutex);
        sem_post(&shared.nstored);
        }
      
    }

	return NULL;
}

void *C(void *arg)
{
    FILE *fp;
    char ch;
    circular_queue L = (circular_queue)arg;
    fp = fopen("test.txt","a+");
    while(1)
    {
        sem_wait(&shared.nstored);
        sem_wait(&shared.mutex);
        if(is_empty(L))
        {
            sem_post(&shared.mutex);
            sem_post(&shared.nstored);
        }
        else
        {
        ch = dequeue(L);
       // putchar(ch);
        if(is_right(ch))
        {
            putc(ch,fp);
            fflush(fp);
        }
        sem_post(&shared.mutex);
        sem_post(&shared.nempty);
        }

    }
    fclose(fp);
    return NULL;
}

circular_queue create()
{
    circular_queue L;
    L = (circular_queue)malloc(sizeof(circular_queue1));
    if(L == NULL)
    {
        printf("failed to malloc!");
        return NULL;
    }
    L->front = L->rear = 0;
    
    return L;
}

int is_right(char ch)
{
    switch(ch)
    {
        case 's':
        return 1;
        break;
        case 'w':
        return 1;
        break;
        case 'a':
        return 1;
        break;
        case 'd':
        return 1;
        break;
        default:
        return 0;
        break;
    }
    
    return 0;
}

int is_empty(circular_queue L)
{
    return L->front == L->rear ? 1 : 0;
}

int is_full(circular_queue L)
{
    return (L->rear + 1) % N == L->front ? 1 : 0;
}

int enqueue(circular_queue L,char val)
{
    if(is_full(L))
    {
    //    printf("queue full");
        return -1;
    }
   L->data[L->rear] = val;
   L->rear = (L->rear + 1) % N;
 
   return 0;
}

char dequeue(circular_queue L)
{
    int val;
    if(is_empty(L))
    {
      //  printf("queue empty!");
        return 'k';
    }
    val = L->data[L->front + 1];
    L->front = (L->front + 1) % N;
    //printf("%c ",val);   
    return val;
    
}

int pf(circular_queue L)
{
    if(is_empty(L))
    {
        printf("empty!");
        return -1;
    }
    while(!is_empty(L))
    {
        printf("%c ",dequeue(L));
    }
    return 0;
}

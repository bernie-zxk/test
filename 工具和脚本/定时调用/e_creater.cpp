#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <libgen.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream.h>
#include <unistd.h>

#define TIMER_FAILURE -1
#define ABS TIMER_ABSTIME
#define REL 0
#define TIMERS 1

time_t lasttime;
timer_t                  timerid[TIMERS];
//define control structure for timer example
struct timer_definitions{
	int		type;					//abs or relative timer
    struct	sigevent          evp;	//event structure
	struct	itimerspec       timeout;	//timer interval
};
struct timer_definitions timer_values[TIMERS]={
	{REL,{SIGEV_SIGNAL,SIGUSR1},{60,0,60,0}},
};

void setuptimer();

void settimer(int s);

struct tm *  tm1;
time_t      now;

int ll_interval=0,ll_start=0,ll_end=0;

char e_path[200];

int
main(int argc, char* argv[])
{
    int        state = 0;
    timeval    lasttime;			//计时终止时间
    if(argc ==1)
    {
       ll_interval=60;
       ll_start=0;
       ll_end=0;
    }
    else if(argc == 2)
    {
       ll_interval=atoi(argv[1]);
       ll_start=0;
       ll_end=0;
    }
    else if(argc == 3)
    {
       ll_interval=atoi(argv[1]);
       ll_start=atoi(argv[2]);
       ll_end=0;
    }
    else
    {
       ll_interval=atoi(argv[1]);
       ll_start=atoi(argv[2]);
       ll_end=atoi(argv[3]);
    }
    if(ll_end > 0) ll_end=ll_end*60;
    if(ll_start > 0) 
      ll_start=ll_start*60;
    else
      ll_start=0;
    
     if(getenv("CCAPI_E")!= NULL)
     {
        strcpy(e_path,getenv("CCAPI_E"));
     }
     else
     {
        printf("环境变量CCAPI_E没有设置\n");
	exit(1);
     }
    setuptimer();
   
    for(;;)
    {
	 sleep(86400);
      
    }
    return state;
}

void setuptimer()
{
   //time_t                   local,timerid;
	sigset_t mask;
	struct sigaction         act;
	clockid_t clock_id =CLOCK_REALTIME;
  
   sigemptyset(&mask);
   act.sa_handler = settimer;
   act.sa_flags = 0;
   sigemptyset(&act.sa_mask);

	for(int it=0; it<TIMERS;it++){
		int ret=timer_create(clock_id, &timer_values[it].evp, &timerid[it]);
		if(ret==TIMER_FAILURE){
			perror("timer_create");
			exit(TIMER_FAILURE);
		}

		sigaction(timer_values[0].evp.sigev_signo,&act,NULL);
                
         now=time(NULL);
         if(ll_start >= 60)
		   ll_start=ll_start -(now%60) ;
		 else
           ll_start=60 -(now%60) ;

		 timer_values[0].timeout.it_value.tv_sec=ll_start;// -(now%60) +60;
	     timer_values[0].timeout.it_value.tv_nsec=0;

	     timer_values[0].timeout.it_interval.tv_sec=ll_interval;
	     timer_values[0].timeout.it_interval.tv_nsec=0;

		if(ll_end > 0)ll_end=ll_end+now;
		
		timer_settime(timerid[0] , timer_values[0].type ,&timer_values[0].timeout ,NULL);
   }
	
	sigaction(SIGINT,&act,NULL);
}
void settimer(int s)
{
	char cmd[100];
	int i,status,it;
	bzero(cmd,100);
	now=time(NULL);

//	tm1=localtime(&now);
 //	printf("tv_hour =%d,tv_min=%d,tv_sec=%d\n",tm1->tm_hour,tm1->tm_min ,tm1->tm_sec );

  switch (s){
   case SIGUSR1:
			sprintf(cmd,"rteform %s",e_path);
			printf(" *********** settimer  SIGUSR1 \n");
			printf(" *********** %s start \n",cmd);
			system(cmd);
			printf(" *********** %s end \n",cmd);
			if(ll_end > 0 && ll_end <now)
		        {
			   printf("*******执行结束\n ");
			   exit(1);
			}	
		        	
			
		   break;
   case SIGINT:
			for(it=0; it<TIMERS;it++)
				status=timer_delete(timerid[it]);
				printf(" *********** settimer  SIGINT ok \n");
			exit(1);
			break;
              default:
                break;

   }
 }


#include <time.h>
#include <sys/time.h>
#include <iostream.h>

int main(int argc, char *argv[])
{
	time_t      tim;
	struct tm   *time_ptr;

	struct timeval now_time;
	gettimeofday(&now_time, NULL);

	time(&tim);
	time_ptr = localtime(&tim);

	cout<<time_ptr->tm_year<<"��"<<time_ptr->tm_mon<<"��"<<time_ptr->tm_mday<<"��"<<time_ptr->tm_hour<<"ʱ"<<time_ptr->tm_min<<"��"<<time_ptr->tm_sec<<"��"<<endl;
	cout<<tim<<endl;
	cout<<now_time.tv_sec<<endl;

}
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

	cout<<time_ptr->tm_year<<"年"<<time_ptr->tm_mon<<"月"<<time_ptr->tm_mday<<"日"<<time_ptr->tm_hour<<"时"<<time_ptr->tm_min<<"分"<<time_ptr->tm_sec<<"秒"<<endl;
	cout<<tim<<endl;
	cout<<now_time.tv_sec<<endl;

}
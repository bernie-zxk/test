#include <stdio.h>
#include <time.h>

int main()
{


	time_t t = time(NULL);
	struct tm * gmt = gmtime(&t);

	printf("Year: %d\n", gmt->tm_year + 1900);
    printf("Month: %d\n", gmt->tm_mon + 1);
	printf("Day: %d\n", gmt->tm_mday);
    printf("Hour: %d\n", gmt->tm_hour);
	printf("Minute: %d\n", gmt->tm_min);
	printf("Second: %d\n", gmt->tm_sec);

	//ctime
	puts(ctime(&t));//ctime(&t)获得的时间字符串最后会有一个\n

	//asctime
    puts(asctime(gmt));

    //strftime
	char s[0x20];
    strftime(s, 0x20, "%X %x", gmt);

	puts(s);

	struct tm date = {0};
	date.tm_year = 2012 - 1900;
	date.tm_mon = 12 - 1;
	date.tm_mday = 21;
	puts(asctime(&date));
	time_t ti = mktime(&date);
	struct tm * pdate = localtime(&ti);
	printf("%d\n", pdate->tm_wday);
	puts(asctime(pdate));

	return 0;
}

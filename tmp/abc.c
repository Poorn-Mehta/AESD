#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

int main()
{
	struct tm *tm;
	time_t t;
	char str_time[100];
	char str_date[100];

	t = time(0);
	tm = localtime(&t);

	strftime(str_time, sizeof(str_time), "%H %M %S", tm);
	strftime(str_date, sizeof(str_date), "%d %m %Y", tm);
	
	printf("Date (HH-MM-SS): %s\n", str_date);
	printf("Time (DD-MM-YYYY): %s\n", str_time);
	return 0;
}

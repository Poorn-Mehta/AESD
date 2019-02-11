#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>

int main()
{

	struct tm *custom_time_structure;
	time_t total_seconds;
	char time_string[100], date_string[100];
	int32_t presort[256], postsort[256];
	int16_t data_length, i;

	total_seconds = time(0);
	custom_time_structure = localtime(&total_seconds);

	FILE *fp;
	fp = fopen("/usr/bin/problem6.txt", "a+");
	long int ret_status;

	ret_status = getpid();
	fprintf(fp, "\n\n--->>> Current Process ID: %ld", ret_status);

	ret_status = getuid();
	fprintf(fp, "\n\n--->>> Current User ID: %ld", ret_status);

	strftime(time_string, sizeof(time_string), "%H %M %S", custom_time_structure);
	strftime(date_string, sizeof(date_string), "%d %m %Y", custom_time_structure);
	fprintf(fp, "\n\n--->>> Current Date (HH-MM-SS): %s\n", date_string);
	fprintf(fp, "\n\n--->>> Current Time (DD-MM-YYYY): %s\n", time_string);
	fprintf(fp, "\n\n--->>> Invoking 'customcall' system call\n");

	srand(time(0));
	data_length = 256;
	for(i = 0; i < data_length; i ++)
	{
		presort[i] = (rand() % 1000);
		postsort[i] = 0;
	}
	ret_status = syscall(398, &presort, data_length, &postsort); 
	if(ret_status == 0)
	{
		fprintf(fp, "\nSystem call 'customcall' executed correctly\n");
		fprintf(fp, "\n---Data Fed---\n");
		for(i = 0; i < data_length; i ++)	fprintf(fp, "%d ", presort[i]);
		fprintf(fp, "\n---Got Output---\n");
		for(i = 0; i < data_length; i ++)	fprintf(fp, "%d ", postsort[i]);
	}
    	else
	{
		fprintf(fp, "System call 'mycall' did not execute as expected - return %d\n", ret_status);
		perror("Error:\n");
	}
	fclose(fp);         
	return 0;
}

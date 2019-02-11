#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

int main()
{  
	long int ret_status;
	printf("\n\n--->>>Invoking 'customcall' system call<<<---\n");
	int32_t data1[256], data2[256];
	int16_t data_length, i;

	srand(time(0));
	data_length = 256;

	for(i = 0; i < data_length; i ++)
	{
		data1[i] = (rand() % 1000);
		data2[i] = 0;
	}
	ret_status = syscall(398, &data1, data_length, &data2); 
	if(ret_status == 0)
	{
		printf("System call 'customcall' executed correctly\n");
		printf("---Data Fed---\n");
		for(i = 0; i < data_length; i ++)	printf("%d ", data1[i]);
		printf("\n---Got Output---\n");
		for(i = 0; i < data_length; i ++)	printf("%d ", data2[i]);
	}
    	else
	{
		printf("System call 'mycall' did not execute as expected - return %d\n",ret_status);
		perror("Error:\n");
	}   

	for(i = 0; i < data_length; i ++)
	{
		data1[i] = (rand() % 1000);
		data2[i] = 0;
	}
	ret_status = syscall(398, &data1, data_length, &data2); 
	if(ret_status == 0)
	{
		printf("System call 'customcall' executed correctly\n");
		printf("---Data Fed---\n");
		for(i = 0; i < data_length; i ++)	printf("%d ", data1[i]);
		printf("\n---Got Output---\n");
		for(i = 0; i < data_length; i ++)	printf("%d ", data2[i]);
	}
    	else
	{
		printf("System call 'mycall' did not execute as expected - return %d\n",ret_status);
		perror("Error:\n");
	}

	for(i = 0; i < data_length; i ++)
	{
		data1[i] = (rand() % 1000);
		data2[i] = 0;
	}
	ret_status = syscall(398, &data1, data_length, &data2); 
	if(ret_status == 0)
	{
		printf("System call 'customcall' executed correctly\n");
		printf("---Data Fed---\n");
		for(i = 0; i < data_length; i ++)	printf("%d ", data1[i]);
		printf("\n---Got Output---\n");
		for(i = 0; i < data_length; i ++)	printf("%d ", data2[i]);
	}
    	else
	{
		printf("System call 'mycall' did not execute as expected - return %d\n",ret_status);
		perror("Error:\n");
	}
      
	return 0;
}

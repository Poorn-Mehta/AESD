#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <malloc.h> 
#include <stdbool.h>
#include <sys/shm.h>
#include <sys/mman.h>

// Reference: https://stackoverflow.com/questions/5656530/how-to-use-shared-memory-with-linux-in-c

// Necessary variables for time stamps
struct timespec timespec_struct;
struct timeval current_time;

typedef struct{
	char str[20];
	int num;
}info;

info info1,info2, info3, info4;
info *p1 = &info1;
info *p2 = &info2;
info *p3 = &info3;
info *p4 = &info4;

int flag = 0;
char rx_string[100];
void *shmem;

void* create_shared_memory(size_t size) 
{
	// Our memory buffer will be readable and writable:
	int protection = PROT_READ | PROT_WRITE;

	// The buffer will be shared (meaning other processes can access it), but
	// anonymous (meaning third-party processes cannot obtain an address for it),
	// so only this process and its children will be able to use it:
	int visibility = MAP_ANONYMOUS | MAP_SHARED;

	// The remaining parameters to `mmap()` are not important for this use case,
	// but the manpage for `mmap` explains their purpose.
	return mmap(NULL, size, protection, visibility, 0, 0);
}

void Command_LED(int led, int id)
{
	static FILE *fptr;
	fptr = fopen("shmem.txt","a");
	gettimeofday(&current_time, 0);
	if(id)
	{
		if(led) fprintf(fptr,"\n<%lu.%06lu> LED is Turned ON by Parent\n", current_time.tv_sec, current_time.tv_usec);
		else  fprintf(fptr,"\n<%lu.%06lu> LED is Turned OFF by Parent\n", current_time.tv_sec, current_time.tv_usec);
	}
	else
	{
		if(led) fprintf(fptr,"\n<%lu.%06lu> LED is Turned ON by Child\n", current_time.tv_sec, current_time.tv_usec);
		else  fprintf(fptr,"\n<%lu.%06lu> LED is Turned OFF by Child\n", current_time.tv_sec, current_time.tv_usec);

	}
	fclose(fptr);
}

void sig_exit(int value)
{
    static FILE *fptr;
    flag = 1;
    printf("\nSIGTERM Received, Exiting...\n");
    fptr = fopen("shmem.txt","a");
    gettimeofday(&current_time, 0);
    fprintf(fptr,"\n<%lu.%06lu> Exiting... Closing Program and File Pointers\n", current_time.tv_sec, current_time.tv_usec);
    fclose(fptr);
}

void sigact_setup(void)
{
   struct sigaction sig_act;
   memset(&sig_act, 0, sizeof(struct sigaction));
   sig_act.sa_handler = &sig_exit;
   sigaction(SIGTERM, &sig_act, 0);

}

void Child_Function(char *argchar, int lstate)
{
	static FILE *fptr;
	fptr = fopen("shmem.txt","a");
	fprintf(fptr,"\nChild PID:%d\n",getpid());

	// read
	memcpy(p3, shmem, sizeof(info));
	gettimeofday(&current_time, 0);
	if(p3->num == 1)
	{
		Command_LED(1, 0);
		fprintf(fptr,"<%lu.%06lu> Child: Read String *%s* LED State: Turn ON\n", current_time.tv_sec, current_time.tv_usec, p3->str);
	}
	else if(p3->num == 2)
	{
		Command_LED(0, 0);
		fprintf(fptr,"<%lu.%06lu> Child: Read String *%s* LED State: Turn OFF\n", current_time.tv_sec, current_time.tv_usec, p3->str);
	}
	else	fprintf(fptr,"<%lu.%06lu> Child: Read String *%s* LED State: No Change\n", current_time.tv_sec, current_time.tv_usec, p3->str);

	p2->num = lstate;
	strcpy(p2->str, argchar);
	// write
	gettimeofday(&current_time, 0);
	if(p2->num == 1)		fprintf(fptr,"<%lu.%06lu> Child: Wrote String *%s* LED Command: Turn ON\n", current_time.tv_sec, current_time.tv_usec, p2->str);
	else if(p2->num == 2)	fprintf(fptr,"<%lu.%06lu> Child: Wrote String *%s* LED Command: Turn OFF\n", current_time.tv_sec, current_time.tv_usec, p2->str);
	else	fprintf(fptr,"<%lu.%06lu> Child: Wrote String *%s* LED State: No Change\n", current_time.tv_sec, current_time.tv_usec, p2->str);
	memcpy(shmem, p2, sizeof(info));

	sleep(1);

	fclose(fptr);
}

void Parent_Function(char *argchar, int lstate)
{
	static FILE *fptr;
	fptr = fopen("shmem.txt","a");
	fprintf(fptr,"\nParent PID:%d\n",getpid());

	p1->num = lstate;
	strcpy(p1->str, argchar);
	// write
	gettimeofday(&current_time, 0);
	if(p1->num == 1)		fprintf(fptr,"<%lu.%06lu> Parent: Wrote String *%s* LED Command: Turn ON\n", current_time.tv_sec, current_time.tv_usec, p1->str);
	else if(p1->num == 2)	fprintf(fptr,"<%lu.%06lu> Parent: Wrote String *%s* LED Command: Turn OFF\n", current_time.tv_sec, current_time.tv_usec, p1->str);
	else	fprintf(fptr,"<%lu.%06lu> Parent: Wrote String *%s* LED State: No Change\n", current_time.tv_sec, current_time.tv_usec, p1->str);
	memcpy(shmem, p1, sizeof(info));

	sleep(1);

	// read
	memcpy(p4, shmem, sizeof(info));
	gettimeofday(&current_time, 0);
	if(p4->num == 1)
	{
		Command_LED(1, 1);
		fprintf(fptr,"<%lu.%06lu> Parent: Read String *%s* LED State: Turn ON\n", current_time.tv_sec, current_time.tv_usec, p4->str);
	}
	else if(p4->num == 2)
	{
		Command_LED(0, 1);
		fprintf(fptr,"<%lu.%06lu> Parent: Read String *%s* LED State: Turn OFF\n", current_time.tv_sec, current_time.tv_usec, p4->str);
	}
	else	fprintf(fptr,"<%lu.%06lu> Parent: Read String *%s* LED State: No Change\n", current_time.tv_sec, current_time.tv_usec, p4->str);

	fclose(fptr);
}

int main(void)
{
	static FILE *fptr;
	clock_gettime(CLOCK_REALTIME, &timespec_struct);
	printf("\nProcess ID:%d\n", getpid());

	fptr = fopen("shmem.txt","w+");
	fprintf(fptr, "Process ID:%d\n", getpid());

	sigact_setup();

	shmem = create_shared_memory(4096);

	gettimeofday(&current_time, 0);
	fprintf(fptr, "Time Stamp: *%lu.%06lu*\n", current_time.tv_sec, current_time.tv_usec);
	fprintf(fptr,"IPC using Shared Memory\n");
	fclose(fptr);

	int pid = fork();
	if (pid == 0) 
	{
		printf("Child Start\n");
		Child_Function("C to P 1", 1); 
		sleep(1);
		Child_Function("C to P 2", 0);
		sleep(1);
		Child_Function("C to P 3", 2);
		sleep(1);
		Child_Function("C to P 4", 1);
		sleep(1);
		Child_Function("C to P 5", 0);
		sleep(1);
		Child_Function("C to P 6", 0);
		sleep(1);
		Child_Function("C to P 7", 2);
		sleep(1);
		Child_Function("C to P 8", 1);
		sleep(1);
		Child_Function("C to P 9", 0);
		sleep(1);
		Child_Function("C to P 10", 2);
		printf("Child Done\n");
	}
	else
	{
		printf("Parent Start\n");
		Parent_Function("P to C 1", 0);
		sleep(1);
		Parent_Function("P to C 2", 2);
		sleep(1);
		Parent_Function("P to C 3", 1);
		sleep(1);
		Parent_Function("P to C 4", 0);
		sleep(1);
		Parent_Function("P to C 5", 2);
		sleep(1);
		Parent_Function("P to C 6", 1); 
		sleep(1);
		Parent_Function("P to C 7", 0);
		sleep(1);
		Parent_Function("P to C 8", 2);
		sleep(1);
		Parent_Function("P to C 9", 1);
		sleep(1);
		Parent_Function("P to C 10", 0);
		printf("Parent Done\n");
	}

	while(flag == 0);

	fclose(fptr);
}

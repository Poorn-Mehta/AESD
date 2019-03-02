#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <errno.h>
#define queue_name 		"/custom_q"
#define queue_size		8


// for time
struct timespec timespec_struct;
struct timeval current_time;

// flag for sigterm
int flag = 0;

// global file pointer
FILE *fptr;

// data structure
typedef struct
{
  char str[20];
  int num;
}info;

info info1, info2;
info *p1 = &info1;
info *p2 = &info2;

// led controller example function
void Command_LED(int led)
{
	gettimeofday(&current_time, 0);
	if(led) fprintf(fptr,"<%lu.%06lu> LED is Turned ON by Second Queue\n", current_time.tv_sec, current_time.tv_usec);
	else  fprintf(fptr,"<%lu.%06lu> LED is Turned OFF by Second Queue\n", current_time.tv_sec, current_time.tv_usec);
}

// sigterm handler
void sig_exit(int value)
{
	flag = 1;
	printf("\nSIGTERM Received, Exiting...\n");
	gettimeofday(&current_time, 0);
	fprintf(fptr,"\n<%lu.%06lu>Second Queue Exiting... Closing Program and File Pointers\n", current_time.tv_sec, current_time.tv_usec);
}

// sigterm setup
void sigact_setup(void)
{
   struct sigaction sig_act;
   memset(&sig_act, 0, sizeof(struct sigaction));
   sig_act.sa_handler = &sig_exit;
   sigaction(SIGTERM, &sig_act, 0);

}

int main(void)
{
	// setting up clock for time stamps
	clock_gettime(CLOCK_REALTIME, &timespec_struct);

	printf("\nProcess ID:%d\n", getpid());

	sigact_setup();

	int info_in, info_out;

	// Queue setup
	mqd_t custom_queue;
	struct mq_attr custom_queue_attr;
	custom_queue_attr.mq_maxmsg = queue_size;
	custom_queue_attr.mq_msgsize = sizeof(info);

	custom_queue = mq_open(queue_name, O_CREAT | O_RDWR, 0666, &custom_queue_attr);		
	if(custom_queue == -1)
	{
		printf("\nQueue opening error");
		return(0);
	}

	
	// POSIX Queue init complete, main process start
	fptr = fopen("plog.txt","a");
	if(fptr == 0)
	{
		printf("\nError while opening file for logging\n");
		return 0;
	}

	fprintf(fptr, "\n\nSecond Queue Start with PID: %d\n", getpid());
	gettimeofday(&current_time, 0);
	fprintf(fptr, "Time Stamp: *%lu.%06lu*\n", current_time.tv_sec, current_time.tv_usec);
    	fprintf(fptr,"IPC using POSIX Queue\n");
	fprintf(fptr,"No special resources being utilized\n");

	// tx1, rx1 - both contains two messages - 1 string and 1 integer
	strcpy(p1->str, "C to S 1");
	p1->num = 1;
	gettimeofday(&current_time, 0);
	fprintf(fptr,"\n<%lu.%06lu> Sending to First Queue *%s* and LED Command: Turn ON", current_time.tv_sec, current_time.tv_usec, p1->str);
	info_out = mq_send(custom_queue,p1,sizeof(info),0);    	
	if (info_out < 0)
	{
		printf("\nQueue Writing Failed\n");
		return(0);
	}
	gettimeofday(&current_time, 0);
	info_in = mq_receive(custom_queue,p2,sizeof(info),0);
	if(info_in < 0)
	{
		printf("\nQueue Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from First Queue *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED ON\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED OFF\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged\n", current_time.tv_sec, current_time.tv_usec);

	// tx2, rx2
	strcpy(p1->str, "C to S 2");
	p1->num = 2;
	gettimeofday(&current_time, 0);
	fprintf(fptr,"\n<%lu.%06lu> Sending to First Queue *%s* and LED Command: Turn OFF", current_time.tv_sec, current_time.tv_usec, p1->str);
	info_out = mq_send(custom_queue,p1,sizeof(info),0);    	
	if (info_out < 0)
	{
		printf("\nQueue Writing Failed\n");
		return(0);
	}
	gettimeofday(&current_time, 0);
	info_in = mq_receive(custom_queue,p2,sizeof(info),0);
	if(info_in < 0)
	{
		printf("\nQueue Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from First Queue *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED ON\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED OFF\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged\n", current_time.tv_sec, current_time.tv_usec);

	// tx3, rx3
	strcpy(p1->str, "C to S 3");
	p1->num = 0;
	gettimeofday(&current_time, 0);
	fprintf(fptr,"\n<%lu.%06lu> Sending to First Queue *%s* and LED Command: No Change", current_time.tv_sec, current_time.tv_usec, p1->str);
	info_out = mq_send(custom_queue,p1,sizeof(info),0);    	
	if (info_out < 0)
	{
		printf("\nQueue Writing Failed\n");
		return(0);
	}
	gettimeofday(&current_time, 0);
	info_in = mq_receive(custom_queue,p2,sizeof(info),0);
	if(info_in < 0)
	{
		printf("\nQueue Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from First Queue *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED ON\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED OFF\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged\n", current_time.tv_sec, current_time.tv_usec);

	// tx4, rx4
	strcpy(p1->str, "C to S 4");
	p1->num = 2;
	gettimeofday(&current_time, 0);
	fprintf(fptr,"\n<%lu.%06lu> Sending to First Queue *%s* and LED Command: Turn OFF", current_time.tv_sec, current_time.tv_usec, p1->str);
	info_out = mq_send(custom_queue,p1,sizeof(info),0);    	
	if (info_out < 0)
	{
		printf("\nQueue Writing Failed\n");
		return(0);
	}
	gettimeofday(&current_time, 0);
	info_in = mq_receive(custom_queue,p2,sizeof(info),0);
	if(info_in < 0)
	{
		printf("\nQueue Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from First Queue *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED ON\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED OFF\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged\n", current_time.tv_sec, current_time.tv_usec);

	// tx5, rx5
	strcpy(p1->str, "C to S 5");
	p1->num = 1;
	gettimeofday(&current_time, 0);
	fprintf(fptr,"\n<%lu.%06lu> Sending to First Queue *%s* and LED Command: Turn ON", current_time.tv_sec, current_time.tv_usec, p1->str);
	info_out = mq_send(custom_queue,p1,sizeof(info),0);    	
	if (info_out < 0)
	{
		printf("\nQueue Writing Failed\n");
		return(0);
	}
	gettimeofday(&current_time, 0);
	info_in = mq_receive(custom_queue,p2,sizeof(info),0);
	if(info_in < 0)
	{
		printf("\nQueue Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from First Queue *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED ON\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED OFF\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged\n", current_time.tv_sec, current_time.tv_usec);

	// tx6, rx6
	strcpy(p1->str, "C to S 6");
	p1->num = 0;
	gettimeofday(&current_time, 0);
	fprintf(fptr,"\n<%lu.%06lu> Sending to First Queue *%s* and LED Command: No Change", current_time.tv_sec, current_time.tv_usec, p1->str);
	info_out = mq_send(custom_queue,p1,sizeof(info),0);    	
	if (info_out < 0)
	{
		printf("\nQueue Writing Failed\n");
		return(0);
	}
	gettimeofday(&current_time, 0);
	info_in = mq_receive(custom_queue,p2,sizeof(info),0);
	if(info_in < 0)
	{
		printf("\nQueue Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from First Queue *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED ON\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED OFF\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged\n", current_time.tv_sec, current_time.tv_usec);

	// tx7, rx7
	strcpy(p1->str, "C to S 7");
	p1->num = 2;
	gettimeofday(&current_time, 0);
	fprintf(fptr,"\n<%lu.%06lu> Sending to First Queue *%s* and LED Command: Turn OFF", current_time.tv_sec, current_time.tv_usec, p1->str);
	info_out = mq_send(custom_queue,p1,sizeof(info),0);    	
	if (info_out < 0)
	{
		printf("\nQueue Writing Failed\n");
		return(0);
	}
	gettimeofday(&current_time, 0);
	info_in = mq_receive(custom_queue,p2,sizeof(info),0);
	if(info_in < 0)
	{
		printf("\nQueue Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from First Queue *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED ON\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED OFF\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged\n", current_time.tv_sec, current_time.tv_usec);

	// tx8, rx8
	strcpy(p1->str, "C to S 8");
	p1->num = 1;
	gettimeofday(&current_time, 0);
	fprintf(fptr,"\n<%lu.%06lu> Sending to First Queue *%s* and LED Command: Turn ON", current_time.tv_sec, current_time.tv_usec, p1->str);
	info_out = mq_send(custom_queue,p1,sizeof(info),0);    	
	if (info_out < 0)
	{
		printf("\nQueue Writing Failed\n");
		return(0);
	}
	gettimeofday(&current_time, 0);
	info_in = mq_receive(custom_queue,p2,sizeof(info),0);
	if(info_in < 0)
	{
		printf("\nQueue Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from First Queue *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED ON\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED OFF\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged\n", current_time.tv_sec, current_time.tv_usec);

	// tx9, rx9
	strcpy(p1->str, "C to S 9");
	p1->num = 1;
	gettimeofday(&current_time, 0);
	fprintf(fptr,"\n<%lu.%06lu> Sending to First Queue *%s* and LED Command: Turn ON", current_time.tv_sec, current_time.tv_usec, p1->str);
	info_out = mq_send(custom_queue,p1,sizeof(info),0);    	
	if (info_out < 0)
	{
		printf("\nQueue Writing Failed\n");
		return(0);
	}
	gettimeofday(&current_time, 0);
	info_in = mq_receive(custom_queue,p2,sizeof(info),0);
	if(info_in < 0)
	{
		printf("\nQueue Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from First Queue *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED ON\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED OFF\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged\n", current_time.tv_sec, current_time.tv_usec);

	// tx10, rx10
	strcpy(p1->str, "C to S 10");
	p1->num = 0;
	gettimeofday(&current_time, 0);
	fprintf(fptr,"\n<%lu.%06lu> Sending to First Queue *%s* and LED Command: No Change", current_time.tv_sec, current_time.tv_usec, p1->str);
	info_out = mq_send(custom_queue,p1,sizeof(info),0);    	
	if (info_out < 0)
	{
		printf("\nQueue Writing Failed\n");
		return(0);
	}
	gettimeofday(&current_time, 0);
	info_in = mq_receive(custom_queue,p2,sizeof(info),0);
	if(info_in < 0)
	{
		printf("\nQueue Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from First Queue *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED ON\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from First Queue: LED OFF\n", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged\n", current_time.tv_sec, current_time.tv_usec);


	// Wait for termination signal
	while(flag == 0);

	fclose(fptr);
	mq_close(custom_queue);
	mq_unlink(queue_name);								
	return 0;
}






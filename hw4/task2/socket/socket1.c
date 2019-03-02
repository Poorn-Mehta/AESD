#include <stdio.h>
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
#include <stdint.h> 
#include <malloc.h> 

#define PORT 2000

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
	gettimeofday(&current_time, NULL);
	if(led) fprintf(fptr,"\n<%lu.%06lu> LED is Turned ON by Server", current_time.tv_sec, current_time.tv_usec);
	else  fprintf(fptr,"\n<%lu.%06lu> LED is Turned OFF by Server", current_time.tv_sec, current_time.tv_usec);
}

// sigterm handler
void sig_exit(int value)
{
	flag = 1;
	printf("\nSIGTERM Received, Exiting...\n");
	gettimeofday(&current_time, NULL);
	fprintf(fptr,"\n<%lu.%06lu>Server Exiting... Closing Socket and File Pointers\n", current_time.tv_sec, current_time.tv_usec);
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

	int new_socket, custom_socket, info_in, info_out;
	struct sockaddr_in custom_server;

	// socket init on server end
	new_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(new_socket < 0)
	{
		printf("\nSocket Creation Failed\n");
		return(0);
	}

	custom_server.sin_family = AF_INET;
	custom_server.sin_addr.s_addr = INADDR_ANY;
	custom_server.sin_port = htons(PORT);

	if(bind(new_socket, (struct sockaddr *)&custom_server, sizeof(custom_server)) < 0)
	{
		printf("\nSocket Binding Failed\n");
		return(0);

	}

	if(listen(new_socket, 5) < 0)
	{
		printf("\nSocket Listening Failed\n");
		return(0);
	}

	custom_socket = accept(new_socket, (struct sockaddr *)0, 0);
	if(custom_socket < 0)
	{
		printf("\nSocket Accepting Failed\n");
		return(0);
	}

	// socket init complete, main process start
	fptr = fopen("slog.txt","w+");
	if(fptr == 0)
	{
		printf("\nError while opening file for logging\n");
		return 0;
	}

	fprintf(fptr, "Server Start with PID: %d\n", getpid());
	gettimeofday(&current_time, NULL);
	fprintf(fptr, "Time Stamp: *%lu.%06lu*\n", current_time.tv_sec, current_time.tv_usec);
    	fprintf(fptr,"IPC using Socket\n");

	// rx1, tx1 - both contains two messages - 1 string and 1 integer
	gettimeofday(&current_time, NULL);
	info_in = read(custom_socket,p2,sizeof(info));
	if(info_in < 0)
	{
		printf("\nSocket Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from Client *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED ON", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED OFF", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged", current_time.tv_sec, current_time.tv_usec);
	strcpy(p1->str, "S to C 1");
	p1->num = 0;
	gettimeofday(&current_time, NULL);
	fprintf(fptr,"\n<%lu.%06lu> Sending to Client *%s* and LED Command: No Change\n", current_time.tv_sec, current_time.tv_usec, p1->str);
    	info_out = write(custom_socket,p1,sizeof(info));
	if (info_out < 0)
	{
		printf("\nSocket Writing Failed\n");
		return(0);
	}

	// rx2, tx2
	gettimeofday(&current_time, NULL);
	info_in = read(custom_socket,p2,sizeof(info));
	if(info_in < 0)
	{
		printf("\nSocket Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from Client *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED ON", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED OFF", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged", current_time.tv_sec, current_time.tv_usec);
	strcpy(p1->str, "S to C 2");
	p1->num = 2;
	gettimeofday(&current_time, NULL);
	fprintf(fptr,"\n<%lu.%06lu> Sending to Client *%s* and LED Command: Turn OFF\n", current_time.tv_sec, current_time.tv_usec, p1->str);
    	info_out = write(custom_socket,p1,sizeof(info));
	if (info_out < 0)
	{
		printf("\nSocket Writing Failed\n");
		return(0);
	}

	// rx3, tx3
	gettimeofday(&current_time, NULL);
	info_in = read(custom_socket,p2,sizeof(info));
	if(info_in < 0)
	{
		printf("\nSocket Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from Client *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED ON", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED OFF", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged", current_time.tv_sec, current_time.tv_usec);
	strcpy(p1->str, "S to C 3");
	p1->num = 1;
	gettimeofday(&current_time, NULL);
	fprintf(fptr,"\n<%lu.%06lu> Sending to Client *%s* and LED Command: Turn ON\n", current_time.tv_sec, current_time.tv_usec, p1->str);
    	info_out = write(custom_socket,p1,sizeof(info));
	if (info_out < 0)
	{
		printf("\nSocket Writing Failed\n");
		return(0);
	}

	// rx4, tx4
	gettimeofday(&current_time, NULL);
	info_in = read(custom_socket,p2,sizeof(info));
	if(info_in < 0)
	{
		printf("\nSocket Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from Client *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED ON", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED OFF", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged", current_time.tv_sec, current_time.tv_usec);
	strcpy(p1->str, "S to C 4");
	p1->num = 0;
	gettimeofday(&current_time, NULL);
	fprintf(fptr,"\n<%lu.%06lu> Sending to Client *%s* and LED Command: No Change\n", current_time.tv_sec, current_time.tv_usec, p1->str);
    	info_out = write(custom_socket,p1,sizeof(info));
	if (info_out < 0)
	{
		printf("\nSocket Writing Failed\n");
		return(0);
	}

	// rx5, tx5
	gettimeofday(&current_time, NULL);
	info_in = read(custom_socket,p2,sizeof(info));
	if(info_in < 0)
	{
		printf("\nSocket Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from Client *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED ON", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED OFF", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged", current_time.tv_sec, current_time.tv_usec);
	strcpy(p1->str, "S to C 5");
	p1->num = 1;
	gettimeofday(&current_time, NULL);
	fprintf(fptr,"\n<%lu.%06lu> Sending to Client *%s* and LED Command: Turn ON\n", current_time.tv_sec, current_time.tv_usec, p1->str);
    	info_out = write(custom_socket,p1,sizeof(info));
	if (info_out < 0)
	{
		printf("\nSocket Writing Failed\n");
		return(0);
	}

	// rx6, tx6
	gettimeofday(&current_time, NULL);
	info_in = read(custom_socket,p2,sizeof(info));
	if(info_in < 0)
	{
		printf("\nSocket Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from Client *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED ON", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED OFF", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged", current_time.tv_sec, current_time.tv_usec);
	strcpy(p1->str, "S to C 6");
	p1->num = 0;
	gettimeofday(&current_time, NULL);
	fprintf(fptr,"\n<%lu.%06lu> Sending to Client *%s* and LED Command: No Change\n", current_time.tv_sec, current_time.tv_usec, p1->str);
    	info_out = write(custom_socket,p1,sizeof(info));
	if (info_out < 0)
	{
		printf("\nSocket Writing Failed\n");
		return(0);
	}

	// rx7, tx7
	gettimeofday(&current_time, NULL);
	info_in = read(custom_socket,p2,sizeof(info));
	if(info_in < 0)
	{
		printf("\nSocket Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from Client *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED ON", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED OFF", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged", current_time.tv_sec, current_time.tv_usec);
	strcpy(p1->str, "S to C 7");
	p1->num = 1;
	gettimeofday(&current_time, NULL);
	fprintf(fptr,"\n<%lu.%06lu> Sending to Client *%s* and LED Command: Turn ON\n", current_time.tv_sec, current_time.tv_usec, p1->str);
    	info_out = write(custom_socket,p1,sizeof(info));
	if (info_out < 0)
	{
		printf("\nSocket Writing Failed\n");
		return(0);
	}

	// rx8, tx8
	gettimeofday(&current_time, NULL);
	info_in = read(custom_socket,p2,sizeof(info));
	if(info_in < 0)
	{
		printf("\nSocket Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from Client *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED ON", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED OFF", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged", current_time.tv_sec, current_time.tv_usec);
	strcpy(p1->str, "S to C 8");
	p1->num = 1;
	gettimeofday(&current_time, NULL);
	fprintf(fptr,"\n<%lu.%06lu> Sending to Client *%s* and LED Command: Turn ON\n", current_time.tv_sec, current_time.tv_usec, p1->str);
    	info_out = write(custom_socket,p1,sizeof(info));
	if (info_out < 0)
	{
		printf("\nSocket Writing Failed\n");
		return(0);
	}

	// rx9, tx9
	gettimeofday(&current_time, NULL);
	info_in = read(custom_socket,p2,sizeof(info));
	if(info_in < 0)
	{
		printf("\nSocket Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from Client *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED ON", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED OFF", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged", current_time.tv_sec, current_time.tv_usec);
	strcpy(p1->str, "S to C 9");
	p1->num = 2;
	gettimeofday(&current_time, NULL);
	fprintf(fptr,"\n<%lu.%06lu> Sending to Client *%s* and LED Command: Turn OFF\n", current_time.tv_sec, current_time.tv_usec, p1->str);
    	info_out = write(custom_socket,p1,sizeof(info));
	if (info_out < 0)
	{
		printf("\nSocket Writing Failed\n");
		return(0);
	}

	// rx10, tx10
	gettimeofday(&current_time, NULL);
	info_in = read(custom_socket,p2,sizeof(info));
	if(info_in < 0)
	{
		printf("\nSocket Reading Failed\n");
		return(0);
	}
	fprintf(fptr,"\n<%lu.%06lu> String from Client *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
	if(p2->num == 1)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED ON", current_time.tv_sec, current_time.tv_usec);
		Command_LED(1);
	}
	else if(p2->num == 2)
	{
		fprintf(fptr,"\n<%lu.%06lu> Command from Client: LED OFF", current_time.tv_sec, current_time.tv_usec);
		Command_LED(0);
	}
	else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged", current_time.tv_sec, current_time.tv_usec);
	strcpy(p1->str, "S to C 10");
	p1->num = 0;
	gettimeofday(&current_time, NULL);
	fprintf(fptr,"\n<%lu.%06lu> Sending to Client *%s* and LED Command: No Change\n", current_time.tv_sec, current_time.tv_usec, p1->str);
    	info_out = write(custom_socket,p1,sizeof(info));
	if (info_out < 0)
	{
		printf("\nSocket Writing Failed\n");
		return(0);
	}

	// Wait for termination signal
	while(flag == 0);

	fclose(fptr);
	return(0);
}

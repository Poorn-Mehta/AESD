#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <sys/types.h>

// Necessary variables for time stamps
struct timespec timespec_struct;
struct timeval current_time;

typedef struct {
  char str[20];
  int num;
} info;

info info2;
info *info1 = &info2;

int flag = 0;

int pipe1_p_to_c[2], pipe2_p_to_c[2], ret_val, new_process_id;
char rx_string[100];

void Command_LED(int led, int id)
{
	static FILE *fptr;
	fptr = fopen("logger.txt","a");
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

void Parent_Function(char *argchar, int lstate)
{
	static FILE *fptr;
	fptr = fopen("logger.txt","a");
	fprintf(fptr,"\nParent PID:%d\n",getpid());

	info1->num = lstate;
	strcpy(info1->str, argchar);
	gettimeofday(&current_time, 0);
	if(lstate == 1)		fprintf(fptr,"<%lu.%06lu> Parent: Transmitting String *%s* LED Command: Turn ON\n", current_time.tv_sec, current_time.tv_usec, info1->str);
	else if(lstate == 2)	fprintf(fptr,"<%lu.%06lu> Parent: Transmitting String *%s* LED Command: Turn OFF\n", current_time.tv_sec, current_time.tv_usec, info1->str);
	else	fprintf(fptr,"<%lu.%06lu> Parent: Transmitting String *%s* LED State: No Change\n", current_time.tv_sec, current_time.tv_usec, info1->str);
	write(pipe1_p_to_c[1],info1, sizeof(info));

	read(pipe2_p_to_c[0], info1, sizeof(info));
	strcpy(rx_string, info1->str);
	gettimeofday(&current_time, 0);
	if(info1->num == 1)
	{
		Command_LED(1, 1);
		fprintf(fptr,"<%lu.%06lu> Parent: Receiving String *%s* LED State: Turn ON\n", current_time.tv_sec, current_time.tv_usec, rx_string);
	}
	else if(info1->num == 2)
	{
		Command_LED(0, 1);
		fprintf(fptr,"<%lu.%06lu> Parent: Receiving String *%s* LED State: Turn OFF\n", current_time.tv_sec, current_time.tv_usec, rx_string);
	}
	else	fprintf(fptr,"<%lu.%06lu> Parent: Receiving String *%s* LED State: No Change\n", current_time.tv_sec, current_time.tv_usec, rx_string);
	fclose(fptr);
}

void Child_Function(char *argchar, int lstate)
{
	static FILE *fptr;
	fptr = fopen("logger.txt","a");
	fprintf(fptr,"\nChild PID:%d\n",getpid());

	info1->num = lstate;
	strcpy(info1->str, argchar);
	gettimeofday(&current_time, 0);
	if(lstate == 1)		fprintf(fptr,"<%lu.%06lu> Child: Transmitting String *%s* LED Command: Turn ON\n", current_time.tv_sec, current_time.tv_usec, info1->str);
	else if(lstate == 2)	fprintf(fptr,"<%lu.%06lu> Child: Transmitting String *%s* LED Command: Turn OFF\n", current_time.tv_sec, current_time.tv_usec, info1->str);
	else	fprintf(fptr,"<%lu.%06lu> Child: Transmitting String *%s* LED State: No Change\n", current_time.tv_sec, current_time.tv_usec, info1->str);
	write(pipe2_p_to_c[1], info1, sizeof(info));

	read(pipe1_p_to_c[0], info1, sizeof(info));
	strcpy(rx_string, info1->str);
	gettimeofday(&current_time, 0);
	if(info1->num == 1)
	{
		Command_LED(1, 0);
		fprintf(fptr,"<%lu.%06lu> Child: Receiving String *%s* LED State: Turn ON\n", current_time.tv_sec, current_time.tv_usec, rx_string);
	}
	else if(info1->num == 2)
	{
		Command_LED(0, 0);
		fprintf(fptr,"<%lu.%06lu> Child: Receiving String *%s* LED State: Turn OFF\n", current_time.tv_sec, current_time.tv_usec, rx_string);
	}
	else	fprintf(fptr,"<%lu.%06lu> Child: Receiving String *%s* LED State: No Change\n", current_time.tv_sec, current_time.tv_usec, rx_string);

	fclose(fptr);
}

void sig_exit(int value)
{
    static FILE *fptr;
    flag = 1;
    printf("\nSIGTERM Received, Exiting...\n");
    fptr = fopen("logger.txt","a");
    gettimeofday(&current_time, 0);
    fprintf(fptr,"\n<%lu.%06lu> Exiting... Closing Pipes and File Pointers\n", current_time.tv_sec, current_time.tv_usec);
    fclose(fptr);
}

void sigact_setup(void)
{
   struct sigaction sig_act;
   memset(&sig_act, 0, sizeof(struct sigaction));
   sig_act.sa_handler = &sig_exit;
   sigaction(SIGTERM, &sig_act, 0);

}

int main()
{
    info1->num = 0;
    static FILE *fptr;
    clock_gettime(CLOCK_REALTIME, &timespec_struct);
    printf("\nProcess ID:%d\n", getpid());

    sigact_setup();

    fptr = fopen("logger.txt","w+");
    fprintf(fptr, "Process ID:%d\n", getpid());

   ret_val = pipe(pipe1_p_to_c);
   if (ret_val < 0)
   {
      printf("Pipe 1 Failed\n");
      return 1;
   }

   ret_val = pipe(pipe2_p_to_c);
   if (ret_val < 0)
   {
      printf("Pipe 2 Failed\n");
      return 1;
   }

    gettimeofday(&current_time, 0);
    fprintf(fptr, "Time Stamp: *%lu.%06lu*\n", current_time.tv_sec, current_time.tv_usec);
    fprintf(fptr,"IPC using Pipes\n");
    fprintf(fptr,"No special resources being utilized\n");
    fclose(fptr);

    new_process_id = fork();

    if (new_process_id == 0)
    {
        Child_Function("C to P 1", 1);
        Child_Function("C to P 2", 0);
        Child_Function("C to P 3", 2);
        Child_Function("C to P 4", 1);
        Child_Function("C to P 5", 0);
        Child_Function("C to P 6", 1);
        Child_Function("C to P 7", 0);
        Child_Function("C to P 8", 2);
        Child_Function("C to P 9", 1);
        Child_Function("C to P 10", 0);
    }
    else
    {
        Parent_Function("P to C 1", 0);
        Parent_Function("P to C 2", 2);
        Parent_Function("P to C 3", 1);
        Parent_Function("P to C 4", 0);
        Parent_Function("P to C 5", 2);
        Parent_Function("P to C 6", 0);
        Parent_Function("P to C 7", 2);
        Parent_Function("P to C 8", 1);
        Parent_Function("P to C 9", 0);
        Parent_Function("P to C 10", 2);
    }

     while(flag == 0);

     fclose(fptr);
     close(pipe1_p_to_c[0]);
     close(pipe1_p_to_c[1]);
     close(pipe2_p_to_c[0]);
     close(pipe2_p_to_c[1]);

     return 0;
}

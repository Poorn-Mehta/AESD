#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <malloc.h>
#include <unistd.h>

// Necessary variables for time stamps
struct timespec timespec_struct;
struct timeval current_time;

// This will indicate whether any valid user signal has been received or not
int flag = 0;

// Two child threads
pthread_t child_thread1, child_thread2;

// Structure for passing file name. Can add other parameters for future use
struct thread_info
{
  char *string;
};

// For child thread 1
void *child1_function(void *thread_input)
{
	// Local file pointers
	static FILE *fptr = 0;
	static FILE *fptr2;

	// Opening file for logging
	fptr2 = fopen(((struct thread_info *)thread_input)->string, "a");

	// Logging information
	gettimeofday(&current_time, 0);
	fprintf(fptr2, "\n--->>> Child 1 Started at *%lu.%06lu* <<<---\n", current_time.tv_sec, current_time.tv_usec);
	fprintf(fptr2, "Child 1 Identities\nLinux Thread ID: *%ld* POSIX Thread ID: *%lu*", syscall(SYS_gettid), pthread_self());
	fprintf(fptr2, "\nTask: To process an input text file and display number of characters repetitions less than 100\n");

	// Variables for character counting algorithm
	// It is a very simple method - basically taking an array with 26 16bit elements, using 8 most significant bits to store
	// ASCII code of the uppercase alphabet, and the lower 8 bits used for storing repetitions of that character (even in lower case)
	// 8 bits are sufficient since we only need less than 100 repetitions in the output
  	uint16_t count[26], i, cnt;
  	char ip, cmp;

	// Input file opened for reading
  	fptr = fopen("gdb.txt","r");

	// Initializing with correct ASCII values and count bits set to 0
  	for(i = 'A'; i < ('A' + 26); i ++)	count[(i-'A')] = (i<<8);

	// Loop till the end of file is reached
  	while(1)
  	{
		// Input character
  		ip = fgetc(fptr);
  		if(feof(fptr))	break;

		// Compare input character to all Uppercase and Lowercase alphabets
  		for(i = 0; i < 26; i ++)
  		{
  			cmp = count[i] >> 8;
  			if((ip == cmp) || (ip == (cmp + 32)))
  			{
  				if((count[i] & 0x00FF) < 0x00FF)	count[i] += 1;	// Capping at 255, to keep ASCII codes safe
  			}
  		}
  	}

	// Print output
  	for(i = 0; i < 26; i ++)
  	{
  		cmp = count[i] >> 8;
  		cnt = count[i] & 0x00FF;
	        if(cnt < 100)  fprintf(fptr2, "Char %c Count %d\n", cmp, cnt);
  	}

	// Logging
	gettimeofday(&current_time, 0);
	fprintf(fptr2, "\n--->>> Child 1 Exited at *%lu.%06lu* <<<---\n", current_time.tv_sec, current_time.tv_usec);
	fprintf(fptr2, "Exit Reason: Process Completed\n");

	// Closing files and exiting thread
	fclose(fptr2);
	if(fptr)	fclose(fptr);
	pthread_exit(0);
}

void cpu_usage(int value)
{
	// Simply passes signal value to the main child thread 2 handler
	flag = value;
}

// Child thread 2 function integrating CPU Usage functionality
// Prints CPU Usage details (copied from /proc/stat) and writes to cpu_stat_sourceon logger file
// Raises a flag and prints exit messages on receipt of user signal
void *child2_function(void *thread_input)
{
	// Local file pointer
	static FILE *fptr, *fptr2;

	// Opening logger file for starting information
	fptr = fopen(((struct thread_info *)thread_input)->string, "a");

	if(fptr)
	{
		// Gathering and printing useful information
		printf("\nThread ID of Child 2: %ld", syscall(SYS_gettid));
		printf("\nUse this for passing signal specifically to this thread\n");
		gettimeofday(&current_time, 0);
		fprintf(fptr, "\n--->>> Child 2 Started at *%lu.%06lu* <<<---\n", current_time.tv_sec, current_time.tv_usec);
		fprintf(fptr, "Child 2 Identities\nLinux Thread ID: *%ld* POSIX Thread ID: *%lu*", syscall(SYS_gettid), pthread_self());
		fprintf(fptr, "\nTask: To log CPU usage details at every 100ms\n");

		fclose(fptr);
	}

	// Handling failed attempt
	else	printf("\nError opening log file for child thread 2 starting details\n");

	// Configuring timer and signal action
  	struct sigaction custom_signal_action;
	struct itimerval custom_timer;

	// Set all initial values to 0 in the structure
	memset(&custom_signal_action, 0, sizeof (custom_signal_action));

	// Set signal action handler to point to the address of the target function (to execute on receiving signal)
	custom_signal_action.sa_handler = &cpu_usage;

	// Setting interval to 100ms
	custom_timer.it_interval.tv_sec = 0;
	custom_timer.it_interval.tv_usec = 100000;

	// Setting initial delay to 1 second
	custom_timer.it_value.tv_sec = 1;
	custom_timer.it_value.tv_usec = 0;

	// Setting the signal action to kick in the handler function for these 3 signals
	sigaction (SIGVTALRM, &custom_signal_action, 0);
	sigaction (SIGUSR1, &custom_signal_action, 0);
	sigaction (SIGUSR2, &custom_signal_action, 0);

	// Starting timer
	setitimer (ITIMER_VIRTUAL, &custom_timer, 0);

	// Keep child thread 2 running
	while(1)
	{
		// Wait for signal
		while(flag == 0);

		// If timer interrupt has passed signal, log cpu usage
		if(flag == SIGVTALRM)
		{
			// Setting source of cpu usage data
			char *cpu_stat_source = "cat /proc/stat | head -n 2";

			// Using pipe stream from 'cat' process
			FILE *fptr2 = popen(cpu_stat_source, "r");
			if(fptr2)
			{
				// Open common file for logging
				fptr = fopen(((struct thread_info *)thread_input)->string, "a");
				if(fptr)
				{
					// General message
					gettimeofday(&current_time, 0);
					fprintf(fptr, "\nCPU Stats Captured at *%lu.%06lu*\n", current_time.tv_sec, current_time.tv_usec);

					// Copy each character till the process's text output has reached end of file indicator
				  	while(!feof(fptr2))
					{
						char tchar;
						tchar = fgetc(fptr2);
						fprintf(fptr, "%c", tchar);
					}
				}

				// Handling failed attempt
				else	printf("\nError opening log file for cpu usage data logging\n");

				fclose(fptr);
			}

			// Handling failed attempt
			else	printf("\nError opening process for cpu usage data\n");

			flag = 0;
		}

		// In case of user signals, log and kill the child 2 thread
		else if(flag == SIGUSR1 || flag == SIGUSR2)
		{
			// Notifying user
			printf("\nUser Signal Passed - Killing Child 2\n");

			// Opening logging file for exiting information
			fptr2 = fopen(((struct thread_info *)thread_input)->string, "a");
			if(fptr2)
			{
				gettimeofday(&current_time, 0);
				fprintf(fptr2, "\n--->>> Child 2 Exited at *%lu.%06lu* <<<---\n", current_time.tv_sec, current_time.tv_usec);
				if(flag == SIGUSR1)	fprintf(fptr2, "Exit Reason: User Signal 1 Received (%d)\n", flag);
				else	fprintf(fptr2, "Exit Reason: User Signal 2 Received (%d)\n", flag);
				fclose(fptr2);
				flag = 1;
			}

			// Handling failed attempt
			else	printf("\nError opening log file for child thread 2 exiting details\n");

			// Immediately terminate the thread (unlike pthread_cancel)
			pthread_exit(0);

			// Break the infinite loop
			break;

		}
	}
}

// Main, accepts arguments from command line
int main(int argc, char **argv)
{
	// Structure setting to be passed as argument to the thread
	struct thread_info *tptr = (struct thread_info *)malloc(sizeof(struct thread_info));
	if(tptr)
	{
		// Local file pointer
		static FILE *fptr;

		// Get string and add .txt after that
		tptr->string = argv[1];
		strcat(tptr->string,".txt");

		// Print some useful information
		printf("\nProcess ID: %d Parent PID: %d\n", getpid(), getppid());

		// Log information of parent thread (main function in this case)
		clock_gettime(CLOCK_REALTIME, &timespec_struct);
		fptr = fopen(tptr->string, "w+");

		if(fptr)
		{
			gettimeofday(&current_time, 0);
			fprintf(fptr, "--->>> Master Thread Created & Started at *%lu.%06lu* <<<---\n", current_time.tv_sec, current_time.tv_usec);
			fclose(fptr);
		}

		// Handling failed attempt
		else	printf("\nError opening log file for printing information about starting of main thread\n");

		// Create and launch thread functions along with strcuture pointer as argument
	 	pthread_create(&child_thread1, 0, child1_function, (void *)tptr);
	 	pthread_create(&child_thread2, 0, child2_function, (void *)tptr);

		// Wait for threads to terminate
		pthread_join(child_thread1, 0);
		pthread_join(child_thread2, 0);

		// Opening log file to print information about exit of the main thread
		fptr = fopen(tptr->string, "a");

		if(fptr)
		{
			gettimeofday(&current_time, 0);
			fprintf(fptr, "\n--->>> Master Thread Exited at *%lu.%06lu* <<<---\n", current_time.tv_sec, current_time.tv_usec);
			fclose(fptr);
		}

		// Handling failed attempt
		else	printf("\nError opening log file for printing information about exiting of main thread\n");
	}

	// Handling failed allocation
	else	printf("\nMalloc Failed...Exiting\n");
}

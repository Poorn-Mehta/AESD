#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define ARR_SIZE 50


int main(int argc, char **argv)
{
	/* Print to standard out an interesting string using printf */
	printf("Hello! This is an interesting string!\n");
	
	/* Create a file */
	FILE *MyFileP;
	
	/* Modify the permissions of the file to be read/write and open the file (for writing) */
	MyFileP = fopen("test.txt", "w+");
	
	/* Write a character to the file */
	fputc('a', MyFileP);
	
	/* Close the file */
	fclose(MyFileP);
	
	/*  Open the file (in append mode) */
	MyFileP = fopen("test.txt", "a");
	
	
	/* Dynamically allocate an array of memory */
	char *CharArr;
	CharArr = (char *)malloc(sizeof(char)*ARR_SIZE);			//Allocate memory for array
	//Error checking
	if(CharArr == NULL)
	{
		printf("ERROR: malloc of size %d failed!\n", ARR_SIZE);
		exit(1);
	}
	
	
	/* Read an input string from the command line and write to the string to the allocated array */
	printf("Input a string: ");
	fgets(CharArr, ARR_SIZE, stdin);
	
	
	/* Write the string to the file */
	fprintf(MyFileP, "%s", CharArr);
	
	/* Flush file output */
	fflush(MyFileP);  
	
	
	/* Close the file */
	fclose(MyFileP);
	
	
	/* Open the file (for reading) */
	MyFileP = fopen("test.txt", "r");
	
	
	/* Read a single character (getc) */
	getc(MyFileP);
	
	
	/* Read a string of characters (gets) */
	fgets(CharArr, ARR_SIZE, MyFileP);
	
	
	/* Close the file */
	fclose(MyFileP);
	
	
	/* Free the memory */
	free(CharArr);
	
	return 0;
}

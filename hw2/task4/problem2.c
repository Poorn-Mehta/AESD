#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>

int main()
{
	printf("It is interesting how much effort does it take to just think about an interesting string really\n");

	FILE *fp;
	fp = fopen("problem2.txt", "w+");
	fputc('Z', fp);
	fclose(fp);

	fp = fopen("problem2.txt", "a");
	char *str;
	str = (char*) malloc(50);
	printf("\nType in a string:");
	fgets(str, 50, stdin);
	fprintf(fp,"%s",str);
	fflush(fp);
	fclose(fp);

	fp = fopen("problem2.txt", "r");
	char c, arr[50];
	c = fgetc(fp);
	fgets(arr, 50, fp);
	fclose(fp);
	free(str);
	return 0;
}


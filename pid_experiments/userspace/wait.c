#include <stdio.h>
#include <unistd.h>

int main(void)
{
	long pid = getpid();
	int c;
	printf("My PID is: %ld\n"
	       "I'm going to wait until input is sent to me from stdin.\n",
	       pid);
	c = getchar();
	return 0;
}

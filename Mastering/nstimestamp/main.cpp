#include <stdio.h>
#include <time.h>
#include <stdlib.h>
int main()
{
	time_t curtime = time(NULL);
	tm *now = localtime(&curtime);

	char formatted_time[2048];
	strftime(formatted_time, 2048, "%Y%m%d_%H%M%S",  now);

	puts(formatted_time);
}
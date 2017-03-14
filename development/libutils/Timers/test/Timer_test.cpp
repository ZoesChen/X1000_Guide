#include <stdio.h>
#include <utils/Timers.h>

int main()
{
	printf("timer test\n");
	nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
	printf("now:%d\n",now);
	return 0;
}

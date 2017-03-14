#include<stdio.h>
#include<pthread.h>
#include<c_pthread.h>

void * f(void *buf)
{

	int policy;
	struct sched_param param;
	pthread_getschedparam(pthread_self(), &policy, &param);
	printf("*****mythread : param.sched_priority = %d\n", param.sched_priority);
	switch(policy)
	{
		case SCHED_OTHER:
			printf("*****mythread :policy = SCHED_OTHER\n"); break;
		case SCHED_RR :
			printf("*****mythread :policy = SCHED_RR\n"); break;
		case SCHED_FIFO :
			printf("*****mythread :policy =  SCHED_FIFO\n"); break;

	}
	while(1)
	{
		sleep(1);
		printf("***********mythread run !!!*********\n");
	}
}



int main()
{
	creatthread("mypthread", 10, 1024, f, NULL);

	sleep(5);
	printf("*******main******\n");
	pthreadJoin();

}

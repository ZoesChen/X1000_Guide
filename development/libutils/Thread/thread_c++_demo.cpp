#include<stdio.h>
#include<unistd.h>
#include<mThread.h>

class mythread :public Thread
{
private:
        bool    threadLoop() {
                int policy;
                struct sched_param param;
                pthread_getschedparam(pthread_self(), &policy, &param);
                printf("mythread : param.sched_priority = %d\n", param.sched_priority);
                switch(policy)
                {
                        case SCHED_OTHER:
                                printf("mythread :policy = SCHED_OTHER\n"); break;
                        case SCHED_RR :
                                printf("mythread :policy = SCHED_RR\n"); break;
                        case SCHED_FIFO :
                                printf("mythread :policy =  SCHED_FIFO\n"); break;
                }
                while(1) {
			printf("***********mythread run !!!*********\n");
			sleep(3);
		}
              //  return true;
        }

public:
        mythread() : Thread() {
        }

        ~mythread()
        {
        }

};


int main()
{
        mythread *id = new mythread;
        id->run("mythread", 10, 1024);

        printf("******main********\n");
        while(1);

}




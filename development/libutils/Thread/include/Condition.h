#ifndef __CONDITION_H
#define __CONDITION_H
#include<pthread.h>
#include<Mutex.h>

class Condition {

private:
        pthread_cond_t mCond;

public:
         Condition();
         ~Condition();

         int wait(Mutex & mutex);
         void signal();
         void broadcast();

};

#endif

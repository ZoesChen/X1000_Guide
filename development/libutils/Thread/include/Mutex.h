#ifndef __MUTEX_H
#define __MUTEX_H
#include<pthread.h>

class Mutex {

private:
        friend class Condition;
        pthread_mutex_t mutex;

public:
        class AutoLock {
        private:
                const Mutex *mLock;
        public:
                inline AutoLock(const Mutex * mutex) : mLock(mutex)
                {
                        mLock->lock();
                }

                inline ~AutoLock()
                {
                        mLock->unlock();

                }

        };

        Mutex();
        ~Mutex();
        void lock();
        void unlock();

};

#endif

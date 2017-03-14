#include<Condition.h>

Condition::Condition() {
        pthread_cond_init(&mCond, NULL);

}

Condition::~Condition() {
        pthread_cond_destroy(&mCond);

}

int Condition::wait(Mutex & mutex) {

        pthread_cond_wait(&mCond, &mutex.mutex);
}

void Condition::broadcast() {
        pthread_cond_broadcast(&mCond);

}

void Condition::signal() {

        pthread_cond_signal(&mCond);

}


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/prctl.h>
#include <sys/types.h>
#include<errno.h>
#include <mThread.h>

pthread_t getThreadId();

struct thread_data_t {
	void*   	entryFunction;
	void*           userData;
	char *          threadName;

	// we use this trampoline when we need to set the priority with
	// nice/setpriority, and name with prctl.
	static int trampoline(const thread_data_t* t) {
		int (*f)(void *) = (int (*)(void *))t->entryFunction;
		void* u = t->userData;
		char * name = t->threadName;
		delete t;
		if (name) {
			prctl(PR_SET_NAME, name, NULL, NULL, NULL);
			free(name);
		}
		return f(u);
	}

};

int CreateThread(void * entryFunction,
		void * data,
		u32 threadStackSize,
		const char * threadName,
		int threadPriority,
		pthread_t * threadId)
{
	void * UserData = NULL;
	pthread_attr_t attr;
	pthread_attr_init(&attr);     //init attrbute structict
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //set thread as DETACHED status  //notice!!!

	if(threadName != NULL) {
		thread_data_t * t = new thread_data_t;
		t->threadName = threadName ? strdup(threadName) : NULL;
		t->userData = data;
		t->entryFunction = entryFunction;
		entryFunction = (void *)&thread_data_t::trampoline;
		UserData = t;

	}

	if(threadStackSize) {
		pthread_attr_setstacksize(&attr, threadStackSize);  // every thread only have stack of itself
	}

	/******************************/
	if(threadPriority != 0) {
		struct sched_param param;
		param.sched_priority = threadPriority;

		//three policies : SCHED_OTHER,SCHED_RR,SCHED_FIFO

		if(threadPriority > 0)
			pthread_attr_setschedpolicy(&attr, SCHED_RR);

		pthread_attr_setschedparam(&attr, &param);
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	}
	/******************************/


	pthread_t tid;
	int result = pthread_create(&tid, &attr, (void* (*)(void *))entryFunction, UserData);
	pthread_attr_destroy(&attr);
	if(result !=  0) {
		printf("CreatThread failed\n");
		return -1;

	}

	if(threadId != NULL) {

		*threadId = tid;
	}
	return 0;

}

pthread_t getThreadId()
{

	return pthread_self();

}

Thread::Thread(): mThread((pthread_t)-1), mRunning(false), mExitPending(false)
{
	trackMe(true, false);
}

Thread::~Thread()
{
}

int Thread::run(const char *name, int priority, u32 stack)
{
	Mutex::AutoLock _t((const Mutex *)&mutex);

	if(mRunning) {
		return -1;
	}

	mStatus = 0;
	mExitPending = false;
	mRunning = true;
	mThread = (pthread_t)-1;

	mHoldSelf = new sp<Thread>(this);
	//*mHoldSelf = this;


	int res = CreateThread(((void *)(_threadLoop)), this, stack, name, priority, &mThread);
	if(res < 0) {
		mStatus = -1;
		mRunning = false;
		mThread = (pthread_t)-1;
		delete mHoldSelf;
		return mStatus;
	}

	return 0;
}

static int Thread::_threadLoop(void * user) {

	Thread * const self = static_cast<Thread *>(user);

	sp<Thread> strong(*self->mHoldSelf);
	wp<Thread> weak(strong);
	delete self->mHoldSelf;
	//self->tid = getThreadId();

	bool first = true;
	do {
		bool result;
		if(first) {
			first = false;
			self->mStatus = self->readyToRun();
			result = (self->mStatus == 0);

			if(result && !self->exitPending()) {

				result = self->threadLoop();
			}
		} else {
			result = self->threadLoop();
		}
		{
			Mutex::AutoLock _t((const Mutex *)&(self->mutex));
			if(result == false || self->mExitPending) {
				self->mExitPending = true;
				self->mRunning = false;
				self->mThread = (pthread_t) -1;

				self->condition.broadcast();
				printf("*******children broadcast \n");
				break;
			}
		}
		strong.clear();
		strong = weak.promote();

	} while(strong != 0);

	return 0;

}

void Thread::requestExit()
{
	Mutex::AutoLock _t((const Mutex *)&mutex);
	mExitPending = true;
}

int Thread::requestExitAndWait()
{
	Mutex::AutoLock _t((const Mutex *)&mutex);
	if(mThread == getThreadId()) {
		printf("Thread (mThread=%u, getTreadId = %u):    	\
		don't call waitForExit from this Thread object's 	\
		thread, It's a deadlock\n", (u32)mThread, (u32)getThreadId());
		return -1;
	}

	mExitPending = true;

	while(mRunning == true) {
		condition.wait(mutex);
	}

	mExitPending = false;
	return 0;
}

int Thread::join()
{
	Mutex::AutoLock _t((const Mutex *)&mutex);
	if(mThread == getThreadId()) {
		printf("Thread (mThread=%u, getTreadId = %u):    	\
		don't call waitForExit from this Thread object's 	\
		thread, It's a deadlock\n", (u32)mThread, (u32)getThreadId());
		return -1;
	}

	mExitPending = true;
	while(mRunning == true) {
		condition.wait(mutex);
	}

	return 0;

}

bool Thread::isRunning() const {

	Mutex::AutoLock _t((const Mutex *)&mutex);
	return mRunning;
}

bool Thread::exitPending() const {

	Mutex::AutoLock _t((const Mutex *)&mutex);
	return mExitPending;

}

int Thread::readyToRun()
{
	return 0;
}


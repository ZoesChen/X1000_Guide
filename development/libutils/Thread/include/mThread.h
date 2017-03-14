#ifndef __THREAD_H
#define __THREAD_H

#include<pthread.h>
#include<Mutex.h>
#include<Condition.h>
#include<StrongPointer.h>
#include<RefBase.h>

typedef unsigned int u32;

class Thread : public RefBase {

private:
	volatile bool	mRunning;
	volatile bool   mExitPending;
	int 		mStatus;
	pthread_t 	mThread;
	Mutex		mutex;
	Condition	condition;
	sp<Thread>		*mHoldSelf;
//	pid_t 		tid;
	static int 	_threadLoop(void * user);

protected:
	virtual bool	threadLoop() = 0;

public:
	Thread();
	~Thread();
	virtual int run(const char *name, int priority, u32 stack);
	virtual void requestExit();
	virtual int readyToRun();
	int 	requestExitAndWait();
	int 	join();
	bool	isRunning() const;
	bool 	exitPending() const;
};

#endif

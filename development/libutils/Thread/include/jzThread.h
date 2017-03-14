#ifndef __JZTHREAD_H
#define __JZTHREAD_H

#include<stdio.h>
#include<unistd.h>
#include<mThread.h>

class jzThread :public Thread
{
private:
	bool    threadLoop() {
		f(userdat);
		return true;

	}
	void *(*f)(void *);
	void *userdat;

public:
	jzThread(void * (*entryFunc)(void  *), void *userdat) : Thread(), f(entryFunc), userdat(userdat) {
	//	trackMe(true, false);
	//	run(name, priority, stack);
	}

	void join() {
		static_cast<Thread *>(this)->join();
	}

	~jzThread()
	{
	}

};

#endif



#include<jzThread.h>

#ifdef __cpluscplus
extern "C" {
#endif

static jzThread * pthread = NULL;

extern "C" int creatthread(char *name, int priority, unsigned int stack, void *entryFunc, void *userdat)
{
	pthread = new jzThread(entryFunc, userdat);
	pthread->run(name, priority, stack);
}

extern "C" void pthreadJoin()
{
	if(pthread)
		pthread->join();
	else
		printf("please run creatthread() !\n");
	return;
}

#ifdef __cpluscplus
}
#endif

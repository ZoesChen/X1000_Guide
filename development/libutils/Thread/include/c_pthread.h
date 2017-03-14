#ifndef __TEST_PTHREAD_H
#define __TEST_PTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif

/*this func is to create thread
name:thread name.
priority:thread priority.
stack:thread stack size.
entryfunc:user function.
userdat:user data
*/
int creatthread(char *name, int priority, unsigned int stack, void *entryFunc, void *userdat);


/*Wait until this thread exits. Returns immediately if entryFunc func is not yet running.*/
void pthreadJoin();

#ifdef __cpluscplus
}
#endif

#endif

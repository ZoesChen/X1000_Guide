#ifndef __CIM_SIGNAL_H__
#define __CIM_SIGNAL_H__

typedef	void	Sigfunc(int);
extern Sigfunc * cim_signal(int signo, Sigfunc *func);

#endif /* __CIM_SIGNAL_H__ */

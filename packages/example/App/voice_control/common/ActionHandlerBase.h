#ifndef __ACTION_HANDLER_BASE_H__
#define __ACTION_HANDLER_BASE_H__

#include <cJSON.h>
#include <debug.h>

class ActionBase;
class ActionHandlerBase
{
public:
	ActionHandlerBase(cJSON * actJson, int sock);
	~ActionHandlerBase();

	virtual int doAction();

	// default ack ActionOK
	virtual int doAck();
	// default ack ActionOK
	virtual const char * getAck();

	int setSocket(int sock);
	int mSocket;
	cJSON * getJsonRoot() { return mJsonRoot;}

private:
	cJSON * mJsonRoot;
	ActionBase *mAck; 
};

#endif	/* __ACTION_HANDLER_BASE_H__ */

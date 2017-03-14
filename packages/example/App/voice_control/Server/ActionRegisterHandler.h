#ifndef __ACTION_REGISTER_HANDLE_H
#define __ACTION_REGISTER_HANDLE_H

struct cJSON;
class CClient;
class ActionRegisterAck;
class ActionRegisterParser;

class ActionRegisterHandler: public ActionHandlerBase
{
public:
	ActionRegisterHandler(CClient * c, cJSON * json);
	~ActionRegisterHandler();

	virtual int doAction();

	virtual int doAck();
	//cJSON * getAck();

private:
	CClient * mClient;
	ActionRegisterParser * mRegisterParser;
	ActionRegisterAck * mAck;
	//cJSON * mJsonRoot;
};

#endif	/* __ACTION_REGISTER_HANDLE_H */

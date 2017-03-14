#ifndef __ACTION_REGISTER_ACK_H
#define __ACTION_REGISTER_ACK_H

#include <ActionBase.h>


class ClientConfig;

class ActionRegisterAck: public ActionBase
{
public:
	ActionRegisterAck(int sock, ClientConfig *config);
	~ActionRegisterAck();

	//int setClientID(const char *);

private:
	ClientConfig *mConfig;
};


struct ClientHeader;

class ActionRegisterAckParser: public ActionBaseParser
{
public:
	ActionRegisterAckParser(cJSON *jsonRoot);
	~ActionRegisterAckParser();

private:
	ClientHeader* mClientHeader;
};

#endif	/* __ACTION_REGISTER_ACK_H */

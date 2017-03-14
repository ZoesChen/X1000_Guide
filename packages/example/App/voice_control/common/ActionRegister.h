#ifndef __ACTION_REGISTER_H
#define __ACTION_REGISTER_H

#include <ActionBase.h>


class ClientConfig;

class ActionRegister: public ActionBase
{
public:
	ActionRegister(int sock, ClientConfig *config);
	~ActionRegister();

private:
	ClientConfig *mConfig;
};


struct ClientHeader;
class ActionRegisterParser: public ActionBaseParser
{
public:
	ActionRegisterParser(cJSON *jsonRoot);
	~ActionRegisterParser();

	//const char * getClientName();

private:
	//ClientHeader* mClientHeader;
	//char * mClientName;
};

#endif	/* __ACTION_REGISTER_H */

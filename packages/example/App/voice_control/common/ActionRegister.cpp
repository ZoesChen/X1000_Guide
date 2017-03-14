#include <ActionBase.h>
#include <ActionRegister.h>
#include <DeviceInterface.h>
#include <ClientConfig.h>



// --------------------------- Action ---------------------------------
ActionRegister::ActionRegister(int sock, ClientConfig *config)
	: ActionBase(sock), mConfig(config)
{
	DEBUG_LINE();
	if (!mConfig)
		return;
	DEBUG_LINE();
	setActionType(ACTION_TYPE_REGISTER);
	printf("mConfig->getClientName(): %s\n", mConfig->getClientName());
	DEBUG_LINE();
	if (mConfig->getClientName())
		addClientHeaderItem(CLIENT_NAME_STRING, mConfig->getClientName());
	DEBUG_LINE();
	addClientHeaderItem(CLIENT_ID_STRING, "");
	DEBUG_LINE();
}

ActionRegister::~ActionRegister()
{

}

// --------------------------- Action Parser ---------------------------------
//#include <ClientHeader.h>

ActionRegisterParser::ActionRegisterParser(cJSON *jsonRoot)
	: ActionBaseParser(jsonRoot)
{

}

ActionRegisterParser::~ActionRegisterParser()
{

}


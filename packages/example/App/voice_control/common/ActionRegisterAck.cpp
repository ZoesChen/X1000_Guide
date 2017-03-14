#include <ActionBase.h>
#include <ActionRegisterAck.h>
#include <DeviceInterface.h>
#include <ClientConfig.h>



// --------------------------- Action ---------------------------------
ActionRegisterAck::ActionRegisterAck(int sock, ClientConfig *config)
	: ActionBase(sock), mConfig(config)
{
	if (!mConfig)
		return;
	setActionType(ACTION_TYPE_REGISTER_ACK);
	addClientHeaderItem(CLIENT_NAME_STRING, mConfig->getClientName());
	// generate client ID
	//addClientHeaderItem(CLIENT_ID_STRING, id);
}

ActionRegisterAck::~ActionRegisterAck()
{

}

/*
int ActionRegisterAck::setClientID(const char * id)
{
	addClientHeaderItem(CLIENT_ID_STRING, id);
}
*/

// --------------------------- Action Parser ---------------------------------
#include <ClientHeader.h>

ActionRegisterAckParser::ActionRegisterAckParser(cJSON *jsonRoot)
	: ActionBaseParser(jsonRoot)
{
	// if (!mClientHeader)
	// 	return;
	// mClientHeader->Name = getClientHeaderItemValue(CLIENT_NAME_STRING);
	// mClientHeader->ID = getClientHeaderItemValue(CLIENT_ID_STRING);
}

ActionRegisterAckParser::~ActionRegisterAckParser()
{

}

// ClientHeader* ActionRegisterAckParser::getClientHeader()
// {
// 	return mClientHeader;
// }

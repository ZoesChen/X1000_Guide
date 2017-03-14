#include <ActionOK.h>

#include <DeviceInterface.h>

ActionOK::ActionOK(int sock): ActionBase(sock)
{
	setActionType(ACTION_TYPE_ACK_OK);
	addOperateItem("ack", "ok");
}


ActionOK::~ActionOK()
{

}


// --------------------------- Action Parser ---------------------------------

ActionOKParser::ActionOKParser(cJSON *jsonRoot)
	: ActionBaseParser(jsonRoot)
{

}

ActionOKParser::~ActionOKParser()
{

}

#include <string.h>
#include <DeviceInterface.h>
#include <ClientConfig.h>
#include <ActionBase.h>
#include <ActionSendSpeechCommand.h>



// --------------------------- Action ---------------------------------
ActionSendSpeechCommand::ActionSendSpeechCommand(int sock, ClientConfig *config)
	: ActionBase(sock), mConfig(config)
{
	DEBUG_LINE();
	if (!mConfig)
		return;
	DEBUG_LINE();
	setActionType(ACTION_TYPE_SPEECH_COMMAND);
	printf("mConfig->getClientName(): %s\n", mConfig->getClientName());
	DEBUG_LINE();
	if (mConfig->getClientName())
		addClientHeaderItem(CLIENT_NAME_STRING, mConfig->getClientName());
	DEBUG_LINE();
	addClientHeaderItem(CLIENT_ID_STRING, "");
	DEBUG_LINE();
}

ActionSendSpeechCommand::~ActionSendSpeechCommand()
{

}


int ActionSendSpeechCommand::setSpeechCommand(const char * cmd)
{
	addOperateItem(SPEECH_OPERATE_STRING, cmd);
	return 0;
}

// --------------------------- Action Parser ---------------------------------
//#include <ClientHeader.h>

ActionSendSpeechCommandParser::ActionSendSpeechCommandParser(cJSON *jsonRoot)
	: ActionBaseParser(jsonRoot)
{
	const char * actionType;
	actionType = getActionType();

	if ( !strcmp(actionType, ACTION_TYPE_SPEECH_COMMAND)) {
		parseState = 0;
	}
	else {
		parseState = -1;
	}
}

ActionSendSpeechCommandParser::~ActionSendSpeechCommandParser()
{

}

int ActionSendSpeechCommandParser::getSpeechCommand(const char ** pcmd)
{
	const char * cmd;

	if (parseState) {
		printf("ActionSendSpeechCommandParser state wrong....\n");
		return -1;
	}
	cmd = getOperateValue(SPEECH_OPERATE_STRING);
	printf("parse SPEECH_COMMAND: %s\n", cmd);
	*pcmd = cmd;

	return 0;
}

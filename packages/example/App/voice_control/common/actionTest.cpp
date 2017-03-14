#include <stdio.h>
#include <string.h>

#include <cJSON.h>
#include <DeviceInterface.h>
#include <ActionBase.h>
#include <ActionRegister.h>
#include <ActionRegisterAck.h>
#include <ActionOK.h>
#include <ClientConfig.h>

int parseMsg(const char * msg)
{
	cJSON *cJsonRoot = messageStringToCJson(msg);

	const char * action = getActionType(cJsonRoot);

	printf("--------------------------------------------------------------------------------\n");
	printf("Parse action: %s\n", action);
	printf("--------------------------------------------------------------------------------\n");

	ActionBaseParser * parser;
	parser = NULL;

	if (!strncmp(action, ACTION_TYPE_REGISTER, strlen(ACTION_TYPE_REGISTER)) ) {
		parser = new ActionRegisterParser(cJsonRoot);
	}
	else if (!strncmp(action, ACTION_TYPE_REGISTER_ACK, strlen(ACTION_TYPE_REGISTER_ACK)) ) {
		parser = new ActionRegisterAckParser(cJsonRoot);
	}
	else if (!strncmp(action, ACTION_TYPE_ACK_OK, strlen(ACTION_TYPE_ACK_OK)) ) {
		parser = new ActionOKParser(cJsonRoot);
	}
	else if (!strncmp(action, ACTION_TYPE_UPLOAD_ACTION, strlen(ACTION_TYPE_UPLOAD_ACTION)) ) {
		//parser = new Action Parser(cJsonRoot);
	}
	else if (!strncmp(action, ACTION_TYPE_SPEECH_COMMAND, strlen(ACTION_TYPE_SPEECH_COMMAND)) ) {
		//parser = new Action Parser(cJsonRoot);
	}
	else {
		printf("Unknown action: %s\n", action);
	}

	if (parser) {
		printf("ActionParser::getActionType()= %s\n", parser->getActionType());
		printf("ActionParser::getClientName()= %s\n", parser->getClientName());
		printf("ActionParser::getClientID()= %s\n", parser->getClientID());
		printf("ActionParser::getOperateNum()= %d\n", parser->getOperateNum());
	}

	return 0;
}


int main(int argc, char * argv[])
{
	char * msg;
	ClientConfig * config;
	DEBUG_LINE();
	config = new ClientConfig();

	DEBUG_LINE();
	config->setClientName("air_conditioner");
	config->setClientID("air_conditioner_001");
	DEBUG_LINE();

	ActionBase * act;

	act = new ActionRegister(-1, config);
	printf("new Action: %p\n", act);
	msg = act->makeString();
	printf("--------------------------------------------------------------------------------\n");
	printf("%s\n", msg);
	printf("--------------------------------------------------------------------------------\n");

	parseMsg(msg);


	act = new ActionRegisterAck(-1, config);
	printf("new Action: %p\n", act);
	msg = act->makeString();
	printf("--------------------------------------------------------------------------------\n");
	printf("%s\n", msg);
	printf("--------------------------------------------------------------------------------\n");
	parseMsg(msg);

	act = new ActionOK(-1);
	printf("new Action: %p\n", act);
	msg = act->makeString();
	printf("--------------------------------------------------------------------------------\n");
	printf("%s\n", msg);
	printf("--------------------------------------------------------------------------------\n");
	parseMsg(msg);


	return 0;
}

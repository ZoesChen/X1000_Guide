#include <string.h>
#include <cJSON.h>
#include <ActionBase.h>
#include <ActionRegister.h>
#include <ActionRegisterAck.h>
#include <DeviceInterface.h>
#include <ClientConfig.h>
#include <ActionHandlerBase.h>
#include <ActionRegisterHandler.h>
#include "CClient.h"



ActionRegisterHandler::ActionRegisterHandler(CClient * c, cJSON * json)
	:ActionHandlerBase(json, -1), mClient(c)
{

	setSocket(mClient->getSocket());
}


ActionRegisterHandler::~ActionRegisterHandler()
{
	DEBUG_LINE();
	if (mRegisterParser) {
		delete mRegisterParser;
		mRegisterParser = NULL;
	}
	if (mAck) {
		delete mAck;
		mAck = NULL;
	}
}

int ActionRegisterHandler::doAction()
{
	ClientConfig *config;
	const char * clientID;
	mRegisterParser = new ActionRegisterParser(getJsonRoot());
	config = mClient->getClientConfig();
	if (!config)
		return -1;
	//clientHeader = mRegisterParser->getClientHeader();
	const char * clientName = mRegisterParser->getClientName();
	printf("clientName= %p\n", clientName);
	config->setClientName((clientName));

	// generate client ID
	clientID = "generate client ID 001";
	config->setClientID((clientID));

	//if (!strcmp(clientName, CLIENT_NAME_LIGHT)) {
	if (mClient) {
		mClient->registerToServer(clientName);
	}

	return 0;
}

int ActionRegisterHandler::doAck()
{
	const char * clientID;
	ClientConfig *config;

		DEBUG_LINE();
	config = mClient->getClientConfig();
		DEBUG_LINE();
	if (!config)
		return -1;
		DEBUG_LINE();
	clientID = config->getClientID();

		DEBUG_LINE();
		// socket
	mAck = new ActionRegisterAck(mSocket, config);
	// generate client ID
		DEBUG_LINE();
	mAck->setClientID(clientID);

		DEBUG_LINE();
 	mAck->commitToServer();
		DEBUG_LINE();
 
	return 0;
}

// cJSON * ActionRegisterHandler::getAck()
// {
	// socket
	//mAck = new ActionRegisterAck();

	// generate client ID

	//return mAck->makeJsonString();
// 	return NULL;
// }

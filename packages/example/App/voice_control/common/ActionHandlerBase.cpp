#include <ActionBase.h>
#include <ActionRegister.h>
#include <DeviceInterface.h>
#include <ClientConfig.h>
#include <ActionOK.h>
#include <ActionHandlerBase.h>
//#include <CClient.h>


ActionHandlerBase::ActionHandlerBase(cJSON * json, int sock)
	:mJsonRoot(json), mSocket(sock), mAck(NULL)
{


}


ActionHandlerBase::~ActionHandlerBase()
{
	//delete mRegisterParser;
	if (mAck)
		delete mAck;
}

int ActionHandlerBase::setSocket(int sock)
{

	mSocket = sock;
	return 0;
}
int ActionHandlerBase::doAction()
{
	DEBUG_LINE();

	return 0;
}

// default ack ActionOK
int ActionHandlerBase::doAck()
{
	char * msg;
		DEBUG_LINE();
	mAck = new ActionOK(mSocket);

		DEBUG_LINE();
	mAck->commitToServer();
		DEBUG_LINE();

	return 0;
}

// default ack ActionOK
const char * ActionHandlerBase::getAck()
{
		DEBUG_LINE();

	mAck = new ActionOK(mSocket);

	return mAck->makeJsonString();
}

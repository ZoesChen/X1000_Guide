#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <ActionBase.h>
#include <DeviceInterface.h>
#include <ClientConfig.h>



// --------------------------- Action ---------------------------------
ActionBase::ActionBase(int sock)
	: mSocket(sock)
{

	DEBUG_LINE();
	mJsonRoot = cJSON_CreateObject();
	DEBUG_LINE();
	if(NULL == mJsonRoot)
	{
		IOT_WARNIG("cJSON_CreateObject() failed, no memory.\n");
		return ;
	}
	DEBUG_LINE();
	mJsonClientHeader = cJSON_CreateObject();
	if(NULL == mJsonClientHeader)
	{
		IOT_WARNIG("cJSON_CreateObject() failed, no memory.\n");
		return ;
	}
	mJsonOperate = cJSON_CreateObject();
	if(NULL == mJsonOperate)
	{
		IOT_WARNIG("cJSON_CreateObject() failed, no memory.\n");
		return ;
	}


}

ActionBase::~ActionBase()
{
	printf("mJsonString:\n%s\n", mJsonString);
	DEBUG_LINE();
#if 0
	if (mJsonOperate)
		cJSON_Delete(mJsonOperate);
	DEBUG_LINE();
	if (mJsonClientHeader)
		cJSON_Delete(mJsonClientHeader);
#endif
	DEBUG_LINE();
	if (mJsonRoot)
		cJSON_Delete(mJsonRoot);
	DEBUG_LINE();

	// free mJsonString


}

int ActionBase::addRootItem(struct OperateStruct * ops)
{
	if ( !ops || ! mJsonRoot)
		return -1;

	cJSON_AddStringToObject(mJsonRoot, ops->name, ops->value);

	return 0;
}

int ActionBase::setActionType(const char * action)
{
	cJSON_AddStringToObject(mJsonRoot, ACTION_TYPE_STRING, action);
	return 0;
}

int ActionBase::addClientHeaderItem(struct OperateStruct * ops)
{

	addClientHeaderItem(ops->name, ops->value);

	return 0;
}

int ActionBase::addClientHeaderItem(const char * name, const char * value)
{
	if (!mJsonClientHeader)
		return -1;
	cJSON_AddStringToObject(mJsonClientHeader, name, value);

	return 0;
}

int ActionBase::setClientName(const char * name)
{
	addClientHeaderItem(CLIENT_NAME_STRING, name);
	return 0;
}

int ActionBase::setClientID(const char * id)
{
	addClientHeaderItem(CLIENT_ID_STRING, id);
	return 0;
}

int ActionBase::getOperateItemNum()
{
	if (!mJsonOperate)
		return 0;

	return cJSON_GetArraySize(mJsonOperate);
}

int ActionBase::setOperateName(const char * name)
{
	mOperatename = name;
	return 0;
}

int ActionBase::addOperateItem(struct OperateStruct * ops)
{
	if (!ops)
		return -1;
	addOperateItem(ops->name, ops->value);
	return 0;
}

int ActionBase::addOperateItem(const char * name, const char * value)
{
	if (!mJsonOperate)
		return -1;

	cJSON_AddStringToObject(mJsonOperate, name, value);

	return 0;
}

int ActionBase::commitToServer()
{

	// socket send
	const char * msg; 
	msg = makeJsonString();
	printf("mSocket=%d\n", mSocket);
	if (mSocket>0) {
	DEBUG_LINE();

		send(mSocket, msg, strlen(msg), 0);
	DEBUG_LINE();
	}
	else {
		printf("failed mSocket= %d\n", mSocket);
	}

	return 0;
}

const char * ActionBase::makeJsonString()
{

	DEBUG_LINE();
	cJSON_AddItemToObject(mJsonRoot, CLIENT_HEADER_STRING, mJsonClientHeader);
	DEBUG_LINE();
	cJSON_AddItemToObject(mJsonRoot, ACTION_OPERATE_STRING, mJsonOperate);
	DEBUG_LINE();

	mJsonString = cJSON_Print(mJsonRoot);
	DEBUG_LINE();

	return mJsonString;
}


// --------------------------- Action Parser ---------------------------------

cJSON * messageStringToCJson(const char * msg)
{
	cJSON *pParseJson = cJSON_Parse(msg);

	if (1) {
		const char *p = cJSON_Print(pParseJson);
		printf("--------------------------------------------------------------------------------\n");
		printf("%s\n", p);
		printf("--------------------------------------------------------------------------------\n");
	}

	return pParseJson;
}

const char * getActionType(cJSON * jsonMsg)
{
	const char *act;
	cJSON *pSub;
	pSub = cJSON_GetObjectItem(jsonMsg, ACTION_TYPE_STRING);
	if(NULL == pSub)
	{
		cJSON_Delete(pSub);
		return NULL;
	}

	act = pSub->valuestring;
	//printf("%s: %s\n", ACTION_TYPE_STRING, pSub->valuestring);

	return act;
}

#include <ClientHeader.h>

ActionBaseParser::ActionBaseParser(cJSON *jsonRoot)
	: parseState(0),mJsonRoot(jsonRoot)
{
	cJSON *pSub;

	mJsonStr = NULL;
	mJsonClientHeader = NULL;
	mJsonOperate = NULL;
	mActionType = NULL;
	mClientName = NULL;
	mClientID = NULL;
	m_ops_num = 0;


	// parse Action type
	pSub = cJSON_GetObjectItem(mJsonRoot, ACTION_TYPE_STRING);
	if(NULL != pSub)
	{
		mActionType = pSub->valuestring;
	}
	else {
		//cJSON_Delete(pSub);
		//return NULL;
	}

	// parse client header
	pSub = cJSON_GetObjectItem(mJsonRoot, CLIENT_HEADER_STRING);
	if(pSub) {
		mJsonClientHeader = pSub;
		pSub = cJSON_GetObjectItem(mJsonClientHeader, CLIENT_NAME_STRING);
		if(pSub) {
			mClientName = pSub->valuestring;
		}
		pSub = cJSON_GetObjectItem(mJsonClientHeader, CLIENT_ID_STRING);
		if(pSub) {
			mClientID = pSub->valuestring;
		}
	}

	// parse action operates
	pSub = cJSON_GetObjectItem(mJsonRoot, ACTION_OPERATE_STRING);
	if(pSub) {
		mJsonOperate = pSub;
		m_ops_num = cJSON_GetArraySize(mJsonOperate);
	}
}

ActionBaseParser::~ActionBaseParser()
{
	// free...
}

const char *  ActionBaseParser::getClientName()
{
	return mClientName;
}

const char * ActionBaseParser::getClientID()
{
	return mClientID;
}


int ActionBaseParser::getOperateNum()
{

	//m_ops_num = cJSON_GetArraySize(mJsonOperate);

	return m_ops_num;
}


const char * ActionBaseParser::getOperateValue(const char * name)
{
	cJSON *pSub;

	pSub = cJSON_GetObjectItem(mJsonOperate, name);
	if(pSub) {
		return pSub->valuestring;
	}

	return NULL;
}

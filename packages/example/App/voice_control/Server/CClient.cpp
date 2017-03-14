
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>

#include <DeviceInterface.h>
#include <ActionBase.h>
#include <ActionRegister.h>
#include <ActionRegisterAck.h>
#include <ActionOK.h>
#include <ClientConfig.h>
#include <ActionHandlerBase.h>
#include <ActionRegisterHandler.h>
#include "CClient.h"




#define MAX_NUM_BUF 1024
#define DEBUG "CClient:"
#define RECV "RECV :"
#define SEND "SEND :"

using namespace std;

class CClient;

struct ARG
{
	CClient * pThis;
	string var;
};

CClient::CClient(const int sClient, const sockaddr_in &addrClient)
	:mtx(PTHREAD_MUTEX_INITIALIZER), cond(PTHREAD_COND_INITIALIZER)
{
		DEBUG_LINE();
	m_socket = sClient; //client_sockfd
	m_addr = addrClient;  //client_addr
	m_bConning = 0; //0: disconnected;
	m_bExit = 0; //0: client not exit;
	memset(m_data.buf, 0, MAX_NUM_BUF);

		DEBUG_LINE();
	//mtx = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init(&mtx, NULL);
	pthread_cond_init(&cond, NULL);
	g_message_received = 0;

		DEBUG_LINE();
	mClientConfig = new ClientConfig();
		DEBUG_LINE();

}

CClient::~CClient()
{
	close(m_socket);

	delete mClientConfig;
}

bool CClient::StartRunning(void)
{
	m_bConning = 1;
	pthread_t tid_socket_recv, tid_socket_send;
	ARG *arg =new ARG();
	arg->pThis =this;
	// main thread loop
	pthread_create(&tid_socket_send,NULL,mainThreadLoop,(void*)arg);
	// receive data thread
	pthread_create(&tid_socket_recv,NULL,RecvDataThread,(void*)arg);
	return 1;
}


void * CClient::RecvDataThread(void* pParam)
{
	cout<< RECV << "I am in RecvDataThread" <<endl;
	ARG *arg = (ARG*)pParam;
	CClient *pClient = arg->pThis;

	char * recvBuf =new char [MAX_NUM_BUF];

	for (;pClient->m_bConning;)
	{
		DEBUG_LINE();
		// clear buffer
		memset (recvBuf,0,MAX_NUM_BUF);

		DEBUG_LINE();
		// recv() maybe blocking
		int	reVal = recv(pClient->m_socket, recvBuf, MAX_NUM_BUF, 0);
		DEBUG_LINE();
		if ( reVal == 0)
		{
			cout << "RecvDataThread: client exit" <<endl;
			pClient->m_socket = 0;
			pClient->DisConning();
			if (pClient->mCallbackFunc) {
				// client name
				const char * clientName;
				ClientConfig *config;
				config = pClient->getClientConfig();
				clientName = config->getClientName();
				printf("getClientName(): %s\n", clientName);
				if (clientName)
					pClient->mCallbackFunc(pClient, eServerCmdClientExit, (void*)clientName);
			}
			return NULL;
			break;
		}
		else if (reVal > 0)
		{
			cout<<DEBUG << recvBuf<<endl;
			//pClient->HandleData(recvBuf);
			DEBUG_LINE();

			pthread_mutex_lock(&pClient->mtx);
			pClient->g_message_received = 1;
			DEBUG_LINE();
			memcpy(pClient->m_data.buf , recvBuf, MAX_NUM_BUF);
			pthread_cond_signal(&pClient->cond);
			DEBUG_LINE();
			pthread_mutex_unlock(&pClient->mtx);
			DEBUG_LINE();

		}
		else
		{
			cout <<DEBUG<< "recv data error!!!" <<endl;
		}
	}

	return	0;

}

// main thread loop
void * CClient::mainThreadLoop(void*pParam)
{
	cout <<DEBUG<< "I am in mainThreadLoop" <<endl;
	ARG *arg = (ARG*)pParam;
	CClient *pClient = arg->pThis;

		DEBUG_LINE();
	for(;pClient->m_bConning;)
	{
		DEBUG_LINE();
		if(!pClient->m_bConning)
		{
			pClient->m_bExit =	1;
			cout << "mainThreadLoop: client exit" <<endl;
			break;
		}
		DEBUG_LINE();
		cout << "send waiting...."<<endl;
		pthread_mutex_lock(&pClient->mtx);
		DEBUG_LINE();

		// waiting commands
		if (pClient->g_message_received == 0) {
			DEBUG_LINE();
			pthread_cond_wait(&pClient->cond,&pClient->mtx);
		}

		cout << "send be wake up ..."<<endl;

		DEBUG_LINE();
		if (1){
			// wakeup by RecvDataThread
			//int	val =send(pClient->m_socket,pClient->m_data.buf,MAX_NUM_BUF,0);
			//printf("pClient->m_data.buf:\n%s\n", pClient->m_data.buf);
			pClient->HandleMessage(pClient->m_data.buf);
		}
		else {
			// wakeup by Server mainThreadLoop

			// getMessage() from server

		}

		//memset(pClient->m_data.buf,0,MAX_NUM_BUF);
		pClient->g_message_received = 0;
		pthread_mutex_unlock(&pClient->mtx);
		cout <<DEBUG<<"send data over..."<<endl;
	}

	cout <<DEBUG<<"CClient mainThreadLoop over..."<<endl;

	return	0;
}

void CClient::HandleData(const char* pExpr)
{


	return ;
}


void CClient::HandleMessage(const char* msg)
{
	cout <<DEBUG<< "I am in HandleMessage"<<endl;

	// parse client message(action)
	ActionHandlerBase * actHandler;
	const char * action;
	cJSON * actionJson;

	DEBUG_LINE();
	actionJson = messageStringToCJson(msg);
	action = getActionType(actionJson);

	actHandler = NULL;
	if (!strcmp(action, ACTION_TYPE_REGISTER)) {
		DEBUG_LINE();
		actHandler = new ActionRegisterHandler(this, actionJson);
	}
	else if (!strcmp(action, ACTION_TYPE_UPLOAD_ACTION)) {
		//actHandler = new ActionUploadHandler(this, actionJson);
	}
	else if (!strcmp(action, ACTION_TYPE_SPEECH_COMMAND)) {
		//actHandler = new ActionUploadHandler(this, actionJson);
/*
		printf("--------------------------------------------------------------------------------\n");
		printf("%s() ACTION_TYPE_SPEECH_COMMAND: \n%s", __FUNCTION__, msg);
		printf("\n--------------------------------------------------------------------------------\n");
*/

		// server callback...
		if (mCallbackFunc)
			mCallbackFunc(this, eServerCmdSpeechCommand, (void*)msg);

	}
	else {
		printf("Warning unknown action: %s\n", action);
	}


 	if ( actHandler ) {
		DEBUG_LINE();
		actHandler->doAction();

		DEBUG_LINE();
		// send ack cjson
		actHandler->doAck();
		DEBUG_LINE();
		delete actHandler;
	}

	DEBUG_LINE();
	cJSON_Delete(actionJson);
	DEBUG_LINE();
}

int CClient::setCallback(clientCallback_t c)
{
	mCallbackFunc = c;
	return 0;
}

int CClient::setMessage(const char *msg)
{

	mServerMsg = msg;
	return 0;
}

int CClient::getMessage(const char **msg)
{
	*msg = mServerMsg;
	return 0;
}

int CClient::registerToServer(const char * clientType)
{

	if (mCallbackFunc)
		mCallbackFunc(this, eServerCmdRegister, (void*)clientType);
	return 0;
}

int CClient::setSpeechCommand(const char * speech)
{
	// send message in mainThreadLoop?
	// mutex lock?
	memset(m_data.buf, 0, 1024);
	memcpy(m_data.buf, speech, 1024);

	DEBUG_LINE();
	if (m_socket) {				// check socket
		int	val = send(m_socket, m_data.buf, MAX_NUM_BUF,0);
		printf("transmit SPEECH_COMMAND to remote client, send() ret=%d\n", val);
	}

	DEBUG_LINE();
	return 0;
}

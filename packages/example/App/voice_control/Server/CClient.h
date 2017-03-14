#ifndef CClIENT_H
#define CClIENT_H

#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <ClientHeader.h>


#define TIMEFOR_THREAD_CLIENT 500
#define MAX_NUM_CLIENT 10
#define MAX_NUM_BUF 1024

#define HEADERLEN (sinzeof(hdr))


enum eServerCommand {
	eServerCmdDefault=0,
	eServerCmdClientExit,
	eServerCmdRegister,
	eServerCmdSpeechCommand,
};




typedef struct _head
{
	char type;
	unsigned short len;
}hdr,*phdr;

typedef struct _data
{
	char buf[MAX_NUM_BUF];
}DATABUF,*pDataBuf;

typedef int (*clientCallback_t)(void * client, int cmd, void * data);

class ClientConfig;

class CClient
{
public:
	CClient(const int sclient,const sockaddr_in &addrClient);
	virtual ~CClient();
	bool StartRunning(void);
	void HandleData(const char *p);
	void HandleMessage(const char *p);
	bool IsConning(void){return m_bConning;}
	bool DisConning(void){return m_bConning =0;}
	bool IsExit(void){return m_bExit;}

	int registerToServer(const char * clientType);
	int setSpeechCommand(const char * speech);

	int setCallback(clientCallback_t c);
	int getMessage(const char ** msg);
	int setMessage(const char * msg);

	/* getClientConfig */


	static void * RecvDataThread(void *pParam);
	static void * mainThreadLoop(void *pParam);
	ClientConfig *getClientConfig() { return mClientConfig;}
	int getSocket() {return m_socket;}

private:
	CClient();

private:
	int m_socket;
	sockaddr_in m_addr;
	DATABUF m_data;
//		HANDLE m_hEvent;
//		HANDLE m_hThreadSend;
//		HANDLE m_hThreadRecv;
//		CRITICAL_SECTION m_cs;
	bool m_bConning;
	bool m_bExit;
	pthread_mutex_t mtx;
	pthread_cond_t cond;
	int g_message_received;

	//ClientHeader mClientHeader;
	ClientConfig *mClientConfig;

	clientCallback_t mCallbackFunc;
	const char * mServerMsg;
};

#endif //CClIENT_H

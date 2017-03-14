#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <sys/shm.h>
#include <pthread.h>
#include <list>

#include <debug.h>

#include "CClient.h"
#include <DeviceInterface.h>

using namespace std;

#define SERVER_SOCKET_PORT    8887
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define SERVER "SERVER :"
#define ACCEPT "ACCEPT :"


typedef list<CClient*> CLIENTLIST;
CLIENTLIST clientlist;

static pthread_mutex_t gMainThreadMtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gHelperThreadMtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pthread_cond = PTHREAD_COND_INITIALIZER;
int server_sockfd;

CClient *mClientLight;
int client_request_type;
char speech_command[1024];

//void InitDB();

int groupClient(CClient *pClient, const char* name)
{
	printf("--------------------------------------------------------------------------------\n");
	printf("%s() client name: %s\n", __FUNCTION__, name);
	if (!strcmp(name, CLIENT_NAME_LIGHT)) {
		pthread_mutex_lock(&gMainThreadMtx);
		mClientLight = pClient;
		pthread_mutex_unlock(&gMainThreadMtx);
	}

	return 0;
}

//typedef int (*clientCallback_t)(void * client);
int clientCallback(void * client, int cmd, void * data)
{
	CClient * pClient;
	//const char * msg;
	printf("%s() cmd: %d\n", __FUNCTION__, cmd);
	pClient = (CClient*) client;

	switch (cmd) {
	case eServerCmdClientExit:
		groupClient(NULL, (const char*)data); // data = NULL
		break;
	case eServerCmdRegister:
		groupClient(pClient, (const char*)data);
		break;
	case eServerCmdSpeechCommand:
	{
		//const char * speech_command = (const char *)data;
		memset(speech_command, 0, 1024);
		memcpy(speech_command, data, 1024);
		printf("pClient speech_command:\n");
		printf("%s", speech_command);
		printf("\n");

		pthread_mutex_lock(&gMainThreadMtx);
		client_request_type = eServerCmdSpeechCommand;
		// wakeup main thread loop to dispatch speech command

		pthread_cond_signal(&pthread_cond);
		pthread_mutex_unlock(&gMainThreadMtx);
	}
	break;
	default:
		printf("%s() unknown cmd: %d\n", __FUNCTION__, cmd);
		break;
	}

	DEBUG_LINE();
	return 0;
}


void  * HelperThread(void *arg)
{
	while (1)
	{
		pthread_mutex_lock(&gHelperThreadMtx);
		cout << "HelperThread : i am in HelperThread" <<endl;
		CLIENTLIST::iterator iter =clientlist.begin();
		for(iter;iter!=clientlist.end();)
		{
			CClient *pClient =(CClient*)*iter;
			if(pClient->IsExit())
			{
				clientlist.erase(iter++);
				delete pClient;
				pClient =NULL;
			}
			else
			{
				iter++;
			}
		}
		pthread_mutex_unlock(&gHelperThreadMtx);
		sleep(5);
	}
}

void * socketAccept(void *arg)
{
	pthread_detach(pthread_self());

	cout << ACCEPT<< "I am in socketaccept" <<endl;
	int client_sockfd;
	sockaddr_in client_sockaddr;
	socklen_t client_len = sizeof(client_sockaddr);
	memset(&client_sockaddr,0,client_len);

	while (1)
	{
		DEBUG_LINE();
		client_sockfd = accept(server_sockfd,(struct sockaddr*)&client_sockaddr,&client_len);
		DEBUG_LINE();

		if(client_sockfd== -1)
		{
			cout<<ACCEPT << "Server Accept Failed! " <<endl;
			exit(1);
		}
		else
		{
		DEBUG_LINE();
			CClient * pClient =new CClient (client_sockfd,client_sockaddr);
		DEBUG_LINE();

			pClient->setCallback(clientCallback);

		DEBUG_LINE();
			pthread_mutex_lock(&gHelperThreadMtx);
		DEBUG_LINE();
			clientlist.push_back(pClient);
			cout << "clientlist.begin()-----clientlist.end() :" <<endl;
			CLIENTLIST::iterator i;
			for(i =clientlist.begin();i!=clientlist.end();++i)
			{
				cout << *i <<endl;
			}
			pthread_mutex_unlock(&gHelperThreadMtx);
			char szIP[100];
			strcpy(szIP,inet_ntoa(client_sockaddr.sin_addr));
			cout <<ACCEPT << "-----------------client "<< szIP <<"  connect!!"<<endl;
			pClient->StartRunning();
		}
	}
}


bool CreateHelperAndAcceptThread(void)
{
	pthread_t tid_thread_help;
	pthread_create(&tid_thread_help,NULL,HelperThread,&server_sockfd);
	pthread_t tid_socket_accept;
	pthread_create(&tid_socket_accept,NULL,socketAccept,&server_sockfd);
	return 1;
}


int main(int argc, char **argv)
{
//	InitDB();//init database

	server_sockfd = socket(AF_INET,SOCK_STREAM,0);
	if( server_sockfd < 0)
	{
		cout << " SERVER : Create Socket Failed!"<<endl;
		exit(1);
	}

	int opt =1;
	setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	struct sockaddr_in server_sockaddr;
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(SERVER_SOCKET_PORT);
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t server_len = sizeof(server_sockaddr);
	if( bind(server_sockfd,(struct sockaddr*)&server_sockaddr,server_len)== -1 )
	{
		cout<< "Server Bind Port :" << SERVER_SOCKET_PORT << " Failed!"<<endl;
		exit(1);
	}

	if ( listen(server_sockfd, LENGTH_OF_LISTEN_QUEUE)== -1 )
	{
		cout<<"Server Listen Failed!"<<endl;
		exit(1);
	}

	pthread_mutex_init(&gHelperThreadMtx, NULL);
	pthread_mutex_init(&gMainThreadMtx, NULL);
	pthread_cond_init(&pthread_cond, NULL);

	CreateHelperAndAcceptThread();

	while(1)
	{
		printf("Server main thread...\n");
		//sleep(5);
		struct timespec abstime;

		clock_gettime(0, &abstime);
		abstime.tv_sec += 3;

		pthread_mutex_lock(&gMainThreadMtx);
		/* wait speech commands */
		pthread_cond_wait(&pthread_cond, &gMainThreadMtx);
		//pthread_cond_timedwait(&pthread_cond, &gMainThreadMtx, &abstime);
		// wakeup by speech client

		switch ( client_request_type ) {
		case eServerCmdSpeechCommand:
			if (mClientLight) {
				printf("server main thread loop, mClientLight= %p\n", mClientLight);

				// transmit speech_command to light client
				//mClientLight->setMessage(speech_msg);
				mClientLight->setSpeechCommand(speech_command);

				// wakeup light client
			}
			break;
		default:
			printf("unknown client_request_type: %d\n", client_request_type);
			break;
		}
		client_request_type = 0;
		pthread_mutex_unlock(&gMainThreadMtx);
	}
}


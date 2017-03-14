#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <pthread.h>

#include <debug.h>

#include <DeviceInterface.h>
#include <ActionBase.h>
#include <ActionRegister.h>
#include <ActionRegisterAck.h>
#include <ActionOK.h>
#include <ClientConfig.h>
#include  <ActionSendSpeechCommand.h>




#define SERVER_PORT    8887
#define BUFFER_SIZE 1024
#define MSG_MAX_SIZE 512
#define SERVER_IP_ADD "127.0.0.1"
#define MSG_TYPE "set"
#define random(x) (rand()%x)


using namespace std;

static pthread_mutex_t buf_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t thread_cond = PTHREAD_COND_INITIALIZER;
static int g_message_received;
static char sock_buffer_recv[BUFFER_SIZE];
static char sock_buffer_send[BUFFER_SIZE];
static char * gClientName;
static char * gClientID;
static ClientConfig * gConfig;
static int client_sockfd;


extern int init_speech_thread(int c);
extern int do_speech_loop(int client_sockfd, ClientConfig * gConfig);

void * sockeRecvFromServer(void *arg)
{
	pthread_detach(pthread_self());
	int sockfd =*(int*)arg;
	int recvlen = 0;

	while(1)
	{
		printf("begin receive ....\n");

		// clear buffer
		memset(sock_buffer_recv,0,BUFFER_SIZE);
		recvlen = recv(sockfd,sock_buffer_recv,BUFFER_SIZE,0);

		if(recvlen== -1)
		{
			perror("recv");
			exit(1);
		}
		else if(recvlen > 0)
		{
			printf("received:");
			char * p =sock_buffer_recv;
			cout << p <<endl;

			pthread_mutex_lock(&buf_mtx);
			g_message_received = 1;
			memcpy(sock_buffer_send,sock_buffer_recv,BUFFER_SIZE);
			// wakup main thread
			pthread_cond_signal(&thread_cond);
			pthread_mutex_unlock(&buf_mtx);

		}
		else if(recvlen == 0)
		{
			printf("socket end!!\n");
			exit (1);
		}
	}
	close(sockfd);
	return NULL;
}

static int sendMessageToServer(	int client_sockfd, char * msg)
{
	int len = strlen(msg);
	send(client_sockfd,msg,len,0);

	return 0;
}

static int register_client()
{
	DEBUG_LINE();
	printf("client_sockfd=%d\n", client_sockfd);

	pthread_mutex_lock(&buf_mtx);
	g_message_received = 0;

	ActionRegister * act = new ActionRegister(client_sockfd, gConfig);
	DEBUG_LINE();
	act->commitToServer();
	DEBUG_LINE();
	//delete act; // ???
	DEBUG_LINE();

	// wait server ACK?
	if (g_message_received == 0) {
		DEBUG_LINE();
		/* wait server ack */
		pthread_cond_wait(&thread_cond, &buf_mtx);
	}

	const char * action;
	cJSON *cJsonRoot;

	//pthread_mutex_lock(&buf_mtx);
	DEBUG_LINE();
	printf("%s\n", &sock_buffer_send[0]);

	// handle message in shared buffer
	cJsonRoot = messageStringToCJson((const char *)&sock_buffer_send[0]);
	action = getActionType(cJsonRoot);
	printf("recv action= %s\n", action);

	ActionBaseParser * parser;
	if (!strcmp(action, ACTION_TYPE_REGISTER_ACK)) {
		parser = new ActionBaseParser(cJsonRoot);
		const char * id = parser->getClientID();
		printf("register_client ack id: %s\n", id);
		gClientID = strdup(id);
	}
	else {
		printf("register_client ack wrong: %s\n", action);
		gClientID = NULL;
	}

	pthread_mutex_unlock(&buf_mtx);

	// set client ID
	gConfig->setClientID(gClientID);

	// delete cJsonRoot



	return 0;
}

#if 0
static int upload_config_to_server(IniParser* ini)
{
	ActionUploadConfig * act = new ActionUploadConfig(config);
	//msg = act->makeString();
	//sendMessageToServer(client_sockfd, msg);

	act->commitToServer();
	delete act;
	return 0;
}
#endif


int main(int argc, char **argv)
{
	char buffer[BUFFER_SIZE];
	//int client_sockfd;
	struct sockaddr_in server_addr;

	if((client_sockfd =socket(AF_INET,SOCK_STREAM,0))== -1)
	{
		perror("socket");
		exit(1);
	}

	DEBUG_LINE();
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);

	if (argc>1)
		server_addr.sin_addr.s_addr =inet_addr(argv[1]);
	else
		server_addr.sin_addr.s_addr =inet_addr(SERVER_IP_ADD);

	bzero(&(server_addr.sin_zero),sizeof(server_addr.sin_zero));

	socklen_t server_addr_length = sizeof(server_addr);

	if(connect(client_sockfd,(struct sockaddr*)&server_addr, server_addr_length) < 0)
	{
		printf("Can Not Connect To %s!\n",SERVER_IP_ADD);
		exit(1);
	}

	pthread_mutex_init(&buf_mtx, NULL);
	pthread_cond_init(&thread_cond, NULL);

	pthread_t tid_recv;
	pthread_create(&tid_recv,NULL,sockeRecvFromServer,&client_sockfd);


	DEBUG_LINE();
	// initialize ClientConfig
	gConfig = new ClientConfig();

#if 0
	// parse config.ini
	IniParser * ini = new IniParser(gConfig);

	ini->loadIniConfig(FILE_CONFIG_INI);
#else

	gConfig->setClientName(CLIENT_NAME_SPEECH);

#endif

	// register, get client ID.
	register_client();

	// upload config
	//upload_config_to_server(ini);

	init_speech_thread(0);

	// main thread loop
	while(1)
	{
		DEBUG_LINE();
#if 0
		pthread_mutex_lock(&buf_mtx);

		// wait server command
		pthread_cond_wait(&thread_cond, &buf_mtx);
		printf("client main thread wakeuped...\n");
		/* wait speech commands */
		pthread_mutex_unlock(&buf_mtx);
#endif
		// send speech command
		if (argc<3) {
			do_speech_loop(client_sockfd, gConfig);
		}
		else {
			ActionSendSpeechCommand * act;
			DEBUG_LINE();
			act = new ActionSendSpeechCommand(client_sockfd, gConfig);
			DEBUG_LINE();
			act->setSpeechCommand("TurnON");
			DEBUG_LINE();
			act->commitToServer();

			DEBUG_LINE();
			delete act; // ???
			DEBUG_LINE();
			sleep(3);
			act = new ActionSendSpeechCommand(client_sockfd, gConfig);
			DEBUG_LINE();
			act->setSpeechCommand("TurnOff");
			//act->setSpeechCommand("各位同事");
			DEBUG_LINE();
			act->commitToServer();

			DEBUG_LINE();
			delete act; // ???
			DEBUG_LINE();
			sleep(3);
		}
	}


	while(1)
	{
		const char *msg ="!!!client say hello to you!!!";
		int len = strlen(msg);
		send(client_sockfd,msg,len,0);
		int i = random(10);
		sleep(i);
	}


}

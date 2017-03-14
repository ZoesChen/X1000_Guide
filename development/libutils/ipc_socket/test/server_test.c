#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <limits.h>
#include "socket_local.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#define TEST_COMMUN_SOCKET "madeiningenic"

//please add the share lib to compile

void *server_send(void *arg);
void *server_recv(void *arg);
int main()
{
	int fd_s;
	struct sockaddr_in un;
	socklen_t len=sizeof(un);

	fd_s=socket_local_server("test.socket",ANDROID_SOCKET_NAMESPACE_FILESYSTEM,SOCK_STREAM);
	while(1)
	{
		int connfd;
		pthread_t tid1;
		pthread_t tid2;
		struct sockaddr_in client_addr;
		socklen_t cliaddr_len = sizeof(client_addr);
		connfd = accept(fd_s, (struct sockaddr*)&client_addr, &cliaddr_len);
		if(connfd < 0)
		{
			perror("accept");
			continue;
		}
		pthread_create(&tid1, NULL, (void *)server_send, (void *)connfd);
		pthread_detach(tid1);

		pthread_create(&tid2, NULL, (void *)server_recv, (void *)connfd);
		pthread_detach(tid2);
	}
	close(fd_s);
	return 0;
}

void *server_recv(void *arg)
{
	int btye;
	char char_recv[10];
	int connfd = (int)arg;
	while(1)
	{

		if(btye = recv(connfd,char_recv,sizeof(char_recv),0) == -1)
		{
			perror("recv");
		}
		printf("receive from client is :%s \n",char_recv);
	}
}

void *server_send(void *arg)
{
	int byte;
	char char_send[10];
	int connfd = (int)arg;
	while(1)
	{
		printf("please enter the context you want to send to client :");
		scanf("%s",char_send);
		if(byte = send(connfd,char_send,sizeof(char_send),0) == -1)
		{
			perror("send");
		}
	}
}

#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include "socket_local.h"
#include <sys/socket.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/prctl.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#define TEST_COMMUN_SOCKET "madeiningenic"

//please add the share lib to compile

void *client_recv(void *arg);
void *client_send(void *arg);
int main()
{
	int fd_c;
	int sockfd;
	fd_c=socket_local_client("test.socket",ANDROID_SOCKET_NAMESPACE_FILESYSTEM,SOCK_STREAM);
	sockfd= fd_c;
	pthread_t tid1;
	pthread_t tid2;
	pthread_create(&tid1, NULL, (void *)client_recv, (void *)sockfd);
	pthread_detach(tid1);
	pthread_create(&tid2, NULL, (void *)client_send, (void *)sockfd);
	pthread_detach(tid2);
	while(1);
	close(fd_c);
}

void *client_recv(void *arg)
{
	int byte;
	char char_recv[10];
	int sockfd = (int)arg;
	while(1)
	{
		if(byte = recv(sockfd,char_recv,sizeof(char_recv),0) == -1)
		{
			perror("recv");
		}
		printf("receive from server is :%s \n",char_recv);
	}
}

void *client_send(void *arg)
{
	int byte;
	char char_send[10];
	int sockfd = (int)arg;
	while(1)
	{
		printf("please enter the context you want to send to server :");
		scanf("%s",char_send);
		if(byte = send(sockfd,char_send,sizeof(char_send),0) == -1)
		{
			perror("send");
		}
	}
}

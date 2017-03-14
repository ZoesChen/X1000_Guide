#include <limits.h>
#include "socket_local.h"
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <sys/prctl.h>
#define TEST_COMMUN_SOCKET "made in ingenic"
TEST(LocaleSockt,FILESYSTEM){
    pid_t pid;
    int status;
    int ret;
    int fd=socket_local_server("test.socket",ANDROID_SOCKET_NAMESPACE_FILESYSTEM,SOCK_STREAM);
    EXPECT_LT(0,fd)<<"sever init error";
    pid=fork();
    if(!pid){
        int fd=socket_local_client("test.socket",ANDROID_SOCKET_NAMESPACE_FILESYSTEM,SOCK_STREAM);
	EXPECT_LT(0,fd)<<"client connect error";	//even if it accent fail it will be not enter to fail TEST
							//but it can print error information
	ret=write(fd,TEST_COMMUN_SOCKET,sizeof(TEST_COMMUN_SOCKET));
	while(true)
		sleep(1);
        close(fd);
    }else{
        struct sockaddr_un un;
        int con_fd;
	char buffer[sizeof(TEST_COMMUN_SOCKET)];
        socklen_t  len=sizeof(un);
        con_fd=accept(fd,(struct sockaddr *)&un,&len);
	EXPECT_LT(0,con_fd)<<"accept error";
	ret=read(con_fd,buffer,sizeof(TEST_COMMUN_SOCKET));
	kill(pid,SIGKILL);
	waitpid( pid, &status, 0 );
	EXPECT_STREQ(buffer,TEST_COMMUN_SOCKET);
        close(fd);
        close(con_fd);
    }
}

TEST(LocaleSocket,ABSTRACT){
    pid_t pid;
    int status;
    int ret;
    int fd=socket_local_server("test.socket",ANDROID_SOCKET_NAMESPACE_ABSTRACT,SOCK_STREAM);
    EXPECT_LT(0,fd)<<"sever init error";
    pid=fork();
    if(!pid){
        int fd=socket_local_client("test.socket",ANDROID_SOCKET_NAMESPACE_ABSTRACT,SOCK_STREAM);
        EXPECT_LT(0,fd)<<"client connect error";        //even if it accent fail it will be not enter to fail TEST
                                                        //but it can print error information
        ret=write(fd,TEST_COMMUN_SOCKET,sizeof(TEST_COMMUN_SOCKET));
        while(true)
                sleep(1);
        close(fd);
    }else{
        struct sockaddr_un un;
        int con_fd;
        char buffer[sizeof(TEST_COMMUN_SOCKET)];
        socklen_t  len=sizeof(un);
        con_fd=accept(fd,(struct sockaddr *)&un,&len);
        EXPECT_LT(0,con_fd)<<"accept error";
        ret=read(con_fd,buffer,sizeof(TEST_COMMUN_SOCKET));
        EXPECT_STREQ(buffer,TEST_COMMUN_SOCKET);
        kill(pid,SIGKILL);
        waitpid( pid, &status, 0 );
        close(fd);
        close(con_fd);
    }
}



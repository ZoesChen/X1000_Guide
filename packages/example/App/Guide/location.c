#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include "location.h"
#include "key.h"

static int McuFd;

int OpenMcuDev()
{
	printf("Enter into OpenMcuDev\n");
    McuFd = open(MCU_DEV, O_RDONLY);
	if(McuFd < 0) {
		printf("Open %s fail\n", MCU_DEV);
		return -1;
	}
    
	return 0;
}

int ReadLocationInfo(int *LocationInfo)
{
   fd_set rds;
   int ret;
   int valueFromKernel;

   FD_ZERO(&rds);
   FD_SET(McuFd,&rds);
   
   //int select(int maxfdp,fd_set *readfds,fd_set *writefds,fd_set *exceptfds,const struct timeval *timeout);
   ret = select(McuFd+1,&rds,NULL,NULL,NULL);
   if(ret < 0){
        printf("select error\n");
        exit(1);
   }
   if(FD_ISSET(McuFd,&rds))
   {
        read(McuFd,&valueFromKernel,sizeof(int));
   }
   printf("Read Buf %d\n",valueFromKernel);

   return 0;   
}
int CloseMcuDev()
{
   close(McuFd);
   return 0; 
}




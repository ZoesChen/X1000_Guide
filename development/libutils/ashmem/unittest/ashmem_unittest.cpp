#include <limits.h>
#include "ashmem.h"
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/mman.h>

#define TEST_COMMUN_SOCKET "made in ingenic"
#define TWO_PAGE (4096*2)
TEST(TEST_ASHMEM,ShareMemary){
	int fd;
	pid_t pid;
	fd=ashmem_create_region("test share", TWO_PAGE);
	ASSERT_LT(0,fd);
	void *address=mmap(0,TWO_PAGE,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
	ASSERT_TRUE(address);
	memset(address,0,TWO_PAGE);
	pid=fork();
	if(!pid){
		int son_fd=fd;	//son
		void *address=mmap(0,TWO_PAGE,PROT_READ | PROT_WRITE,MAP_SHARED,son_fd,0);
		memcpy(address,TEST_COMMUN_SOCKET,sizeof(TEST_COMMUN_SOCKET));
	}else{
		wait(NULL);
		EXPECT_STREQ((char *)address,TEST_COMMUN_SOCKET);
	}
	close(fd);
}

TEST(TEST_ASHMEM,MemaryUnpin){
        int fd;
        fd=ashmem_create_region("test share", TWO_PAGE);
        ASSERT_LT(0,fd);
	int prot = PROT_READ;
        prot |= PROT_WRITE;

	ashmem_set_prot_region(fd,prot);
	int pagesize=ashmem_get_size_region(fd);
        void *address=mmap(0,TWO_PAGE,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
	ASSERT_LE(0,ashmem_pin_region(fd, 0, 4096));
        ASSERT_LE(0,ashmem_unpin_region(fd,0,4096));
        ASSERT_TRUE(address);
	memset((char *)address,0,TWO_PAGE);

        close(fd);
}


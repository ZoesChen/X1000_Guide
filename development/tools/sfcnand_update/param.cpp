#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <errno.h>
#include "param.h"
using std::cout;
nand_chip_param::nand_chip_param(int page_offset,int param_offset)
{
    nand_magic=0;
    sysfilename="/dev/mtd0";
    this->pages_offset=page_offset;
    this->param_offset=param_offset;
    fd=open(sysfilename.data(),O_RDONLY);
    if(fd < 0){
        perror("can't find mtd0");
        exit(1);
    }
}
int nand_chip_param::get_param_from_file()
{
    flock pro_mux;
    pro_mux.l_type=F_RDLCK;
    pro_mux.l_whence=SEEK_SET;
    pro_mux.l_start=0;
    pro_mux.l_len=0;
    if(fd>=0){
        fcntl(fd,F_SETLKW,&pro_mux);
        buffer=(char *)malloc(sizeof(int));
        memset(buffer,0,sizeof(int));
        lseek(fd,pages_offset,SEEK_SET);
        read(fd,buffer,sizeof(int));
        page_size=*buffer*1024;
        free(buffer);
        buffer=(char *)malloc(page_size);
        memset(buffer,0,sizeof(int));
        lseek(fd,param_offset,SEEK_SET);
        read(fd,buffer,param_offset%page_size);
        nand_magic=*(int32_t *)(buffer);
        if(nand_magic!=0x6e616e64){
            printf("waring:your burner is to old!\n");
            return -1;
        }

    }
    return 0;
}
int nand_chip_param::get_page_size()
{
    return page_size;
}
const char *nand_chip_param::get_param_buffer()
{

    return this->buffer;
}
nand_chip_param::~nand_chip_param()
{
    free(buffer);
}

#include "uboot.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using std::cout;
uboot::uboot(char *path,int page_size,int page_offset,int param_offset)
{

    fd= open(path,O_WRONLY);
    if(fd<0){
        perror(path);
        cout<<"can't find u-boot"<<"\n";
        exit(1);
    }
    if(page_offset)
        this->pagesize_offset=page_offset;
    if(param_offset)
        this->param_offset=param_offset;
    this->page_size=page_size;
}

int uboot::change_param(const char *param)
{
    char page_write=page_size/1024;
    if(fd>=0){
        lseek(fd,pagesize_offset,SEEK_SET);
        write(fd,&page_write,sizeof(char));//write pagesize
        lseek(fd,param_offset,SEEK_SET);
        write(fd,param,param_offset%page_size);
    }
    return 0;

}
uboot::~uboot()
{

}

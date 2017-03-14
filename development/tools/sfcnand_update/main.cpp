#include <iostream>
#include<stdio.h>
#include<stdlib.h>
#include "param.h"
#include"uboot.h"
#define SPIFLASH_PARAMER_OFFSET (0x3c00)
using std::cout;
using std::string;
static int pages_offset=11;
static int param_offset=SPIFLASH_PARAMER_OFFSET;
int main(int argc,char **argv)
{
    int ret;
    string write_cmd;
    class nand_chip_param s(pages_offset,param_offset);
    if(argc<2){
        cout<<"please check param\n";
        return 1;
    }
    ret=s.get_param_from_file();
    class uboot m(argv[1],s.get_page_size(),pages_offset,param_offset);
    if(ret!=-1)
        m.change_param(s.get_param_buffer());

    ret=system("flash_erase /dev/mtd0 0 0");
    if(ret!=0){
        perror("error in erase");
    }
    write_cmd.clear();
    write_cmd+="nandwrite /dev/mtd0 ";
    write_cmd+=argv[1];
    write_cmd+=" -p -N";
    cout<<write_cmd<<"\n";
    ret=system(write_cmd.data());

    if(ret!=0){
        perror("error in write");
    }
    return 0;
}

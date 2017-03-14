#ifndef __UBOOT_H__
#define __UBOOT_H__
#include<sys/types.h>  /*提供类型pid_t,size_t的定义*/
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/file.h>
class uboot{
public:
    explicit uboot(char *path,int page_size,int pageoffset=0,int paramoffset=0);
    int open_file(char *);
    int change_pagesize(int pagesize);
    int change_param(const char *);
    ~uboot();
    int get_status();
private:
    int fd;
    int page_size;
    int pagesize_offset;
    int param_offset;
};
#endif

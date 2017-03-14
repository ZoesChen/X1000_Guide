#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>		/* for convenience */
#include <stddef.h>		/* for offsetof */
#include <string.h>		/* for convenience */
#include <unistd.h>		/* for convenience */
#include <errno.h>		/* for definition of errno */
#include <stdarg.h>		/* ISO C variable arguments */
#include <fcntl.h>
#include <sys/mman.h>

#include <getopt.h>
#include "x2d.h"
#include "dmmu.h"
#include "format.h"

#define SRC_W 480
#define SRC_H 800
#define DST_W 480
#define DST_H 800

#define SRC_SIZE SRC_W*SRC_H + SRC_W*SRC_H/2   //indicate yuv420 format
#define DST_SIZE DST_W*DST_H*4                 //indicate rgb888
#define FB_DEV_NAME  "/dev/fb0"

#define DISP_FMT_RGB888		(1 << 28)
#define DISP_FMT_RGB565		(1 << 27)
void *fb_base = NULL;
void *src_buf = NULL;
void *dst_buf = NULL;
char *y_buf = NULL;
char *u_buf = NULL;
char *v_buf = NULL;
char format_name[64];
struct option options[] = {
	{"select", 0, 0, 's'},
	{0,0,0,0}
};
static char string[] = "s:";

#define print_opt_help(opt_index, help_str)				\
	do {								\
		printf("\t--%s\t\t-%c\t%s", options[opt_index].name, (char)options[opt_index].val, help_str); \
	} while (0)

#define STR_FMT(s, fmt) \
	{.str = s, .format = fmt }

void help_message()
{
	printf("                          \n----------------help---------------------------\n");
	print_opt_help(0, "select colobar covert format --->rgb888 or rgb565");
}



void generate_yuv420_color_bar()
{
	int i , j;
	int _h = SRC_H / 4;
	int w  = SRC_W;

	for(j = 0; j < _h; j ++) {
		for(i = 0; i < w; i ++) {
			y_buf[i + j * w] = (unsigned char)(0.299*0xff);
			y_buf[w * _h + i + j * w] = (unsigned char)(0.587*0xff);
			y_buf[w * _h * 2 + i + j * w] = (unsigned char)(0.114*0xff);
			y_buf[w * _h * 3 + i + j * w] = 0xff;
		}
	}
	for(j = 0; j < (_h/4); j ++) {
		for(i = 0; i < w; i ++) {
			u_buf[i + j * w] = (unsigned short)(128 - 0.1687*0xff);
			u_buf[w * (_h/4) + i + j * w] = (unsigned short)(128 - 0.3313*0xff);
			u_buf[w * (_h/4) * 2 + i + j * w] = (unsigned short)(128 + 0.5*0xff);
			u_buf[w * (_h/4) * 3 + i + j * w] = (unsigned short)(128 + (-0.1687 - 0.3313 + 0.5)*0xff);

			v_buf[i + j * w] = (unsigned short)(128 + 0.5*0xff);
			v_buf[w * (_h/4) + i + j * w] = (unsigned short)(128 - 0.4187*0xff);
			v_buf[w * (_h/4) * 2 + i + j * w] =(unsigned short)(128 - 0.0813*0xff);
			v_buf[w * (_h/4) * 3 + i + j * w] = (unsigned short)(128 + (0.5 - 0.4187 - 0.0813)*0xff);
		}
	}
}

int main(int argc, char *argv[])
{
	int ret = -1;
	int fb_fd = -1;
	int select_format = 1;
	int format = 0;
	while(1)
	{
		if ((ret = getopt_long(argc, argv, string, options, NULL)) == -1)
			break;
		switch (ret) {
			case 's':
				strcpy(format_name, optarg);
				printf("select format %s\n", optarg);
				break;
			default:
				strcpy(format_name, "rgb888");
				help_message();
				break;
		}

	}
	src_buf = malloc(SRC_SIZE);
	if (src_buf == NULL)
	{
		printf("malloc buf faild!!\n");
		return -1;
	}
	memset(src_buf, 0, SRC_SIZE);

	y_buf = (char *)src_buf;
	u_buf = y_buf + SRC_H * SRC_W;
	v_buf = u_buf + SRC_H * SRC_W / 4;
	if (!strcmp(format_name, "rgb888")){
		generate_yuv420_color_bar();
	}
	else if(!strcmp(format_name, "rgb565")){
		generate_yuv420_color_bar();
	}
	if (x2dConvertInit() < 0)
	{
		printf("X2d init error!!\n");
		goto deinit;
	}

	if ((fb_fd = open(FB_DEV_NAME, O_RDWR)) < 0)
	{
		printf("open fb error!!!\n");
		exit(1);
	}
	fb_base = mmap(0, DST_SIZE, PROT_READ|PROT_WRITE,
			                MAP_SHARED, fb_fd, 0);
	if (NULL == fb_base)
	{
		printf("mmap faild !!!\n");
		goto unmmap;
	}
	if(!strcmp(format_name, "rgb888"))
		convertFromYUV420ArrayToRGB888(y_buf, u_buf, v_buf, SRC_W, SRC_H, SRC_W, SRC_W / 2, fb_base, 480, 800,
				HAL_PIXEL_FORMAT_RGBA_8888, 480 * 4, 0, 0, 480, 800);
	if (!strcmp(format_name, "rgb565")){
		convertFromYUV420ArrayToRGB565(y_buf, u_buf, v_buf, SRC_W, SRC_H, SRC_W, SRC_W / 2, fb_base, 480, 800,
				HAL_PIXEL_FORMAT_RGB_565, 480 * 4, 0, 0, 480, 800);
	}
	if (close(fb_fd) < 0)
		printf("close fd faild!!!\n");
	free(src_buf);
unmmap:
	munmap(fb_base, DST_SIZE);
deinit:
	x2dConvertDeinit();

	return 0;
}

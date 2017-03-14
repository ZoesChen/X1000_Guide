#ifndef __SAVEJPEG_H__
#define __SAVEJPEG_H__

#include "headers.h"

extern int ycbcr422_planar_to_jpeg(__u8 *y, __u8 *cb, __u8 *cr,
				   int image_width, int image_height, FILE *fp, int quality);

extern int yuv422_packed_to_jpeg(__u8 *data, __u32 fmt,
			  int image_width, int image_height,
			  FILE *fp, int quality);

extern int yuv422_sep_to_jpeg(__u8 *data, int image_width, int image_height, FILE *fp, int quality);

int get_picture(unsigned char *buf,int size, const char *name);

int get_pictureYV2(unsigned char *buf,int width,int height, const char *name);

#endif /* __SAVEJPEG_H__ */

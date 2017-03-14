#ifndef __SAVEJPEG_H__
#define __SAVEJPEG_H__

#include "headers.h"

//extern int ycbcr422_planar_to_jpeg(__u8 *y, __u8 *cb, __u8 *cr,
//				   int image_width, int image_height, FILE *fp, int quality);
extern int save_jpeg_userptr;

extern int yuv422_sep_to_jpeg(__u8 *data, int image_width, int image_height, FILE *fp, int quality);
extern int yuv422_packed_to_jpeg(void *handle, const __u8 *frm, FILE *fp, struct display_info *dis, int quality);

extern int yuv420_sep_to_jpeg(__u8 *data, int mb, int image_width, int image_height, FILE *fp, int quality);

extern int yuv444_sep_to_jpeg(__u8 *data, int image_width, int image_height, FILE *fp, int quality);
extern int yuv444_packed_to_jpeg(__u8 *data, FILE *fp, struct display_info *dis, int quality);

extern int itu656_rearrange(__u8 *src, __u8 *dest, int fmt, int width, int height);
#endif /* __SAVEJPEG_H__ */

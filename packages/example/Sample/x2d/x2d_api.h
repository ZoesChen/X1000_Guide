
#ifndef __X2D_API_H__
#define __X2D_API_H__

int x2dConvertInit();
void x2dConvertDeinit();

/**
 * @brief
 * This procedure is through the x2d from YUV420 into RGB888 format
 *
 * @param ysrc_addr	yuv420 date y addres
 * @param usrc_addr	yuv420 date u addres
 * @param vsrc_addr	yuv420 date v addres
 * @param src_w		source date width
 * @param src_h		source date height
 * @param ysrc_stride	y date stride
 * @param vsrc_stride	v date stride
 * @param dst_addr	destination address
 * @param dst_w		destination width
 * @param dst_h		destination height
 * @param dst_format    destination format
 * @param dst_stride	destination stride
 * @param dstRect_x	destination x offsetof
 * @param dstRect_y	destination y offsetof
 * @param dstRect_w	destination width
 * @param dstRect_h	destination height
 */
void convertFromYUV420ArrayToRGB888(void *ysrc_addr, void *usrc_addr, void *vsrc_addr, int src_w, int src_h,
		int ysrc_stride, int vsrc_stride,
		void *dst_addr, int dst_w, int dst_h, int dst_format, int dst_stride,
		int dstRect_x,int dstRect_y,int dstRect_w,int dstRect_h);



void convertFromYUV420ArrayToRGB565(void *ysrc_addr, void *usrc_addr, void *vsrc_addr,
		int src_w, int src_h,
		int ysrc_stride, int  uvsrc_stride,
		void  *dst_addr, int dst_w, int dst_h, int dst_format, int dst_stride,
		int dstRect_x,int dstRect_y,int dstRect_w,int dstRect_h);
#endif

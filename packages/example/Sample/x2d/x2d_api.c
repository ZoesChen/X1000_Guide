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
/*#include <cutils/log.h>*/
#include"x2d_api.h"
#include "x2d.h"
#include "dmmu.h"
#include "format.h"

static unsigned int tlb_base_phys;
struct x2d_hal_info *mX2d = NULL;
struct dmmu_mem_info x2d_YSrcMemInfo, x2d_USrcMemInfo, x2d_VSrcMemInfo, x2d_DstMemInfo;
struct x2d_glb_info *x2d_GlbInfo = NULL;
struct x2d_dst_info *x2d_DstInfo = NULL;
struct src_layer (*x2d_Layer)[4] = NULL;

int x2dConvertInit()
{
	if (dmmu_init() < 0) {
		printf("dmmu_init failed\n");
		return -1;
	}
	/* dmmu get tlb base address */
	if (dmmu_get_page_table_base_phys(&tlb_base_phys) < 0) {
		printf("dmmu get tlb base phys failed\n");
		return -1;
	}

	/* open x2d */
	if ((x2d_open(&mX2d)) < 0) {
		printf("x2d open failed\n");
		return -1;
	}

}

void x2dConvertDeinit()
{
	dmmu_deinit();
	x2d_release(&mX2d);

}


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
		int dstRect_x,int dstRect_y,int dstRect_w,int dstRect_h) {
	int ret = 0;

	if (!dst_addr){
		printf("dst_buf calloc failed\n");
		exit(-1);
	}

	memset(&x2d_YSrcMemInfo, 0, sizeof(struct dmmu_mem_info));
	memset(&x2d_DstMemInfo, 0, sizeof(struct dmmu_mem_info));

	x2d_YSrcMemInfo.vaddr = ysrc_addr;
	x2d_YSrcMemInfo.size = src_w * src_h + src_w * src_h / 2;

	x2d_DstMemInfo.vaddr = dst_addr;
	x2d_DstMemInfo.size = dst_w * dst_h * 4;

	if ((ret = dmmu_map_user_memory(&x2d_YSrcMemInfo)) < 0)
		printf("Error: dmmu map src mem info failed!!");
	if ((ret = dmmu_map_user_memory(&x2d_DstMemInfo)) < 0)
		printf("Error: dmmu map dst mem info failed!!");


	x2d_Layer = mX2d->layer;
	x2d_GlbInfo = mX2d->glb_info;
	x2d_DstInfo = mX2d->dst_info;

	x2d_GlbInfo->layer_num = 1;
	x2d_Layer[0]->format = HAL_PIXEL_FORMAT_JZ_YUV_420_P;
	x2d_Layer[0]->in_width = src_w;
	x2d_Layer[0]->in_height = src_h;
	x2d_Layer[0]->out_width = dstRect_w;
	x2d_Layer[0]->out_height = dstRect_h;

	x2d_Layer[0]->addr = (unsigned int)ysrc_addr;
	x2d_Layer[0]->u_addr = (unsigned int)usrc_addr;
	x2d_Layer[0]->v_addr = (unsigned int)vsrc_addr;
	x2d_Layer[0]->y_stride = ysrc_stride;
	x2d_Layer[0]->v_stride = vsrc_stride;

	x2d_Layer[0]->glb_alpha_en = 1;
	x2d_Layer[0]->global_alpha_val = 0xff;
	x2d_Layer[0]->preRGB_en = 1;
	x2d_Layer[0]->out_w_offset = dstRect_x;
	x2d_Layer[0]->out_h_offset = dstRect_y;

	if (x2d_src_init(mX2d) < 0)
		printf("Error: x2d src hal x2d_init failed!!");

	/* x2d_init x2d dst */
	x2d_GlbInfo->tlb_base = tlb_base_phys;
	x2d_DstInfo->dst_address = (unsigned int)dst_addr;
	x2d_DstInfo->dst_format = dst_format;
	x2d_DstInfo->dst_width = dst_w;
	x2d_DstInfo->dst_height = dst_h;
	x2d_DstInfo->dst_stride = dst_stride;
	x2d_DstInfo->dst_alpha_val = 0xff;
	if (x2d_dst_init(mX2d) < 0)
		printf("Error: x2d dst x2d_init failed!!");

	/* start x2d */
	if ((ret = x2d_start(mX2d)) < 0)
		printf("Error: x2d_start failed!!");

	/* unmap address */
	if ((ret = dmmu_unmap_user_memory(&x2d_YSrcMemInfo)) < 0)
		printf("dmmu unmap src mem info failed\n");
	if ((ret = dmmu_unmap_user_memory(&x2d_DstMemInfo)) < 0)
		printf("dmmu unmap dst mem info failed\n");

}


/**
 * @brief  convertFromYUV420ArrayToRGB565
 * This procedure is through the x2d from YUV420 into RGB565 format
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
void convertFromYUV420ArrayToRGB565(void *ysrc_addr, void *usrc_addr, void *vsrc_addr,
		int src_w, int src_h,
		int ysrc_stride, int  uvsrc_stride,
		void  *dst_addr, int dst_w, int dst_h, int dst_format, int dst_stride,
		int dstRect_x,int dstRect_y,int dstRect_w,int dstRect_h) {
	int ret = 0;

	/* map address */
	memset(&x2d_YSrcMemInfo, 0, sizeof(struct dmmu_mem_info));
	memset(&x2d_DstMemInfo, 0, sizeof(struct dmmu_mem_info));

	x2d_YSrcMemInfo.vaddr = ysrc_addr;
	x2d_YSrcMemInfo.size = src_w * src_h + src_w * src_h / 2 ;

//	x2d_USrcMemInfo.vaddr = (void *)usrc_addr;
//	//x2d_USrcMemInfo.size = uvsrc_stride * src_h;
//	x2d_USrcMemInfo.size = src_w * src_h / 4;
//
//	x2d_VSrcMemInfo.vaddr = (void *)vsrc_addr;
//	//x2d_VSrcMemInfo.size = uvsrc_stride * src_h;
//	x2d_VSrcMemInfo.size = src_w * src_h / 4;

	x2d_DstMemInfo.vaddr = dst_addr;
	x2d_DstMemInfo.size = dst_w * dst_h * 2;

	//memset((void *)dst_addr, 0, dst_stride * dst_h);
	dmmu_match_user_mem_tlb(dst_addr,dst_stride * dst_h);

	if ((ret = dmmu_map_user_memory(&x2d_YSrcMemInfo)) < 0)
		printf("Error: dmmu map src mem info failed!!");
//	if ((ret = dmmu_map_user_memory(&x2d_USrcMemInfo)) < 0)
//		printf("Error: dmmu map src mem info failed!!");
//	if ((ret = dmmu_map_user_memory(&x2d_VSrcMemInfo)) < 0)
//		printf("Error: dmmu map src mem info failed!!");
	if ((ret = dmmu_map_user_memory(&x2d_DstMemInfo)) < 0)
		printf("Error: dmmu map dst mem info failed!!");

	x2d_Layer = mX2d->layer;
	x2d_GlbInfo = mX2d->glb_info;
	x2d_DstInfo = mX2d->dst_info;

	x2d_GlbInfo->layer_num = 1;
	x2d_Layer[0]->format = HAL_PIXEL_FORMAT_JZ_YUV_420_P;
	x2d_Layer[0]->in_width = src_w;
	x2d_Layer[0]->in_height = src_h;
	x2d_Layer[0]->out_width = dstRect_w;
	x2d_Layer[0]->out_height = dstRect_h;

	x2d_Layer[0]->addr = (unsigned int)ysrc_addr;
	x2d_Layer[0]->y_stride = ysrc_stride;
	x2d_Layer[0]->u_addr = (unsigned int)usrc_addr;
	x2d_Layer[0]->v_addr = (unsigned int)vsrc_addr;
	x2d_Layer[0]->v_stride = uvsrc_stride;

	x2d_Layer[0]->glb_alpha_en = 1;
	x2d_Layer[0]->global_alpha_val = 0xff;
	x2d_Layer[0]->preRGB_en = 0;
	x2d_Layer[0]->out_w_offset = dstRect_x;
	x2d_Layer[0]->out_h_offset = dstRect_y;

	if (x2d_src_init(mX2d) < 0)
		printf("Error: x2d src hal x2d_init failed!!");

	/* x2d_init x2d dst */
	x2d_GlbInfo->tlb_base = tlb_base_phys;
	x2d_DstInfo->dst_address = (unsigned int)dst_addr;
	x2d_DstInfo->dst_format = dst_format;
	x2d_DstInfo->dst_width = dst_w;
	x2d_DstInfo->dst_height = dst_h;
	x2d_DstInfo->dst_stride = dst_stride;
	x2d_DstInfo->dst_alpha_val = 0xff;
	if (x2d_dst_init(mX2d) < 0)
		printf("Error: x2d dst x2d_init failed!!");

	/* start x2d */
	if ((ret = x2d_start(mX2d)) < 0)
		printf("Error: x2d_start failed!!");

	/* unmap address */
	if ((ret = dmmu_unmap_user_memory(&x2d_YSrcMemInfo)) < 0)
		printf("dmmu unmap src mem info failed\n");
	if ((ret = dmmu_unmap_user_memory(&x2d_DstMemInfo)) < 0)
		printf("dmmu unmap dst mem info failed\n");
}


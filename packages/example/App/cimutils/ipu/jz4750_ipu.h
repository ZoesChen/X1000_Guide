/*
 * linux/drivers/video/jz4750_ipu.h -- Ingenic Jz4750 On-Chip LCD frame buffer device
 *
 * Copyright (C) 2010-2011, Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __JZ4750_IPU_H__
#define __JZ4750_IPU_H__

#include <linux/types.h>

/*************************************************************************
 * IPU (Image Processing Unit)
 *************************************************************************/

/* IPU Control Register */
#define REG_IPU_CTRL			(IPU_BASE + 0x0)

/* IPU Status Register */
#define REG_IPU_STATUS			(IPU_BASE + 0x4)

/* Data Format Register */
#define REG_IPU_D_FMT			(IPU_BASE + 0x8)

/* Input Y or YUV422 Packaged Data Address Register */
#define REG_IPU_Y_ADDR			(IPU_BASE + 0xc)

/* Input U Data Address Register */
#define REG_IPU_U_ADDR			(IPU_BASE + 0x10)

/* Input V Data Address Register */
#define REG_IPU_V_ADDR			(IPU_BASE + 0x14)

/* Input Geometric Size Register */
#define REG_IPU_IN_FM_GS		(IPU_BASE + 0x18)

/* Input Y Data Line Stride Register */
#define REG_IPU_Y_STRIDE		(IPU_BASE + 0x1c)

/* Input UV Data Line Stride Register */
#define REG_IPU_UV_STRIDE		(IPU_BASE + 0x20)

/* Output Frame Start Address Register */
#define REG_IPU_OUT_ADDR		(IPU_BASE + 0x24)

/* Output Geometric Size Register */
#define REG_IPU_OUT_GS			(IPU_BASE + 0x28)

/* Output Data Line Stride Register */
#define REG_IPU_OUT_STRIDE		(IPU_BASE + 0x2c)

/* Resize Coefficients Table Index Register */
#define REG_IPU_RSZ_COEF_INDEX		(IPU_BASE + 0x30)

/* CSC C0 Coefficient Register */
#define REG_IPU_CSC_CO_COEF		(IPU_BASE + 0x34)

/* CSC C1 Coefficient Register */
#define REG_IPU_CSC_C1_COEF		(IPU_BASE + 0x38)

/* CSC C2 Coefficient Register */
#define REG_IPU_CSC_C2_COEF		(IPU_BASE + 0x3c)

/* CSC C3 Coefficient Register */
#define REG_IPU_CSC_C3_COEF		(IPU_BASE + 0x40)

/* CSC C4 Coefficient Register */
#define REG_IPU_CSC_C4_COEF		(IPU_BASE + 0x44)

/* Horizontal Resize Coefficients Look Up Table Register group */
#define REG_IPU_HRSZ_LUT_BASE 		(IPU_BASE + 0x48)

/* Virtical Resize Coefficients Look Up Table Register group */
#define REG_IPU_VRSZ_LUT_BASE 		(IPU_BASE + 0x4c)

/* CSC Offset Parameter Register */
#define REG_IPU_CSC_OFSET_PARA		(IPU_BASE + 0x50)

/* Input Y Physical Table Address Register */
#define REG_IPU_Y_PHY_T_ADDR		(IPU_BASE + 0x54)

/* Input U Physical Table Address Register */
#define REG_IPU_U_PHY_T_ADDR		(IPU_BASE + 0x58)

/* Input V Physical Table Address Register */
#define REG_IPU_V_PHY_T_ADDR		(IPU_BASE + 0x5c)

/* Output Physical Table Address Register */
#define REG_IPU_OUT_PHY_T_ADDR		(IPU_BASE + 0x60)

/* IPU Control */
#define IPU_CTRL_DFIX_SEL			(1 << 17)
#define IPU_CTRL_FIELD_SEL			(1 << 16)
#define IPU_CTRL_FIELD_CONF_EN			(1 << 15)
#define IPU_CTRL_DISP_SEL			(1 << 14)
#define IPU_CTRL_DPAGE_MAP			(1 << 13)
#define IPU_CTRL_SPAGE_MAP 			(1 << 12)
#define IPU_CTRL_LCDC_SEL			(1 << 11)
#define IPU_CTRL_SPKG_SEL			(1 << 10)
#define IPU_CTRL_V_SCALE			(1 << 9)
#define IPU_CTRL_H_SCALE			(1 << 8)
#define IPU_CTRL_IPU_RST			(1 << 6)
#define IPU_CTRL_FM_IRQ_EN			(1 << 5)
#define IPU_CTRL_CSC_EN				(1 << 4)
#define IPU_CTRL_VRSZ_EN			(1 << 3)
#define IPU_CTRL_HRSZ_EN			(1 << 2)
#define IPU_CTRL_IPU_RUN			(1 << 1)
#define IPU_CTRL_CHIP_EN			(1 << 0)

/* IPU Status */
#define IPU_STAT_SIZE_ERR			(1 << 2)
#define IPU_STAT_FMT_ERR			(1 << 1)
#define IPU_STAT_OUT_END			(1 << 0)

/* IPU Data Format */

#define IPU_D_FMT_RGB_OUT_888_FMT		(1 << 24)

#define IPU_D_FMT_RGB_OUT_OFT_MASK		(0x7 << 21)

#define IPU_D_FMT_RGB_OUT_OFT_RGB		(0 << 21)
#define IPU_D_FMT_RGB_OUT_OFT_RBG		(1 << 21)
#define IPU_D_FMT_RGB_OUT_OFT_GBR		(2 << 21)
#define IPU_D_FMT_RGB_OUT_OFT_GRB		(3 << 21)
#define IPU_D_FMT_RGB_OUT_OFT_BRG		(4 << 21)
#define IPU_D_FMT_RGB_OUT_OFT_BGR		(5 << 21)

#define IPU_D_FMT_OUT_FMT_MASK			(0x3 << 19)

#define IPU_D_FMT_OUT_FMT_RGB555		(0 << 19)
#define IPU_D_FMT_OUT_FMT_RGB565		(1 << 19)
#define IPU_D_FMT_OUT_FMT_RGB888		(2 << 19)
#define IPU_D_FMT_OUT_FMT_YUV422		(3 << 19)

#define IPU_D_FMT_YUV_PKG_OUT_OFT_MASK		(0x7 << 16)

#define IPU_D_FMT_YUV_PKG_OUT_OFT_Y1UY0V	(0 << 16)
#define IPU_D_FMT_YUV_PKG_OUT_OFT_Y1VY0U	(1 << 16)
#define IPU_D_FMT_YUV_PKG_OUT_OFT_UY1VY0	(2 << 16)
#define IPU_D_FMT_YUV_PKG_OUT_OFT_VY1UY0	(3 << 16)
#define IPU_D_FMT_YUV_PKG_OUT_OFT_Y0UY1V	(4 << 16)
#define IPU_D_FMT_YUV_PKG_OUT_OFT_Y0VY1U	(5 << 16)
#define IPU_D_FMT_YUV_PKG_OUT_OFT_UY0VY1	(6 << 16)
#define IPU_D_FMT_YUV_PKG_OUT_OFT_VY0UY1	(7 << 16)

#define IPU_D_FMT_IN_OFT_MASK			(0x3 << 2)

#define IPU_D_FMT_IN_OFT_Y1UY0V			(0 << 2)
#define IPU_D_FMT_IN_OFT_Y1VY0U			(1 << 2)
#define IPU_D_FMT_IN_OFT_UY1VY0			(2 << 2)
#define IPU_D_FMT_IN_OFT_VY1UY0			(3 << 2)

#define IPU_D_FMT_IN_FMT_MASK			(0x3 << 0)

#define IPU_D_FMT_IN_FMT_YUV420			(0 << 0)
#define IPU_D_FMT_IN_FMT_YUV422			(1 << 0)
#define IPU_D_FMT_IN_FMT_YUV444			(2 << 0)
#define IPU_D_FMT_IN_FMT_YUV411			(3 << 0)

/* Input Geometric Size Register */
#define IPU_IN_FM_GS_W_MASK			(0xFFF)
#define IPU_IN_FM_GS_W(n)			((n) << 16)

#define IPU_IN_FM_GS_H_MASK			(0xFFF)
#define IPU_IN_FM_GS_H(n)			((n) << 0)

/* Input UV Data Line Stride Register */
#define IPU_UV_STRIDE_U_S_MASK			(0x1FFF)
#define IPU_UV_STRIDE_U_S(n) 			((n) << 16)

#define IPU_UV_STRIDE_V_S_MASK			(0x1FFF)
#define IPU_UV_STRIDE_V_S(n)			((n) << 0)

/* Output Geometric Size Register */
#define IPU_OUT_GS_W_MASK			(0x7FFF)
#define IPU_OUT_GS_W(n)				((n) << 16)

#define IPU_OUT_GS_H_MASK			(0x1FFF)
#define IPU_OUT_GS_H(n)				((n) << 0)

/* Resize Coefficients Table Index Register */
#define IPU_RSZ_COEF_INDEX_HE_IDX_MASK		(0x1F)
#define IPU_RSZ_COEF_INDEX_HE_IDX(n)		((n) << 16)

#define IPU_RSZ_COEF_INDEX_VE_IDX_MASK		(0x1F)
#define IPU_RSZ_COEF_INDEX_VE_IDX(n)		((n) << 0)

/* Resize Coefficients Look Up Table Register group */
#define IPU_HRSZ_COEF_LUT_START			(1 << 12)

#define IPU_HRSZ_COEF_LUT_W_COEF_MASK		(0x3FF)
#define IPU_HRSZ_COEF_LUT_W_COEF(n)		((n) << 2)

#define IPU_HRSZ_COEF_LUT_IN_EN			(1 << 1)
#define IPU_HRSZ_COEF_LUT_OUT_EN		(1 << 0)

#define IPU_VRSZ_COEF_LUT_START			(1 << 12)

#define IPU_VRSZ_COEF_LUT_W_COEF_MASK		(0x3FF)
#define IPU_VRSZ_COEF_LUT_W_COEF(n)		((n) << 2)

#define IPU_VRSZ_COEF_LUT_IN_EN			(1 << 1)
#define IPU_VRSZ_COEF_LUT_OUT_EN		(1 << 0)

/* CSC Offset Parameter Register */
#define IPU_CSC_OFFSET_PARA_CHROM_OF_MASK	(0xFF)
#define IPU_CSC_OFFSET_PARA_CHROM_OF(n)		((n) << 16)

#define IPU_CSC_OFFSET_LUMA_OF_MASK		(0xFF)
#define IPU_CSC_OFFSET_LUMA_OF(n)		((n) << 0)

/* ioctl commands */
#define JZIPU_IO_PUT_IMG	0x4701

#undef IPU_LUT_LEN
#define IPU_LUT_LEN 32

enum {
	JZIPU_IN_FMT_YUV420,
	JZIPU_IN_FMT_YUV422,
	JZIPU_IN_FMT_YUV444,
	JZIPU_IN_FMT_YUV411,
};

enum {
	JZIPU_IN_OFT_Y1UY0V,
	JZIPU_IN_OFT_Y1VY0U,
	JZIPU_IN_OFT_UY1VY0,
	JZIPU_IN_OFT_VY1UY0,
};

enum {
	JZIPU_OUT_OFT_Y1UY0V,
	JZIPU_OUT_OFT_Y1VY0U,
	JZIPU_OUT_OFT_UY1VY0,
	JZIPU_OUT_OFT_VY1UY0,
	JZIPU_OUT_OFT_Y0UY1V,
	JZIPU_OUT_OFT_Y0VY1U,
	JZIPU_OUT_OFT_UY0VY1,
	JZIPU_OUT_OFT_VY0UY1,
};

enum {
	JZIPU_OUT_FMT_RGB555,
	JZIPU_OUT_FMT_RGB565,
	JZIPU_OUT_FMT_RGB888,
	JZIPU_OUT_FMT_YUV422,
};

enum {
	JZIPU_OUT_OFT_RGB,
	JZIPU_OUT_OFT_RBG,
	JZIPU_OUT_OFT_GBR,
	JZIPU_OUT_OFT_GRB,
	JZIPU_OUT_OFT_BRG,
	JZIPU_OUT_OFT_BGR,
};

struct jzipu_config {
	struct {
		/* NOTE: only those fields with comments can be seted by user space programs */
		__u32   chip_en:1,
			ipu_run:1,
			hrsz_en:1, /* horizon resize. 1: enable, 0: disable */
			vrsz_en:1, /* vertical resize. 1: enable, 0: disable */
			csc_en:1,  /* CSC enable. 1: enable, 0: disable  */
			irq_en:1,
			ipu_rst:1,
			reserved1:1,
			h_scale:1, /* horizon scale. 1: up scaling, 0: down scaling */
			v_scale:1, /* vertical scale. 1: up scaling, 0: down scaling */
			spkg_sel:1, /* 0: seperated YUV frame
				       1: packaged YUV422 */
			lcdc_sel:1, /* output data destination
				       0: output to external memory(normal memery or framebuffer
				       1: output to LCDC FIFO*/
			spage_map:1,
			dpage_map:1,
			disp_sel:1,
			field_conf_en:1,
			field_sel:1,
			dfix_sel:1,
			reserved2:14;
	} ctrl;
	struct {
		__u16	reconfig; /* 1: need reconfig input registers, else not */
		__u8	fmt;
		__u8	offset;

		__u16	width;
		__u16	height;

		__u8	*y_addr;
		__u8	*u_addr;
		__u8	*v_addr;
		__u16	y_stride;
		__u16	u_stride;
		__u16	v_stride;
		__u16	reserved;
	} input;

	struct {
		int	reconfig;

		int	width_lut_size;
		int	height_lut_size;
		unsigned int	width_lut_coef [IPU_LUT_LEN];
		unsigned int	height_lut_coef [IPU_LUT_LEN];
	} resize;

	struct {
		int	reconfig;

		__u8	yuv_pkg_out_offset;
		__u8	fmt;
		__u8	rgb_offset;
		__u8	rgb888_fmt; /* 0: low 24bits, 1: high 24bits */

		__u16	width;
		__u16	height;
		__u32	stride;

		__u8	*mem_addr;
	} output;
};

#endif /* __JZ4750_IPU_H__ */

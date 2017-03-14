#define __LINUX__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fcntl.h"
#include "unistd.h"
#include "sys/ioctl.h"
#include <linux/fb.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <asm/ioctls.h>

#define USE_LCD
//#define USE_FB
//#define USE_IPU_DEV

#include "jz4750_ipu_regops.h"
//#include "jz4750_lcd.h"
//struct jz4750lcd_info jzlcd_info;
#include "jz47_iputype.h"
#include "jz4750_ipu.h"

static int IMG_WIDTH2 = 480;
static int IMG_HEIGHT2 = 272;
//#define IMG_WIDTH2 480
//#define IMG_HEIGHT2 272
#define RGB_SIZE2 (IMG_WIDTH2 * IMG_HEIGHT2 * 3)

/* ================================================================================ */
int xpos, ypos;
extern int jz47_calc_resize_para ();
extern void jz47_free_alloc_mem();
/* ================================================================================ */
#define IPU_OUT_FB        0
#define IPU_OUT_LCD       1
#define IPU_OUT_PAL_TV    2
#define IPU_OUT_NTSC_TV   3
#define IPU_OUT_MEM       8

#define DEBUG_LEVEL  1  /* 1 dump resize message, 2 dump register for every frame.  */
/* ================================================================================ */

int scale_outW=-1;
int scale_outH=-1;

#define CPU_TYPE 4750
#ifndef USE_LCD
#define OUTPUT_MODE IPU_OUT_MEM
#else
#define OUTPUT_MODE IPU_OUT_FB
#endif


static int ipufd = -1;

/* ipu virtual address.   */
volatile unsigned char *ipu_vbase=NULL;

/* struct IPU module to recorde some info */
struct JZ47_IPU_MOD jz47_ipu_module = {
	.output_mode = OUTPUT_MODE, /* Use the frame for the default */
};

/* Flag to indicate the module init status */
int jz47_ipu_module_init = 0;

/* CPU type: 4740, 4750, 4755, 4760 */
int jz47_cpu_type = CPU_TYPE;

/* flush the dcache.  */
unsigned int dcache[4096];

unsigned char *jz47_ipu_input_mem_ptr = NULL;
unsigned char *jz47_ipu_output_mem_ptr = NULL;
unsigned int output_mem_ptr_phy = 0;
static int ipu_fbfd = 0;

int ipu_image_completed = 0;

/* ================================================================================ */

static char *ipu_regs_name[] = {
	"REG_CTRL",         /* 0x0 */
	"REG_STATUS",       /* 0x4 */
	"REG_D_FMT",        /* 0x8 */
	"REG_Y_ADDR",       /* 0xc */
	"REG_U_ADDR",       /* 0x10 */
	"REG_V_ADDR",       /* 0x14 */
	"REG_IN_FM_GS",     /* 0x18 */
	"REG_Y_STRIDE",     /* 0x1c */
	"REG_UV_STRIDE",    /* 0x20 */
	"REG_OUT_ADDR",     /* 0x24 */
	"REG_OUT_GS",       /* 0x28 */
	"REG_OUT_STRIDE",   /* 0x2c */
	"RSZ_COEF_INDEX",   /* 0x30 */
	"REG_CSC_C0_COEF",  /* 0x34 */
	"REG_CSC_C1_COEF",  /* 0x38 */
	"REG_CSC_C2_COEF",  /* 0x3c */
	"REG_CSC_C3_COEF",  /* 0x40 */
	"REG_CSC_C4_COEF",  /* 0x44 */
	"REG_H_LUT",        /* 0x48 */
	"REG_V_LUT",        /* 0x4c */
	"REG_CSC_OFFPARA",  /* 0x50 */
};

static int jz47_dump_ipu_regs(int num)
{
	int i, total;
	if (num >= 0)
	{
		printf ("ipu_reg: %s: 0x%x\n", ipu_regs_name[num >> 2], REG32(ipu_vbase + num));
		return 1;
	}
	if (num == -1)
	{
		total = sizeof (ipu_regs_name) / sizeof (char *);
		for (i = 0; i < total; i++)
			printf ("ipu_reg: %s: 0x%x\n", ipu_regs_name[i], REG32(ipu_vbase + (i << 2)));
	}
	return 1;
}

static int init_lcd_ctrl_fbfd ()
{
	/* Get lcd control info and do some setting.  */
	if (!ipu_fbfd)
		ipu_fbfd = open ("/dev/fb0", O_RDWR);

	if (!ipu_fbfd)
	{
		printf ("%s:%d +++ ERR: can not open /dev/fb0 ++++\n", __FILE__, __LINE__);
		return 0;
	}
	return 1;
}

static void deinit_lcd_ctrl_fbfd ()
{
	/* Get lcd control info and do some setting.  */
	if (ipu_fbfd)
		close (ipu_fbfd);

	ipu_fbfd = 0;
}

static int jz47_get_output_panel_info (void) /* USER MODE */
{
	struct fb_var_screeninfo fbvar;
	struct fb_fix_screeninfo fbfix;
	int output_mode = jz47_ipu_module.output_mode;

	switch (output_mode)
	{
	case IPU_OUT_MEM:
#if 1
		jz47_ipu_module.out_panel.w = 480;
		jz47_ipu_module.out_panel.h = 272;
		jz47_ipu_module.out_panel.bpp_byte = 4;
		jz47_ipu_module.out_panel.bytes_per_line = 480 * 4;
#else
		jz47_ipu_module.out_panel.w = IMG_WIDTH2;
		jz47_ipu_module.out_panel.h = IMG_HEIGHT2;
		jz47_ipu_module.out_panel.bpp_byte = 4;
		jz47_ipu_module.out_panel.bytes_per_line = IMG_WIDTH2 * 4;
#endif
#ifdef USE_FB
		jz47_ipu_module.out_panel.output_phy = output_mem_ptr_phy;
#else
		jz47_ipu_module.out_panel.output_phy = get_phy_addr ((unsigned int)jz47_ipu_output_mem_ptr);
#endif
		break;

	case IPU_OUT_FB:
	default:
		/* open the frame buffer */
		if (! init_lcd_ctrl_fbfd ())
			return 0;

		/* get the frame buffer info */
		ioctl (ipu_fbfd, FBIOGET_VSCREENINFO, &fbvar);
		ioctl (ipu_fbfd, FBIOGET_FSCREENINFO, &fbfix);

		/* set the output panel info */
		jz47_ipu_module.out_panel.w = fbvar.xres;
		jz47_ipu_module.out_panel.h = fbvar.yres;
		jz47_ipu_module.out_panel.bytes_per_line = fbfix.line_length;
		jz47_ipu_module.out_panel.bpp_byte = fbfix.line_length / fbvar.xres;
		jz47_ipu_module.out_panel.output_phy = fbfix.smem_start;

		break;
	}
	return 1;
}

/* ================================================================================ */
/*
  x = -1, y = -1 is center display
  w = -1, h = -1 is orignal w,h
  w = -2, h = -2 is auto fit
  other: specify  by user
*/

static int jz47_calc_ipu_outsize_and_position (int x, int y, int w, int h)
{
	int dispscr_w, dispscr_h;
	int orignal_w = jz47_ipu_module.srcW;
	int orignal_h = jz47_ipu_module.srcH;

	/* record the orignal setting */
	jz47_ipu_module.out_x = x;
	jz47_ipu_module.out_y = y;
	jz47_ipu_module.out_w = w;
	jz47_ipu_module.out_h = h;

	// The MAX display area which can be used by ipu
	dispscr_w = (x < 0) ? jz47_ipu_module.out_panel.w : (jz47_ipu_module.out_panel.w - x);
	dispscr_h = (y < 0) ? jz47_ipu_module.out_panel.h : (jz47_ipu_module.out_panel.h - y);

	// Orignal size playing or auto fit screen playing mode
	if ((w == -1 && h == -1 && (orignal_w > dispscr_w ||  orignal_h > dispscr_h)) || (w == -2 || h == -2))
	{
		float scale_h = (float)orignal_h / dispscr_h;
		float scale_w = (float)orignal_w / dispscr_w;
		if (scale_w > scale_h)
		{
			w = dispscr_w;
			h = (dispscr_w * orignal_h) / orignal_w;
		}
		else
		{
			h = dispscr_h;
			w = (dispscr_h * orignal_w) / orignal_h;
		}
	}

	// w,h is orignal w,h
	w = (w == -1)? orignal_w : w;
	h = (h == -1)? orignal_h : h;

	// w,h must < dispscr_w,dispscr_h
	w = (w > dispscr_w)? dispscr_w : w;
	h = (h > dispscr_h)? dispscr_h : h;

	// w,h must <= 2*(orignal_w, orignal_h)
	w = (w > 2 * orignal_w) ? (2 * orignal_w) : w;
	h = (h > 2 * orignal_h) ? (2 * orignal_h) : h;

	// calc output position out_x, out_y
	jz47_ipu_module.act_x = (x == -1) ? ((jz47_ipu_module.out_panel.w - w) / 2) : x;
	jz47_ipu_module.act_y = (y == -1) ? ((jz47_ipu_module.out_panel.h - h) / 2) : y;

	// set the resize_w, resize_h
	jz47_ipu_module.act_w = w;
	jz47_ipu_module.act_h = h;

	jz47_ipu_module.need_config_resize = 1;
	jz47_ipu_module.need_config_outputpara = 1;
	return 1;
}

/* ================================================================================ */
#ifndef USE_IPU_DEV
static void jz47_config_ipu_input_para (unsigned char *y, unsigned char *u, unsigned char *v)
{
	unsigned int in_fmt;
	unsigned int srcFormat = PIX_FMT_YUV420P;

	in_fmt = INFMT_YCbCr420; // default value
	if (jz47_ipu_module.need_config_inputpara)
	{
		/* Set input Data format.  */
		switch (srcFormat)
		{
		case PIX_FMT_YUV420P:
			/* 			in_fmt = INFMT_YCbCr420; */
			in_fmt = INFMT_YUV420;
			break;

		case PIX_FMT_YUV422P:
			in_fmt = INFMT_YCbCr422;
			break;

		case PIX_FMT_YUV444P:
			in_fmt = INFMT_YCbCr444;
			break;

		case PIX_FMT_YUV411P:
			in_fmt = INFMT_YCbCr411;
			break;
		}
		REG32 (ipu_vbase + REG_D_FMT) &= ~(IN_FMT_MASK);
		REG32 (ipu_vbase + REG_D_FMT) |= in_fmt;

		/* Set input width and height.  */
		REG32(ipu_vbase + REG_IN_FM_GS) = IN_FM_W(jz47_ipu_module.srcW) | IN_FM_H (jz47_ipu_module.srcH & ~1);

		/* Set the CSC COEF */
		REG32(ipu_vbase + REG_CSC_C0_COEF) = YUV_CSC_C0;
		REG32(ipu_vbase + REG_CSC_C1_COEF) = YUV_CSC_C1;
		REG32(ipu_vbase + REG_CSC_C2_COEF) = YUV_CSC_C2;
		REG32(ipu_vbase + REG_CSC_C3_COEF) = YUV_CSC_C3;
		REG32(ipu_vbase + REG_CSC_C4_COEF) = YUV_CSC_C4;

		REG32(ipu_vbase + REG_CSC_OFFPARA) = YUV_CSC_OFFPARA;

		/* FIXME: Configure the stride for YUV.  */
		//REG32(ipu_vbase + REG_Y_STRIDE) = mpi->stride[0];
		//REG32(ipu_vbase + REG_UV_STRIDE) = U_STRIDE(mpi->stride[1]) | V_STRIDE(mpi->stride[2]);

		REG32(ipu_vbase + REG_Y_STRIDE) = IMG_WIDTH2;
		REG32(ipu_vbase + REG_UV_STRIDE) = U_STRIDE(IMG_WIDTH2 / 2) | V_STRIDE(IMG_WIDTH2 / 2);
	}

	/* Set the YUV addr.  */
	REG32(ipu_vbase + REG_Y_ADDR) = get_phy_addr ((unsigned int)y);
	REG32(ipu_vbase + REG_U_ADDR) = get_phy_addr ((unsigned int)u);
	REG32(ipu_vbase + REG_V_ADDR) = get_phy_addr ((unsigned int)v);

}

/* ================================================================================ */
static void jz47_config_ipu_resize_para ()
{
	int i, width_resize_enable, height_resize_enable;
	int width_up, height_up, width_lut_size, height_lut_size;

	width_resize_enable  = jz47_ipu_module.resize_para.width_resize_enable;
	height_resize_enable = jz47_ipu_module.resize_para.height_resize_enable;
	width_up  =  jz47_ipu_module.resize_para.width_up;
	height_up =  jz47_ipu_module.resize_para.height_up;
	width_lut_size  = jz47_ipu_module.resize_para.width_lut_size;
	height_lut_size = jz47_ipu_module.resize_para.height_lut_size;

	/* Enable the rsize configure.  */
	disable_rsize (ipu_vbase);

	if (width_resize_enable)
		enable_hrsize (ipu_vbase);

	if (height_resize_enable)
		enable_vrsize (ipu_vbase);

	/* Enable the scale configure.  */
	REG32 (ipu_vbase + REG_CTRL) &= ~((1 << V_SCALE_SHIFT) | (1 << H_SCALE_SHIFT));
	REG32 (ipu_vbase + REG_CTRL) |= (height_up << V_SCALE_SHIFT) | (width_up << H_SCALE_SHIFT);

	/* Set the LUT index.  */
	REG32 (ipu_vbase + REG_RSZ_COEF_INDEX) = (((height_lut_size - 1) << VE_IDX_SFT)
						  | ((width_lut_size - 1) << HE_IDX_SFT));

	/* set lut. */
	if (jz47_cpu_type == 4740)
	{
		if (height_resize_enable)
		{
			for (i = 0; i < height_lut_size; i++)
				REG32 (ipu_vbase + VRSZ_LUT_BASE + i * 4) = jz47_ipu_module.resize_para.height_lut_coef[i];
		}
		else
			REG32 (ipu_vbase + VRSZ_LUT_BASE) = ((128 << 2)| 0x3);

		if (width_resize_enable)
		{
			for (i = 0; i < width_lut_size; i++)
				REG32 (ipu_vbase + HRSZ_LUT_BASE + i * 4) = jz47_ipu_module.resize_para.width_lut_coef[i];
		}
		else
			REG32 (ipu_vbase + HRSZ_LUT_BASE) = ((128 << 2)| 0x3);
	}
	else
	{
		REG32 (ipu_vbase + VRSZ_LUT_BASE) = (1 << START_N_SFT);
		for (i = 0; i < height_lut_size; i++)
			REG32 (ipu_vbase + VRSZ_LUT_BASE) = jz47_ipu_module.resize_para.height_lut_coef[i];

		REG32 (ipu_vbase + HRSZ_LUT_BASE) = (1 << START_N_SFT);
		for (i = 0; i < width_lut_size; i++)
			REG32 (ipu_vbase + HRSZ_LUT_BASE) = jz47_ipu_module.resize_para.width_lut_coef[i];
	}

	/* dump the resize messages.  */
	if (DEBUG_LEVEL > 0)
	{
		printf ("panel_w = %d, panel_h = %d, srcW = %d, srcH = %d\n",
			jz47_ipu_module.out_panel.w, jz47_ipu_module.out_panel.h,
			jz47_ipu_module.srcW, jz47_ipu_module.srcH);
		printf ("out_x = %d, out_y = %d, out_w = %d, out_h = %d\n",
			jz47_ipu_module.out_x, jz47_ipu_module.out_y,
			jz47_ipu_module.out_w, jz47_ipu_module.out_h);
		printf ("act_x = %d, act_y = %d, act_w = %d, act_h = %d, outW = %d, outH = %d\n",
			jz47_ipu_module.act_x, jz47_ipu_module.act_y,
			jz47_ipu_module.act_w, jz47_ipu_module.act_h,
			jz47_ipu_module.resize_para.outW, jz47_ipu_module.resize_para.outH);
	}

}

/* ================================================================================ */

#define PIX_FMT_RGBA32 0
static void  jz47_config_ipu_output_para ()
{
	int frame_offset, out_x, out_y;
	int rsize_w, rsize_h, outW, outH;
	int output_mode = jz47_ipu_module.output_mode;
	unsigned int out_fmt, dstFormat = PIX_FMT_BGR565;

	/* Get the output parameter from struct.  */
	outW = jz47_ipu_module.resize_para.outW;
	outH = jz47_ipu_module.resize_para.outH;
	out_x = jz47_ipu_module.act_x;
	out_y = jz47_ipu_module.act_y;
	rsize_w = jz47_ipu_module.act_w;
	rsize_h = jz47_ipu_module.act_h;

	/* outW must < resize_w and outH must < resize_h.  */
	outW = (outW <= rsize_w) ? outW : rsize_w;
	outH = (outH <= rsize_h) ? outH : rsize_h;

	/* calc the offset for output.  */
	frame_offset = (out_x + out_y * jz47_ipu_module.out_panel.w) * jz47_ipu_module.out_panel.bpp_byte;
	out_fmt = OUTFMT_RGB565;  // default value

	printf("outx = %d, out_y = %d, outW = %d, outH = %d frame_offset = %d\n",
	       out_x, out_y, outW, outH, frame_offset);

	/* clear some output control bits.  */
	disable_ipu_direct(ipu_vbase);

	switch (dstFormat)
	{
	case PIX_FMT_RGBA32:
		out_fmt = OUTFMT_RGB888;
		outW = outW << 2;
		break;

	case PIX_FMT_RGB555:
		out_fmt = OUTFMT_RGB555;
		outW = outW << 1;
		break;

	case PIX_FMT_RGB565:
	case PIX_FMT_BGR565:
		out_fmt = OUTFMT_RGB565;
		outW = outW << 1;
		break;
	}

	/* clear the OUT_DATA_FMT control bits.  */
	REG32 (ipu_vbase + REG_D_FMT) &= ~(OUT_FMT_MASK);

	switch (output_mode)
	{
	case IPU_OUT_FB:
	default:

		//		set_lcd_fg0();

		REG32(ipu_vbase + REG_OUT_ADDR) = jz47_ipu_module.out_panel.output_phy + frame_offset;
		REG32(ipu_vbase + REG_OUT_STRIDE) = jz47_ipu_module.out_panel.bytes_per_line;
		break;
	}

	REG32(ipu_vbase + REG_OUT_GS) = OUT_FM_W (outW) | OUT_FM_H (outH);
	REG32 (ipu_vbase + REG_D_FMT) |= out_fmt;
	REG32 (ipu_vbase + REG_CTRL) |= CSC_EN;
}


/* mode class A:  OUT_FB, OUT_MEM
   mode class B:  OUT_LCD
*/
/* ================================================================================ */
static int jz47_config_stop_ipu ()
{
	int switch_mode, runing_mode;

	/* Get the runing mode class.  */
	if (jz47_cpu_type == 4740
	    || jz47_ipu_module.ipu_working_mode == IPU_OUT_FB
	    || jz47_ipu_module.ipu_working_mode == IPU_OUT_MEM)
		runing_mode = 'A';
	else
		runing_mode = 'B';

	/* Get the switch mode class.  */
	if (jz47_cpu_type == 4740
	    || jz47_ipu_module.output_mode == IPU_OUT_FB
	    || jz47_ipu_module.output_mode == IPU_OUT_MEM)
		switch_mode = 'A';
	else
		switch_mode = 'B';

	/* Base on the runing_mode and switch_mode, disable lcd or stop ipu.  */
	if (runing_mode == 'A' && switch_mode == 'A')
	{
		printf("AA...\n");
		while (ipu_is_enable(ipu_vbase) && (!polling_end_flag(ipu_vbase))) {
			printf("polling......\n");
		}
		stop_ipu(ipu_vbase);
		clear_end_flag(ipu_vbase);
	}
	return 1;
}


/* ================================================================================ */

static int  jz47_config_run_ipu ()
{
	int output_mode = jz47_ipu_module.output_mode;

	/* set the ipu working mode.  */
	jz47_ipu_module.ipu_working_mode = output_mode;

	/* run ipu for different output mode.  */
	switch (output_mode)
	{
	case IPU_OUT_FB:
	case IPU_OUT_MEM:
	default:
		run_ipu (ipu_vbase);
		break;
	}

	return 1;
}
#else  /* !USE_IPU_DEV */
static struct jzipu_config config;
static void jz47_config_ipu_input_para (unsigned char *y, unsigned char *u, unsigned char *v) {
	unsigned int in_fmt;
	unsigned int srcFormat = PIX_FMT_YUV420P;

	in_fmt = JZIPU_IN_FMT_YUV420; // default value
	if (jz47_ipu_module.need_config_inputpara)
	{
		config.input.reconfig = 1;

		/* Set input Data format.  */
		switch (srcFormat)
		{
		case PIX_FMT_YUV420P:
			/* 			in_fmt = INFMT_YCbCr420; */
			in_fmt = JZIPU_IN_FMT_YUV420;
			break;

		case PIX_FMT_YUV422P:
			in_fmt = JZIPU_IN_FMT_YUV422;
			break;

		case PIX_FMT_YUV444P:
			in_fmt = JZIPU_IN_FMT_YUV444;
			break;

		case PIX_FMT_YUV411P:
			in_fmt = JZIPU_IN_FMT_YUV411;
			break;
		}

		config.input.fmt = in_fmt;

		/* Set input width and height.  */
		config.input.width = jz47_ipu_module.srcW;
		config.input.height = jz47_ipu_module.srcH;

		config.input.y_stride = IMG_WIDTH2;
		config.input.u_stride = IMG_WIDTH2 / 2;
		config.input.v_stride = IMG_WIDTH2 / 2;
	}

	/* Set the YUV addr.  */
	config.input.y_addr = (unsigned char *)get_phy_addr ((unsigned int)y);
	config.input.u_addr = (unsigned char *)get_phy_addr ((unsigned int)u);
	config.input.v_addr = (unsigned char *)get_phy_addr ((unsigned int)v);
}
static void jz47_config_ipu_resize_para ()
{
	int i, width_resize_enable, height_resize_enable;
	int width_up, height_up, width_lut_size, height_lut_size;

	config.resize.reconfig = 1;

	width_resize_enable  = jz47_ipu_module.resize_para.width_resize_enable;
	height_resize_enable = jz47_ipu_module.resize_para.height_resize_enable;
	width_up  =  jz47_ipu_module.resize_para.width_up;
	height_up =  jz47_ipu_module.resize_para.height_up;
	width_lut_size  = jz47_ipu_module.resize_para.width_lut_size;
	height_lut_size = jz47_ipu_module.resize_para.height_lut_size;

	/* Enable the scale configure.  */
	config.ctrl.v_scale = height_up;
	config.ctrl.h_scale = width_up;

	/* Set the LUT index.  */
	config.resize.width_lut_size = width_lut_size;
	config.resize.height_lut_size = height_lut_size;

	for (i = 0; i < height_lut_size; i++)
		config.resize.height_lut_coef[i] = jz47_ipu_module.resize_para.height_lut_coef[i];

	for (i = 0; i < width_lut_size; i++)
		config.resize.width_lut_coef[i] = jz47_ipu_module.resize_para.width_lut_coef[i];

	/* dump the resize messages.  */
	if (DEBUG_LEVEL > 0)
	{
		printf ("panel_w = %d, panel_h = %d, srcW = %d, srcH = %d\n",
			jz47_ipu_module.out_panel.w, jz47_ipu_module.out_panel.h,
			jz47_ipu_module.srcW, jz47_ipu_module.srcH);
		printf ("out_x = %d, out_y = %d, out_w = %d, out_h = %d\n",
			jz47_ipu_module.out_x, jz47_ipu_module.out_y,
			jz47_ipu_module.out_w, jz47_ipu_module.out_h);
		printf ("act_x = %d, act_y = %d, act_w = %d, act_h = %d, outW = %d, outH = %d\n",
			jz47_ipu_module.act_x, jz47_ipu_module.act_y,
			jz47_ipu_module.act_w, jz47_ipu_module.act_h,
			jz47_ipu_module.resize_para.outW, jz47_ipu_module.resize_para.outH);
	}
}

#define PIX_FMT_RGBA32 0
static void  jz47_config_ipu_output_para ()
{
		int frame_offset, out_x, out_y;
	int rsize_w, rsize_h, outW, outH;
	int output_mode = jz47_ipu_module.output_mode;
	unsigned int out_fmt, dstFormat = PIX_FMT_BGR565;

	config.output.reconfig = 1;

	/* Get the output parameter from struct.  */
	outW = jz47_ipu_module.resize_para.outW;
	outH = jz47_ipu_module.resize_para.outH;
	out_x = jz47_ipu_module.act_x;
	out_y = jz47_ipu_module.act_y;
	rsize_w = jz47_ipu_module.act_w;
	rsize_h = jz47_ipu_module.act_h;

	/* outW must < resize_w and outH must < resize_h.  */
	outW = (outW <= rsize_w) ? outW : rsize_w;
	outH = (outH <= rsize_h) ? outH : rsize_h;

	/* calc the offset for output.  */
	frame_offset = (out_x + out_y * jz47_ipu_module.out_panel.w) * jz47_ipu_module.out_panel.bpp_byte;
	out_fmt = JZIPU_OUT_FMT_RGB565;  // default value

	printf("outx = %d, out_y = %d, outW = %d, outH = %d frame_offset = %d\n",
	       out_x, out_y, outW, outH, frame_offset);

	/* clear some output control bits.  */
/* 	disable_ipu_direct(ipu_vbase); */

	switch (dstFormat)
	{
	case PIX_FMT_RGBA32:
		out_fmt = JZIPU_OUT_FMT_RGB888;
		outW = outW << 2;
		break;

	case PIX_FMT_RGB555:
		out_fmt = JZIPU_OUT_FMT_RGB555;
		outW = outW << 1;
		break;

	case PIX_FMT_RGB565:
	case PIX_FMT_BGR565:
		out_fmt = JZIPU_OUT_FMT_RGB565;
		outW = outW << 1;
		break;
	}

	switch (output_mode)
	{
	case IPU_OUT_FB:
	default:
		config.output.mem_addr = (unsigned char *)(jz47_ipu_module.out_panel.output_phy + frame_offset);
		config.output.stride = jz47_ipu_module.out_panel.bytes_per_line;
		break;
	}

	config.output.width = outW;
	config.output.height = outH;
	config.output.fmt = out_fmt;
	config.ctrl.csc_en = 1;
}
static int jz47_config_stop_ipu () {
	/* do nothing */
}
static int  jz47_config_run_ipu ()
{
	if (ioctl(ipufd, JZIPU_IO_PUT_IMG, &config) < 0) {
		printf("put image to /dev/jzipu failed!!!\n");
	}

	return 1;
}
#endif /* USE_IPU_DEV */

/* ================================================================================ */
int jz47_put_image_with_ipu (unsigned char *y, unsigned char *u, unsigned char *v, int img_width, int img_height)
{
	IMG_WIDTH2 = img_width;
        IMG_HEIGHT2 = img_height;
	/* Get the output panel information, calc the output position and shapes */
	if (!jz47_ipu_module_init)
	{
		if (! jz47_get_output_panel_info ())
			return 0;
		jz47_ipu_module.srcW = IMG_WIDTH2;
		jz47_ipu_module.srcH = IMG_HEIGHT2;
		jz47_calc_ipu_outsize_and_position (xpos, ypos, scale_outW, scale_outH);

		jz47_ipu_module.need_config_resize = 1;
		jz47_ipu_module.need_config_inputpara = 1;
		jz47_ipu_module.need_config_outputpara = 1;
		jz47_ipu_module_init = 1;
		//		reset_ipu (ipu_vbase);
	}

	/* calculate resize parameter.  */
	if (jz47_ipu_module.need_config_resize)
		jz47_calc_resize_para ();

	/* Following codes is used to configure IPU, so we need stop the ipu.  */
	jz47_config_stop_ipu ();

	/* configure the input parameter.  */
	jz47_config_ipu_input_para (y, u, v);

	/* configure the resize parameter.  */
	if (jz47_ipu_module.need_config_resize)
		jz47_config_ipu_resize_para ();

	/* configure the output parameter.  */
	if (jz47_ipu_module.need_config_outputpara)
		jz47_config_ipu_output_para ();

	if (DEBUG_LEVEL > 1)
		jz47_dump_ipu_regs(-1);


#if 1
	/* run ipu */
	jz47_config_run_ipu ();
#endif

	//	if (DEBUG_LEVEL > 1)
	//		jz47_dump_ipu_regs(-1);

	/* set some flag for normal.  */
	deinit_lcd_ctrl_fbfd ();

	jz47_ipu_module.need_config_resize = 0;
	jz47_ipu_module.need_config_inputpara = 0;
	jz47_ipu_module.need_config_outputpara = 0;
	ipu_image_completed = 1;
	return 1;
}

static int mmapfd = 0;
static int console_fd = -1;
static int original_vt = -1;

static int fbfd = -1;
static int smem_len = 0;

static void fini_console() {
	struct vt_mode vt;

	ioctl(console_fd, KDSETMODE, KD_TEXT);
	ioctl(console_fd, VT_GETMODE, &vt);
	vt.mode = VT_AUTO;
	ioctl(console_fd, VT_SETMODE, &vt);

	if (original_vt >= 0) {
		ioctl(console_fd, VT_ACTIVATE, original_vt);
		original_vt = -1;
	}

	close(console_fd);
}

static void fini_env()
{
	jz47_exit_memalloc(NULL);
	munmap((void *)ipu_vbase, IPU__SIZE);
	close(mmapfd);
#ifdef USE_FB
	munmap((void *)jz47_ipu_output_mem_ptr, smem_len);
	close(fbfd);
#endif

#if defined USE_FB || defined USE_LCD
	fini_console();
#endif

#ifdef USE_IPU_DEV
	close(ipufd);
#endif
}

static unsigned char *y_buff = NULL;
static unsigned char *u_buff = NULL;
static unsigned char *v_buff = NULL;
//static unsigned char *out_buff = NULL;

#define SIGUSR1 10

static void open_console() {
	int ttyfd;
	int vtnumber;
	char ttystr[1000];

	if ((console_fd = open("/dev/tty0", O_WRONLY, 0)) < 0) {
		printf("error<1>\n");
		exit (1);
	}
	if (ioctl(console_fd, VT_OPENQRY, &vtnumber) < 0 || vtnumber < 0) {
		printf("error<2>\n");
		exit (1);
	}
	close(console_fd);
	console_fd = -1;

	sprintf(ttystr, "/dev/tty%d", vtnumber);
	console_fd = open(ttystr, O_RDWR | O_NDELAY, 0);
	if (console_fd < 0) {
		printf("error<3>\n");
		exit (1);
	}

	{
		struct vt_stat vts;
		if (ioctl(console_fd, VT_GETSTATE, &vts) == 0)
			original_vt = vts.v_active;
	}

	ttyfd = open("/dev/tty", O_RDWR);
	if (ttyfd >= 0) {
		ioctl(ttyfd, TIOCNOTTY, 0);
		close(ttyfd);
	} else {
		printf("error<4>\n");
		exit (1);
	}

	{
		struct vt_mode vt;
		if (ioctl(console_fd, VT_ACTIVATE, vtnumber) != 0)
			printf("error<5>\n");
		if (ioctl(console_fd, VT_WAITACTIVE, vtnumber) != 0)
			printf("error<6>\n");
		if (ioctl(console_fd, VT_GETMODE, &vt) < 0) {
			printf("error<7>\n");
			exit (1);
		}

		vt.mode = VT_PROCESS;
		vt.relsig = SIGUSR1;
		vt.acqsig = SIGUSR1;

		if (ioctl(console_fd, VT_SETMODE, &vt) < 0) {
			printf("error<8>\n");
			exit (1);
		}
	}

	if (ioctl(console_fd, KDSETMODE, KD_GRAPHICS) < 0) {
		printf("error<9>\n");
		exit (1);
	}
}

int init_ipu_env()
{
	struct fb_fix_screeninfo FixedInfo;
	struct fb_var_screeninfo OrigVarInfo;

#ifdef USE_IPU_DEV
	printf("open /dev/jzipu......\n");
	if ((ipufd = open("/dev/jzipu", O_RDWR)) < 0) {
		printf("can not open /dev/jzipu\n");
		exit (1);
	}
#endif

	mmapfd = open("/dev/mem", O_RDWR);
	if (mmapfd < 0) {
		printf("error open /dev/mem, ret = %d\n", mmapfd);
		exit (1);
	}

	ipu_vbase = mmap(0, IPU__SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmapfd, IPU__OFFSET);
	printf("ipu_vbase = %p\n", ipu_vbase);

	ipu_image_completed = 0;
	jz47_ipu_module_init = 0;

#if defined USE_FB || defined USE_LCD
	open_console();
#endif

	jz47_ipu_input_mem_ptr = (unsigned char *)jz4740_alloc_frame(32, 2 * 1024 * 1024); /* 2M */
	if (!jz47_ipu_input_mem_ptr)
		goto fail_alloc;
	memset(jz47_ipu_input_mem_ptr, 0, 480 * 272 + 240 * 136 * 2);
	y_buff = jz47_ipu_input_mem_ptr;
	u_buff = y_buff + 480 * 272;
	v_buff = u_buff + 240 * 136;
	printf("input mem ptr = %p\n", jz47_ipu_input_mem_ptr);

#ifndef USE_LCD
#ifdef USE_FB
	fbfd = open("/dev/fb0", O_RDWR);
	if (fbfd < 0) {
		fprintf(stderr, "Error opening /dev/fb0\n");
		exit (1);
	}

	ioctl(fbfd, FBIOGET_FSCREENINFO, &FixedInfo);
	ioctl(fbfd, FBIOGET_VSCREENINFO, &OrigVarInfo);

	if (FixedInfo.visual != FB_VISUAL_TRUECOLOR && FixedInfo.visual != FB_VISUAL_DIRECTCOLOR) {
		fprintf(stderr, "error!!!!\n");
		exit (1);
	}

	printf("smem_len = %d\n", FixedInfo.smem_len);

	jz47_ipu_output_mem_ptr = (void *)mmap(0,
					       FixedInfo.smem_len,
					       PROT_READ | PROT_WRITE,
					       MAP_SHARED,
					       fbfd,
					       0);
	smem_len = FixedInfo.smem_len;
	output_mem_ptr_phy = FixedInfo.smem_start;

	printf("xres = %d, yres = %d bytes_per_line = %d bpp = %d\n",
	       OrigVarInfo.xres, OrigVarInfo.yres,
	       FixedInfo.line_length, OrigVarInfo.bits_per_pixel);
#else
	jz47_ipu_output_mem_ptr = (unsigned char *)jz4740_alloc_frame(32, 2 * 1024 * 1024); /* *2M */
	printf("use normal mem......\n");
#endif
	if (!jz47_ipu_output_mem_ptr)
		goto fail_alloc_out;
	printf("output mem ptr = %p\n", jz47_ipu_output_mem_ptr);
#endif

	return 0;

 fail_alloc_out:
	jz47_exit_memalloc(NULL);
 fail_alloc:
	fini_env();
	exit (1);
	return -1;
}

#ifdef TEST
extern int yuv_decode(unsigned char *y_buff, unsigned char *u_buff, unsigned char *v_buff);

int main() {
	int ret = 0;
	FILE *bmp_file;
	unsigned char *ipu_out_buff = NULL;
	unsigned char *out_buff = NULL;
	int test_count = 10;
	int j = 599;
	int i = 0;
	int start_line = 0;

	unsigned char *yuv_backup = NULL;
	unsigned char *y_buff_b = NULL;
	unsigned char *u_buff_b = NULL;
	unsigned char *v_buff_b = NULL;

	init_ipu_env();

	yuv_backup = malloc(800 * 600 + 400 * 300 * 2 + 4);
	if (!yuv_backup)
		goto out;
	yuv_backup = (unsigned char *)(((unsigned int)yuv_backup + 3) & (~0x3));
	y_buff_b = yuv_backup;
	u_buff_b = y_buff_b + 800 * 600;
	v_buff_b = u_buff_b + 400 * 300;
	printf("yuv_backup = %p\n", yuv_backup);


	yuv_decode(y_buff_b, u_buff_b, v_buff_b);

	printf("put image with ipu......\n");

	for (start_line = 0; start_line < (600 - 272); start_line += 50) {
		for (j = start_line, i = 0; j < 272 + start_line; j++, i++) {
			memcpy(y_buff + i * 480, y_buff_b + j * 800, 480);
		}
		for (j = start_line / 2, i = 0; j < (272 + start_line) / 2; j++, i++) {
			memcpy(u_buff + i * 240, u_buff_b + j * 400, 240);
			memcpy(v_buff + i * 240, v_buff_b + j * 400, 240);
		}
		jz47_put_image_with_ipu (y_buff, u_buff, v_buff, 480, 272);
		sleep(3);
	}

#if 0
	out_buff = malloc(800 * 600 * 3);
	memset((void *)out_buff, 128, 800 * 600 * 3);
	ipu_out_buff = jz47_ipu_output_mem_ptr;
	for (j = 271; j >= 0; j--) {
		for (i = 0; i < 480 * 3; i+=3) {
			if (test_count -- > 0) {
				printf("%02x %02x %02x %02x\n",
				       *ipu_out_buff, *(ipu_out_buff + 1),
				       *(ipu_out_buff + 2), *(ipu_out_buff + 3));
			}
			/* little endian */
			*(out_buff + j * 800 * 3 + i) = *ipu_out_buff; /* B */
			*(out_buff + j * 800 * 3 + i + 1) = *(ipu_out_buff + 1); /* G */
			*(out_buff + j * 800 * 3 + i + 2) = *(ipu_out_buff + 2); /* R */
			ipu_out_buff += 4;
		}
	}

	bmp_file = fopen("test.bmp", "a");
	if (!bmp_file)
		printf("open test.bmp failed!!!\n");
	ret = fwrite(out_buff, sizeof(unsigned char), 800 * 600 * 3, bmp_file);
	printf("fwrite to bmp_file ret = %d\n", ret);

	fclose(bmp_file);
	free(out_buff);
#else
	sleep(5);
#endif

 out:
	fini_env();

	return 0;
}
#endif /* TEST */

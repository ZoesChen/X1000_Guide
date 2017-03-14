#ifndef __PREVIEW_DISPLAY_H__
#define __PREVIEW_DISPLAY_H__

/*
 * Info for display on LCD panel.
 *
 * size: 16 * 4 = 64 Bytes
 *
 */
typedef unsigned int	__u32;
typedef signed int	__s32;
typedef unsigned short	__u16;
typedef signed short	__s16;
typedef unsigned char	__u8;
typedef signed char	__s8;

enum options {
	OPS_PREVIEW		= 0,
	OPS_SAVE_BMP		= 1,
	OPS_SAVE_JPG		= 2,
	OPS_SAVE_RAW		= 3,
	OPS_DISPLAY_RAW		= 4,
};

struct display_info
{
	// format & resolution
	struct {
		__u32 fourcc;	// compatible with V4L2
		__u32 format;	// local definition, more detail
		__u32 fmt_priv;	// CIMCFG.PACK & CIMCFG.SEP && CIMCR.MBEN
	} fmt;
	__u32 width;
	__u32 height;
	__u32 bpp;

	__u32 xoff;
	__u32 yoff;

	enum options ops;

	__u8 *ybuf;
	__u8 *ubuf;
	__u8 *vbuf;
	__u32 n_bytes;

	__u32	only_y	: 1,	//only Y(ingnore UV)
		window	: 1,	//window cut or not
		ipu	: 1,	//use IPU or not
		planar	: 1,	//planar
		mb	: 1,	//macro block for yuv420
		userptr	: 1,	//use internal TLB (using user space virtual memory instead of kernel space physical memory)
		reserved: 26;
	__u32 padding[2];
};

#define PRINT_DISPLAY_INFO(d)						\
	do {								\
		cim_dbg(">>>>>display_info>>>>>\n");			\
		cim_dbg("inW = %d\n", (d)->inW);			\
		cim_dbg("inH = %d\n", (d)->inH);			\
		cim_dbg("outW = %d\n", (d)->outW);			\
		cim_dbg("outH = %d\n", (d)->outH);			\
		cim_dbg("prev_x_off = %d\n", (d)->prev_x_off);		\
		cim_dbg("prev_y_off = %d\n", (d)->prev_y_off);		\
		cim_dbg("lcd_x_off = %d\n", (d)->lcd_x_off);		\
		cim_dbg("lcd_y_off = %d\n", (d)->lcd_y_off);		\
		cim_dbg("fmt = 0x%08x\n", (d)->fmt);			\
		cim_dbg("window enable = %d\n", (d)->window_enable);	\
		cim_dbg("<<<<<<<end<<<<<<<<<<<<\n");				\
	} while(0)

#endif /* __PREVIEW_DISPLAY_H__ */

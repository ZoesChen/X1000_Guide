#ifndef __CIM_FMT_H__
#define __CIM_FMT_H__

#include <linux/videodev2.h>

#define V4L2_PIX_FMT_YVYU    v4l2_fourcc('Y','V','Y','U') /* 16  YUV 4:2:2     */
#define V4L2_PIX_FMT_VYUY    v4l2_fourcc('V','Y','U','Y') /* 16  YUV 4:2:2     */

#if 0

#define DF_IS_RGB888(f)		( (f)->fmt.pix.pixelformat == V4L2_PIX_FMT_RGB24 )
#define DF_IS_RGB565(f)		( (f)->fmt.pix.pixelformat == V4L2_PIX_FMT_RGB565 )

#define DF_IS_YCbCr444(f)	( (f)->fmt.pix.pixelformat == V4L2_PIX_FMT_YUV444 )

#define DF_IS_YCbCr422(f)					\
	   ( (f)->fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV )	\
	|| ( (f)->fmt.pix.pixelformat == V4L2_PIX_FMT_UYVY )	\
	|| ( (f)->fmt.pix.pixelformat == V4L2_PIX_FMT_YVYU )	\
	|| ( (f)->fmt.pix.pixelformat == V4L2_PIX_FMT_YVYU )

#define DF_IS_YCbCr420(f)					\
	   ( (f)->fmt.pix.pixelformat == V4L2_PIX_FMT_YUV420 )	\
	|| ( (f)->fmt.pix.pixelformat == V4L2_PIX_FMT_NV12 )	\
	|| ( (f)->fmt.pix.pixelformat == V4L2_PIX_FMT_NV21 )	\
	|| ( (f)->fmt.pix.pixelformat == V4L2_PIX_FMT_HM12 )

#else

#define DF_IS_RGB888(fourcc)		( (fourcc) == V4L2_PIX_FMT_RGB24 )
#define DF_IS_RGB565(fourcc)		( (fourcc) == V4L2_PIX_FMT_RGB565 )

#define DF_IS_YCbCr444(fourcc)		( (fourcc) == V4L2_PIX_FMT_YUV444 )

#define DF_IS_YCbCr422(fourcc)			\
	   ( (fourcc) == V4L2_PIX_FMT_YUYV )	\
	|| ( (fourcc) == V4L2_PIX_FMT_UYVY )	\
	|| ( (fourcc) == V4L2_PIX_FMT_YVYU )	\
	|| ( (fourcc) == V4L2_PIX_FMT_YVYU )	\
	|| ( (fourcc) == V4L2_PIX_FMT_YUV422P )

#define DF_IS_YCbCr420(fourcc)			\
	   ( (fourcc) == V4L2_PIX_FMT_YUV420 )	\
	|| ( (fourcc) == V4L2_PIX_FMT_NV12 )	\
	|| ( (fourcc) == V4L2_PIX_FMT_NV21 )	\
	|| ( (fourcc) == V4L2_PIX_FMT_HM12 )



#endif

//from cimtest5 cim_fmt.h

/* fmt[31:28]: category
 * fmt[27:24]: reserved
 * fmt[23:16]: YCbCr444 orders or RGB orders or bayer orders
 * fmt[15:0]:  YCbCr422 orders
 * Note: Y,Cb,Cr also reference to Y,U,V
 */
#define SUPPORT_YUV420
#define SUPPORT_VA

#undef SUPPORT_YUV420
#undef SUPPORT_VA

#define DISP_FMT_YCbCr444	(1 << 31)
#define DISP_FMT_YCbCr422	(1 << 30)
#define DISP_FMT_YCbCr420	(1 << 29)
#define DISP_FMT_RGB888		(1 << 28)
#define DISP_FMT_RGB565		(1 << 27)
#define DISP_FMT_BAYER		(1 << 26)
#define DISP_FMT_ITU656P	(1 << 24)
#define DISP_FMT_ITU656I	(1 << 23)
#define DISP_FMT_OTHER		(1 << 22)
/*
#define DF_IS_YCbCr444(v) ( (v) & DISP_FMT_YCbCr444)
#define DF_IS_YCbCr422(v) ( (v) & DISP_FMT_YCbCr422)
#define DF_IS_YCbCr420(v) ( (v) & DISP_FMT_YCbCr420)
#define DF_IS_RGB888(v) ( (v) & DISP_FMT_RGB888)
#define DF_IS_RGB565(v) ( (v) & DISP_FMT_RGB565)
#define DF_IS_BAYER(v) ( (v) & DISP_FMT_BAYER)
#define DF_IS_ITU656P(v) ( (v) & DISP_FMT_ITU656P)
#define DF_IS_ITU656I(v) ( (v) & DISP_FMT_ITU656I)
*/
#define YCbCr422_MASK		(DISP_FMT_YCbCr422 | 0xFFFF)
#define YCbCr422_Y0CbY1Cr	(DISP_FMT_YCbCr422 | 0x0000)
#define YCbCr422_CbY1CrY0	(DISP_FMT_YCbCr422 | 0x0001)
#define YCbCr422_Y1CrY0Cb	(DISP_FMT_YCbCr422 | 0x0002)
#define YCbCr422_CrY0CbY1	(DISP_FMT_YCbCr422 | 0x0004)
#define YCbCr422_CrY1CbY0	(DISP_FMT_YCbCr422 | 0x0008)
#define YCbCr422_Y1CbY0Cr	(DISP_FMT_YCbCr422 | 0x0010)
#define YCbCr422_CbY0CrY1	(DISP_FMT_YCbCr422 | 0x0020)
#define YCbCr422_Y0CrY1Cb	(DISP_FMT_YCbCr422 | 0x0040)

#define ITU656P_YCbCr422_CbY0CrY1	(DISP_FMT_ITU656P | DISP_FMT_YCbCr422 | 0x0020)
#define ITU656I_YCbCr422_CbY0CrY1	(DISP_FMT_ITU656I | DISP_FMT_YCbCr422 | 0x0020)

#define YCbCr420_FMT		(DISP_FMT_YCbCr420 | 0x0000)

#define ITU656P_YCbCr420_FMT	(DISP_FMT_ITU656P | DISP_FMT_YCbCr420 | 0x0000)
#define ITU656I_YCbCr420_FMT	(DISP_FMT_ITU656I | DISP_FMT_YCbCr420 | 0x0000)

#define YCbCr444_MASK		(DISP_FMT_YCbCr444 | (0xFF << 16))
#define YCbCr444_YUV		(DISP_FMT_YCbCr444 | (0x00 << 16))
#define YCbCr444_YVU		(DISP_FMT_YCbCr444 | (0x01 << 16))
#define YCbCr444_UYV		(DISP_FMT_YCbCr444 | (0x02 << 16))
#define YCbCr444_VYU		(DISP_FMT_YCbCr444 | (0x04 << 16))
#define YCbCr444_UVY		(DISP_FMT_YCbCr444 | (0x08 << 16))
#define YCbCr444_VUY		(DISP_FMT_YCbCr444 | (0x10 << 16))

#define RAW888_MASK		(DISP_FMT_RGB888 | (0xFF << 16))
#define RAW888_GRB		(DISP_FMT_RGB888 | (0x00 << 16))
#define RAW888_GBR		(DISP_FMT_RGB888 | (0x01 << 16))
#define RAW888_RGB		(DISP_FMT_RGB888 | (0x02 << 16))
#define RAW888_BGR		(DISP_FMT_RGB888 | (0x04 << 16))
#define RAW888_RBG		(DISP_FMT_RGB888 | (0x08 << 16))
#define RAW888_BRG		(DISP_FMT_RGB888 | (0x10 << 16))

#define RAW565_MASK		(DISP_FMT_RGB565 | (0xFF << 16))
#define RAW565_GRB		(DISP_FMT_RGB565 | (0x00 << 16))
#define RAW565_GBR		(DISP_FMT_RGB565 | (0x01 << 16))
#define RAW565_RGB		(DISP_FMT_RGB565 | (0x02 << 16))
#define RAW565_BGR		(DISP_FMT_RGB565 | (0x04 << 16))
#define RAW565_RBG		(DISP_FMT_RGB565 | (0x08 << 16))
#define RAW565_BRG		(DISP_FMT_RGB565 | (0x10 << 16))

/*
 * R G R G R G
 * G B G B G B
 * R G R G R G
 * G B G B G B
 * R G R G R G
 * G B G B G B
 *
 * acctually the following format are identical,
 * they are different by the top-right color of the CFA(color filter array)
 */
#define BAYER_MASK		(DISP_FMT_BAYER | (0xFF << 16))

/*
 * G R
 * B R
 */
#define BAYER_GRBG		(DISP_FMT_BAYER | (0x00 << 16))

/*
 * G B
 * R G
 */
#define BAYER_GBRG		(DISP_FMT_BAYER | (0x01 << 16))

/*
 * R G
 * G B
 */
#define BAYER_RGGB		(DISP_FMT_BAYER | (0x02 << 16))

/*
 * B G
 * G R
 */
#define BAYER_BGGR		(DISP_FMT_BAYER | (0x04 << 16))


extern __u32 str_to_fmt(char *str);

#endif /* __CIM_FMT_H__ */

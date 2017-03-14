#include "headers.h"

#if 0
void calc_display_info(int inW, int inH, struct display_info *display_info,
		       const struct fb_info *fb_info, const struct global_info *ginfo)
{
	if (fb_info->xres > inW) {
		display_info->outW = inW;
		display_info->prev_x_off = 0;
		display_info->lcd_x_off = ((fb_info->xres - inW) / 2) & (~0x1);
	} else {
		display_info->lcd_x_off = 0;
		display_info->prev_x_off = ((inW - fb_info->xres) / 2) & (~0x1);
		display_info->outW = fb_info->xres;
	}

	if (fb_info->yres > inH) {
		display_info->outH = inH;
		display_info->prev_y_off = 0;
		display_info->lcd_y_off = ((fb_info->yres - inH) / 2) & (~0x1);
	} else {
		display_info->outH = fb_info->yres;
		display_info->lcd_y_off = 0;
		display_info->prev_y_off = ((inH - fb_info->yres) / 2) & (~0x1);
	}

	if ( (display_info->inW != inW) || (display_info->inH != inH) ) {
	    	display_info->inW = inW;
		display_info->inH = inH;

		if (ginfo->packed) {
			int n_bytes = (inW * inH * fb_info->bpp) >> 3;
			if (display_info->ybuf) {
				free(display_info->ybuf);
				free(display_info->ubuf);
				free(display_info->vbuf);
			}

			display_info->ybuf = malloc(n_bytes);
			display_info->ubuf = malloc(n_bytes);
			display_info->vbuf = malloc(n_bytes);

			display_info->n_bytes = n_bytes;
		}
	}
}

static int frame_to_sep_ycbcr(__u8 **y, __u8 **cb, __u8 **cr,
			      __u8 *frame, struct display_info *display_info,
			      struct global_info *ginfo, __u32 *flag)
{
	__u8 *ybuf = NULL;
	__u8 *ubuf = NULL;
	__u8 *vbuf = NULL;
	int image_width = display_info->width;
	int image_height = display_info->height;
	int image_size = image_width * image_height;

	if (ginfo->packed) {
		/* convert yuv422 pack to planar */
		ybuf = malloc(image_size);
		ubuf = malloc(image_size);
		vbuf = malloc(image_size);

		if ( (ybuf == NULL) || (ubuf == NULL) || (vbuf == NULL) ) {
			printf("%s: alloc memory failed!\n", __FUNCTION__);
			return -1;
		}
		if (DF_IS_YCbCr422(display_info->fmt.fourcc))
			yCbCr422_pack_to_planar(ybuf, ubuf, vbuf, image_width, image_height, frame, display_info->fmt);
		else
			yCbCr444_pack_to_planar(ybuf, ubuf, vbuf, image_width, image_height, frame, display_info->fmt);
		*flag = 1;
	} else {
		ybuf = frame;
		ubuf = frame + image_size;
		if (DF_IS_YCbCr422(display_info->fmt))
			vbuf = ubuf + image_size / 2;
		else	      /* 444 */
			vbuf = ubuf + image_size;
		*flag = 0;
	}

	*y = ybuf;
	*cb = ubuf;
	*cr = vbuf;
	return 0;
}

static void frame_to_sep_cleanup(__u8 *y, __u8 *cb, __u8 *cr, __u32 flag)
{
	if (!flag)
		return;

	free(y);
	free(cb);
	free(cr);
}
#endif

void preview_display(__u8 *frame, struct display_info *display_info, struct fb_info *fb_info)
{
//	if (!display_info->ipu) {
		printf("%s L%d: not use ipu.\n", __func__, __LINE__);
		display_direct_to_fb(frame, display_info, fb_info);
/*	} else {
		__u8 *y = NULL;
		__u8 *cb = NULL;
		__u8 *cr = NULL;
		__u32 flag = 0;

		printf("%s L%d: use ipu.\n", __func__, __LINE__);

		frame_to_sep_ycbcr(&y, &cb, &cr, frame, display_info, ginfo, &flag);
		// put image to ipu 
		jz47_put_image_with_ipu(y, cb, cr, display_info->inW, display_info->inH);

		frame_to_sep_cleanup(y, cb, cr, flag);
	}
*/
}

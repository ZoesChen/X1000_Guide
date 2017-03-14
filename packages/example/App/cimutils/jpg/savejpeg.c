#include <stdio.h>
#include <preview_display.h>
#include "jpeg.h"

#include "savejpeg.h"
#include "jpeglib.h"
#define __u8 unsigned char
extern void * jz_jpeg;
int save_jpeg_userptr = 0;
//compress yuv420 to jpeg
static int jpeg_enc_yv12(unsigned char* buffer, int width, int height, int quality, FILE* outfile)
{
}

static int ycbcr422_planar_to_jpeg(__u8 *y, __u8 *cb, __u8 *cr,
			    int image_width, int image_height, FILE *fp, int quality)
{

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	// JSAMPROW row_pointer[1];  /* pointer to JSAMPLE row[s] */
	// int row_stride;    /* physical row width in image buffer */
	JSAMPIMAGE  buffer;
	int band,i,buf_width[3],buf_height[3], mem_size, max_line, counter;
	__u8 *yuv[3];
	__u8 *pSrc, *pDst;

	yuv[0] = y;
	yuv[1] = cb;
	yuv[2] = cr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);

	cinfo.image_width = image_width;  /* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = 3;    /* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB;  /* colorspace of input image */

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE );

	cinfo.raw_data_in = TRUE;
	cinfo.jpeg_color_space = JCS_YCbCr;
	cinfo.comp_info[0].h_samp_factor = 2;
	cinfo.comp_info[0].v_samp_factor = 1;

	jpeg_start_compress(&cinfo, TRUE);

	buffer = (JSAMPIMAGE) (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_IMAGE, 3 * sizeof(JSAMPARRAY));
	for(band=0; band <3; band++)
	{
		buf_width[band] = cinfo.comp_info[band].width_in_blocks * DCTSIZE;
		buf_height[band] = cinfo.comp_info[band].v_samp_factor * DCTSIZE;
		buffer[band] = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, buf_width[band], buf_height[band]);
	}

	max_line = cinfo.max_v_samp_factor*DCTSIZE;
#if 0
	printf("image width = %d height = %d max_line = %d\n", image_width, image_height, max_line);
	for (band = 0; band < 3; band++) {
		printf("band %d: buf_width = %d, buf_height = %d\n", band, buf_width[band], buf_height[band]);
	}
#endif
	for(counter=0; cinfo.next_scanline < cinfo.image_height; counter++)
	{
		//buffer image copy.
		for(band=0; band <3; band++)
		{
			mem_size = buf_width[band];
			pDst = (__u8 *) buffer[band][0];
			pSrc = (__u8 *) yuv[band] + counter*buf_height[band] * buf_width[band];

			for(i=0; i <buf_height[band]; i++)
			{
				memcpy(pDst, pSrc, mem_size);
				pSrc += buf_width[band];
				pDst += buf_width[band];
			}
		}
		jpeg_write_raw_data(&cinfo, buffer, max_line);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	return 0;

}
int yuv422_to_jpeg_soft (void * handle, const __u8 *frm, FILE *fp, struct display_info *dis, int quality)
{
	__u8 *ybuf = NULL;
	__u8 *ubuf = NULL;
	__u8 *vbuf = NULL;
	int width = dis->width;
	int height = dis->height;
//	int fourcc = dis->fmt.fourcc;
	int fmt = dis->fmt.format;
	int retval = -1;

	/* convert yuv422 pack to planar */

	printf("==>%s(): w=%d, h=%d, fmt=0x%08x\n", __func__, width, height, fmt);

	ybuf = malloc(width * height);
	ubuf = malloc(width * height);
	vbuf = malloc(width * height);

	if ( (ybuf == NULL) || (ubuf == NULL) || (vbuf == NULL) ) {
		printf("%s: alloc memory failed!\n", __FUNCTION__);
		return -1;
	}

	yCbCr422_pack_to_planar(ybuf, ubuf, vbuf, frm, dis);
	//uyvy422_pack_to_planar(ybuf, ubuf, vbuf, image_width, image_height, frm);
	retval = ycbcr422_planar_to_jpeg(ybuf, ubuf, vbuf, width, height, fp, quality);

	free(ybuf);
	free(ubuf);
	free(vbuf);

	return retval;
}

int yuv422_packed_to_jpeg(void *handle, const __u8 *frm, FILE *fp, struct display_info *dis, int quality)
{
	int width = dis->width;
	int height = dis->height;
	int retval = -1;
	if(save_jpeg_userptr)
	{
         	printf("==>%s %dL  save_jpeg_userptr =%d \n", __func__, __LINE__, save_jpeg_userptr);
        	retval = yuv422_to_jpeg (handle, frm, fp, width, height, quality);    //defined in libjpeg.so for userptr model
	}
	else
	{
         	printf("==>%s %dL  save_jpeg_userptr =%d \n", __func__, __LINE__, save_jpeg_userptr);
#ifdef USE_V4L2
                retval = yuv422_to_jpeg (handle, frm, fp, width, height, quality);    //defined in libjpeg.so for userptr model
#else
	        retval = yuv422_to_jpeg_soft(handle, frm, fp, dis, 100);          //defined in savejpeg.c for mmap model
#endif
        }
	return retval;
}

int yuv422_sep_to_jpeg(__u8 *data, int image_width, int image_height, FILE *fp, int quality)
{

}
int itu656_rearrange(__u8 *src, __u8 *dest, int fmt, int width, int height)
{
	return 0;
}
/*
int itu656_yuv422_to_jpeg(__u8 *data, int fmt, int width, int height, FILE *fp, int quality)
{
	__u8 *buf = NULL;
	int ret = 0;
	int i;

	printf("==>%s L%d: fmt=0x%08x, width=%d, height=%d\n", __func__, __LINE__, fmt, width, height);

	buf = malloc(width * height * 3);
	if (buf == NULL) {
		printf("==>%s: fail to alloc memory!!\n", __func__);
		return -1;
	}

	width *= 2;	// 2 bytes per pixel for yuv422

	//rearrange
	for (i = 0; i < height/2; i++) {
		memcpy(buf + 2 * i * width, data + i * width, width);
		memcpy(buf + (2 * i + 1) * width, data + (i + height/2) * width, width);
	}
	printf("i=%d, height=%d\n", i, height);

	width /= 2;

	ret = yuv422_packed_to_jpeg(buf, fmt, width, height, fp, quality);

	free(buf);
	return ret;
}
*/
#if 0
static void yuv420_mb_to_planar(__u8 *data, int width, int height, __u8 *ybuf, __u8 *ubuf, __u8 *vbuf)
{
	int mb_size = 16*16;
	int mb_per_line = width / 16;	//We assume that the width may be divided by 16 without remainder.
	int mb_line_size = mb_per_line * mb_size;	//(320/16) * (16*16)

	int uv_size = 8*8*2;
	int uv_line_size = mb_per_line * uv_size;

	int ylen = width * height;
	int ulen = ylen / 4;
	int vlen = ylen / 4;

	__u8 *buf = data;
	int i, j;

	printf("==>mb_size=%d, mb_per_line=%d, mb_line_size=%d, uv_size=%d, uv_line_size=%d\n", mb_size, mb_per_line, mb_line_size, uv_size, uv_line_size);
	printf("00: w=%d, h=%d\n", width, height);
	printf("11: buf = %p\n", buf);
	//y => 16x16 macro block
	for (j = 0; j < ylen / mb_line_size; j++) {
		for (i = 0; i < mb_line_size; i++) {
			ybuf[i] = buf[i/16 * mb_size + i%16];
		}

		ybuf += mb_line_size;
		buf += mb_line_size;
	}

	printf("22: buf = %p\n", buf);

	//uv => 8x16 macro block (8x8 u + 8x8 v)
	for (j = 0; j < (ulen + vlen) / uv_line_size; j++) {
		for (i = 0; i < uv_line_size; i++) {
			ubuf[i] = buf[ i/8 * uv_size + i%8 ];
			vbuf[i] = buf[ i/8 * uv_size + i%8 + uv_size/2 ];
		//	vbuf[i] = buf[ i/8 * uv_size + i%8 + 8 ];
		}

		ubuf += uv_line_size;
		vbuf += uv_line_size;
		buf += uv_line_size;
	}

	printf("33: buf = %p\n", buf);
}
#else
/* static void yuv420_mb_to_planar(__u8 *data, int width, int height, __u8 *ybuf, __u8 *ubuf, __u8 *vbuf) */
/* { */
/* } */
#endif

#define MB_DEBUG	0
/* static int yuv420_mb_to_jpeg(__u8 *data, int image_width, int image_height, FILE *fp, int quality) */
/* { */
/* 	return 0; */
/* } */

int yuv420_sep_to_jpeg(__u8 *data, int mb, int image_width, int image_height, FILE *fp, int quality)
{
}
#if 0
static void dump_buf(__u8 *buf, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		printf("0x%02x, ", buf[i]);
		if (i % 16 == 15)
			printf("\n");
	}
	printf("\n");
}
#endif

/* static void yuv444_planar_to_yuv422_planar(__u8 *yin, __u8 *uin, __u8 *vin, */
/* 					   __u8 *yout, __u8 *uout, __u8 *vout, */
/* 					   int width, int height) */
/* { */
/* } */

int yuv444_packed_to_jpeg(__u8 *data, FILE *fp, struct display_info *dis, int quality)
{
}

int yuv444_sep_to_jpeg(__u8 *data, int image_width, int image_height, FILE *fp, int quality)
{
	return 0;
}

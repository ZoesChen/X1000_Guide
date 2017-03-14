#include "savebmp.h"

BITMAPFILEHEADER  bfile = {
	.bfType = 0x4d42,	//"BM"
	.bfSize = 800*600*3+0x36,	//frame size + headers
	.bfReserved1 = 0,	
	.bfReserved2 = 0,	
	.bfoffBits = 0x36
};

BITMAPINFOHEADER  binfo = {
	.biSize = 0x28,		//sizeof(BITMAPINFOHEADER)
	.biWidth = 800,
	.biHeight = 600,
	.biPlanes = 1,
	.biBitCount = 0x18,	//24-bit true color
	.biCompress = 0,	//no compress
	.biSizeImage = 0,	//?
	.biXPelsPerMeter = 0x0b40,	//?
	.biYPelsPerMeter = 0x0b40,	//?
	.biClrUsed = 0,		//?
	.biClrImportant = 0	//?
};

int save_bgr_to_bmp(__u8 *buf, struct display_info *dis, FILE *fp)
{
	int len = 0;
	int ret = 0;
	
//	printf("sizeof(unsigned short) = %d\n", sizeof(unsigned short));
//	printf("sizeof(unsigned long) = %d\n", sizeof(unsigned long));
	printf("sizeof bfile = %d\n", sizeof(BITMAPFILEHEADER));
	printf("sizeof binfo = %d\n", sizeof(BITMAPINFOHEADER));

	bfile.bfSize = dis->width * dis->height * 3;
	binfo.biWidth = dis->width;
	binfo.biHeight = dis->height;

	len = bfile.bfSize;

	binfo.biXPelsPerMeter = 0;
	binfo.biYPelsPerMeter = 0;

	ret = fwrite(&bfile, sizeof(BITMAPFILEHEADER), 1, fp);
	if (ret != 1) {
		printf("%s:%d write error!!!\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ret = fwrite(&binfo, sizeof(BITMAPINFOHEADER), 1, fp);
	if (ret != 1) {
		printf("%s:%d write error!!!\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ret = fwrite(buf, sizeof(__u8), len, fp);
	if (ret != len) {
		printf("%s:%d write error!!!\n", __FUNCTION__, __LINE__);
		return -1;
	}

	return 0;
}

//#define BMP_DBG	1
int convert_yuv_to_rgb24(__u8 *frame, __u8 *rgb, struct display_info *dis)
{
	int fourcc = dis->fmt.fourcc;
	int format = dis->fmt.format;

	int line, col;
	int colorR, colorG, colorB;
	struct yuv422_sample yuv422_samp;
	struct yuv422_sample *yuv422_pixel =  NULL;
	struct yuv444_sample yuv444_samp;
	struct yuv444_sample *yuv444_pixel = NULL;
	__u8 y0, y1, u, v;
	__u8 *y = NULL;
	__u8 *cb = NULL;
	__u8 *cr = NULL;

	struct bmp_bgr *bgr = (struct bmp_bgr *)rgb;

#if BMP_DBG
	__u8 *bmp_bgr_buf = rgb;
	__u8 *yuv_buf = frame;

	FILE *bmp_bgr_raw = NULL;
	FILE *yuv_raw = NULL;
	int w = dis->width;
	int h = dis->height;
	int i;

	yuv_raw = fopen("yuv.raw", "w");
	if (!yuv_raw)
		printf("==>%s: fail to open yuv.raw!!\n", __func__);
	else {
		i = fwrite(yuv_buf, sizeof(__u8), w*h*2, yuv_raw);
		if (i != w*h*2)
			printf("==>%s: fail to write yuv.raw!!\n", __func__);
		fclose(yuv_raw);
	}

#endif

	printf("-->%s L%d: planar=%d\n", __func__, __LINE__, dis->planar);
	if (dis->planar) {
		int img_size = dis->width * dis->height;
		if (DF_IS_YCbCr422(fourcc)) {	//YUV422
			y = frame;
			cb = frame + img_size;
			cr = frame + img_size + img_size / 2;
		} else {	//YUV444
			y = frame;
			cb = frame + img_size;
			cr = cb + img_size;
		}
	}

	for (line = dis->yoff; (line < (dis->yoff + dis->height)); line++)
	{
		bgr = (struct bmp_bgr *)(rgb + dis->width * (dis->height - line - 1) * 3);

		if (!dis->planar) {
			if (DF_IS_YCbCr422(fourcc)) {
				yuv422_pixel = (struct yuv422_sample *)
					(frame + line * dis->width * 2 + dis->xoff * 2);
			} else {
				yuv444_pixel = (struct yuv444_sample *)
					(frame + line * dis->width * 3 + dis->xoff * 3);
			}
		}
		for (col = dis->xoff; (col < (dis->xoff + dis->width)); col += 2)
		{
			if (!dis->planar) {
				if (DF_IS_YCbCr422(fourcc)) {
					yCbCr422_normalization(format, yuv422_pixel, &yuv422_samp);
					y0 = yuv422_samp.b1;
					u = yuv422_samp.b2;
					y1 = yuv422_samp.b3;
					v = yuv422_samp.b4;

					yuv422_pixel ++;
				} else {
					yCbCr444_normalization(format, yuv444_pixel, &yuv444_samp);
					y0 = yuv444_samp.b1;
					u = yuv444_samp.b2;
					v = yuv444_samp.b3;

					yuv444_pixel++;

					yCbCr444_normalization(format, yuv444_pixel, &yuv444_samp);
					y1 = yuv444_samp.b1;
					u = (yuv444_samp.b2 + u) / 2;
					v = (yuv444_samp.b3 + v) / 2;

					yuv444_pixel++;
				}
			} else {
				if (DF_IS_YCbCr422(fourcc)) {
					y0 = *(y + line * dis->width + col);
					y1 = *(y + line * dis->width + col + 1);
					u = *(cb + line * dis->width / 2 + col / 2);
					v = *(cr + line * dis->width / 2 + col / 2);
				} else {
					y0 = *(y + line * dis->width + col);
					y1 = *(y + line * dis->width + col + 1);
					u = (*(cb + line * dis->width + col) + *(cb + line * dis->width + col + 1)) / 2;
					v = (*(cr + line * dis->width + col) + *(cr + line * dis->width + col + 1)) / 2;
				}
			}

			if (dis->only_y) {
				fprintf(stdout, "-->%s L%d: ignore uv.\n", __func__, __LINE__);
				u = 128;
				v = 128;
			}

			//convert from YUV domain to RGB domain
			colorB = y0 + ((443 * (u - 128)) >> 8);
			colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);

			colorG = y0 -
				((179 * (v - 128) +
				  86 * (u - 128)) >> 8);

			colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);

			colorR = y0 + ((351 * (v - 128)) >> 8);
			colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);

			//Windows BitMap BGR
			bgr->blue = colorB;
			bgr->green = colorG;
			bgr->red = colorR;
			bgr++;

			colorB = y1 + ((443 * (u - 128)) >> 8);
			colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);

			colorG = y1 -
				((179 * (v - 128) +
				  86 * (u - 128)) >> 8);

			colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);

			colorR = y1 + ((351 * (v - 128)) >> 8);
			colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);

			//Windows BitMap BGR
			bgr->blue = colorB;
			bgr->green = colorG;
			bgr->red = colorR;
			bgr++;
		}
	}

#if BMP_DBG
	bmp_bgr_raw = fopen("bmp_bgr.raw", "w");
	if (!bmp_bgr_raw)
		printf("==>%s: fail to open bmp_bgr.raw!!\n", __func__);
	else {
		i = fwrite(bmp_bgr_buf, sizeof(__u8), w*h*3, bmp_bgr_raw);
		if (i != w*h*3)
			printf("==>%s: fail to write bmp_bgr.raw!!\n", __func__);
		fclose(bmp_bgr_raw);
	}
#endif

	return 0;
}


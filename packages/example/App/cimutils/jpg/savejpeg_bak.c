#include "savejpeg.h"
#include "jpeglib.h"

//compress yuv420 to jpeg
static int jpeg_enc_yv12(unsigned char* buffer, int width, int height, int quality, FILE* outfile)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int ret = 0;

    if(buffer == NULL || width <=0 || height <=0|| outfile == NULL)
        return FALSE;

    printf("==>%s() L%d\n", __func__, __LINE__);

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;
    jpeg_set_defaults(&cinfo);

#if JPEG_LIB_VERSION >= 70

 cinfo.do_fancy_downsampling = FALSE;
 cinfo.dct_method = JDCT_FASTEST;
 cinfo.smoothing_factor = 0;

#endif
    jpeg_set_quality(&cinfo, quality, TRUE);
    cinfo.raw_data_in = TRUE;

    {
        JSAMPARRAY pp[3];
        JSAMPROW *rpY = (JSAMPROW*)malloc(sizeof(JSAMPROW) * height);
        JSAMPROW *rpU = (JSAMPROW*)malloc(sizeof(JSAMPROW) * height);
        JSAMPROW *rpV = (JSAMPROW*)malloc(sizeof(JSAMPROW) * height);
        int k;
        //if(rpY == NULL && rpU == NULL && rpV == NULL)
        if(rpY == NULL || rpU == NULL || rpV == NULL)
        {
            ret = FALSE;
            goto exit;
        }
        cinfo.comp_info[0].h_samp_factor =
        cinfo.comp_info[0].v_samp_factor = 2;
        cinfo.comp_info[1].h_samp_factor =
        cinfo.comp_info[1].v_samp_factor =
        cinfo.comp_info[2].h_samp_factor =
        cinfo.comp_info[2].v_samp_factor = 1;
        jpeg_start_compress(&cinfo, TRUE);

        for (k = 0; k < height; k+=2)
        {
            rpY[k]   = buffer + k*width;
            rpY[k+1] = buffer + (k+1)*width;
            rpU[k/2] = buffer+width*height + (k/2)*width/2;
            rpV[k/2] = buffer+width*height*5/4 + (k/2)*width/2;
        }
        for (k = 0; k < height; k+=2*DCTSIZE)
        {
            pp[0] = &rpY[k];
            pp[1] = &rpU[k/2];
            pp[2] = &rpV[k/2];
            jpeg_write_raw_data(&cinfo, pp, 2*DCTSIZE);
        }
        jpeg_finish_compress(&cinfo);
        free(rpY);
        free(rpU);
        free(rpV);
    }

exit:
    jpeg_destroy_compress(&cinfo);
    return ret;
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
#if 0
int yuv422_packed_to_jpeg(const __u8 *frm, FILE *fp, struct display_info *dis, int quality)
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
#else
int yuv422_packed_to_jpeg(const __u8 *frm, FILE *fp, struct display_info *dis, int quality)
{
	int width = dis->width;
	int height = dis->height;
	int retval = -1;

	retval = yuv422_to_jpeg(frm, fp, width, height, quality);

	return retval;
}
#endif
int yuv422_sep_to_jpeg(__u8 *data, int image_width, int image_height, FILE *fp, int quality)
{

	return ycbcr422_planar_to_jpeg(data, data + image_width * image_height,
				       data + image_width * image_height + image_width * image_height / 2,
				       image_width, image_height, fp, quality);
}

int itu656_rearrange(__u8 *src, __u8 *dest, int fmt, int width, int height)
{
	__u8 *buf = dest;
	int i;

	printf("==>%s L%d: fmt=0x%08x, width=%d, height=%d\n", __func__, __LINE__, fmt, width, height);

	if (DF_IS_YCbCr422(fmt)) {
		width *= 2;	// 2 bytes per pixel for yuv422

		for (i = 0; i < height/2; i++) {
			memcpy(buf + 2 * i * width, src + i * width, width);
			memcpy(buf + (2 * i + 1) * width, src + (i + height/2) * width, width);
		}
	} else if (DF_IS_YCbCr420(fmt)) {
		width = width * 3 / 2;

		for (i = 0; i < height/2; i+=2) {
			memcpy(buf + 2 * i * width, src + i * width, width * 2);
			memcpy(buf + 2 * (i + 1) * width, src + (i + height/2) * width, width * 2);
		}
	} else
		printf("==>%s L%d: invalid format!!\n", __func__, __LINE__);

#if 0
	//rearrange
	for (i = 0; i < height/2; i++) {
		memcpy(buf + 2 * i * width, src + i * width, width);
		memcpy(buf + (2 * i + 1) * width, src + (i + height/2) * width, width);
	}
#endif
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
	int idx1, idx2;

	printf("==>mb_size=%d, mb_per_line=%d, mb_line_size=%d, uv_size=%d, uv_line_size=%d\n", mb_size, mb_per_line, mb_line_size, uv_size, uv_line_size);
	printf("00: w=%d, h=%d\n", width, height);
	printf("11: buf = %p\n", buf);
	//y => 16x16 macro block
	for (j = 0; j < ylen / mb_line_size; j++) {
		for (i = 0; i < mb_line_size; i++) {
			ybuf[i] = buf[ i/width*16 + (i%width)/16 * mb_size + i%16 ];
		}

		ybuf += mb_line_size;
		buf += mb_line_size;
	}

	printf("22: buf = %p\n", buf);

	uv_line_size /= 2;
	//uv => 8x16 macro block (8x8 u + 8x8 v)
	for (j = 0; j < (ulen + vlen) / uv_line_size; j++) {
		for (i = 0; i < uv_line_size; i++) {
		//	ubuf[i] = buf[ i/width*16 + (i%width)/8 * uv_size + i%8 ];
	//		vbuf[i] = buf[ i/width*16 + (i%width)/8 * uv_size + i%8 + uv_size/2 ];
		//	vbuf[i] = buf[ i/width*16 + (i%width)/8 * uv_size + i%8 + 8 ];

			idx1 = i/width*16 + (i%width)/8 * uv_size + i%8;
			idx2 = i/width*16 + (i%width)/8 * uv_size + i%8 + uv_size/2;
			ubuf[i] = buf[idx1];
			vbuf[i] = buf[idx2];
		}

		ubuf += uv_line_size;
		vbuf += uv_line_size;
	//	buf += uv_line_size;
	}

	printf("33: buf = %p\n", buf);
}
#endif

#define MB_DEBUG	0
static int yuv420_mb_to_jpeg(__u8 *data, int image_width, int image_height, FILE *fp, int quality)
{
	__u8 *buf = NULL;
	__u8 *ybuf = NULL;
	__u8 *ubuf = NULL;
	__u8 *vbuf = NULL;
	int ylen = image_width * image_height;
	int ulen = ylen / 4;
	int vlen = ylen / 4;
	int retval = -1;

#if MB_DEBUG
	FILE *yuv420_raw = NULL;
	FILE *raw_data = NULL;
	__u8 *tmp = data;
	int i;

	raw_data = fopen("raw_data.raw", "w");
	if (!raw_data)
		printf("==>%s: fail to open raw_data.raw!!\n", __func__);
	else {
		i = fwrite(tmp, sizeof(__u8), image_width*image_height*2, raw_data);
		if (i != image_width*image_height*2)
			printf("==>%s: fail to write raw_data.raw!!\n", __func__);
		fclose(raw_data);
	}
#endif

	printf("w=%d, h=%d, ylen=%d, ulen=%d, vlen=%d\n", image_width, image_height, ylen, ulen, vlen);

	/* convert yuv420 macro-block to planar */
	buf = malloc(image_width * image_height * 3);
	ybuf = malloc(image_width * image_height);
	ubuf = malloc(image_width * image_height);
	vbuf = malloc(image_width * image_height);

	if ( (buf == NULL) || (ybuf == NULL) || (ubuf == NULL) || (vbuf == NULL) ) {
		printf("%s: alloc memory failed!\n", __FUNCTION__);
		return -1;
	}

	yuv420_mb_to_planar(data, image_width, image_height, ybuf, ubuf, vbuf);

	memcpy(buf, ybuf, ylen);
	memcpy(buf + ylen, ubuf, ulen);
	memcpy(buf + ylen + ulen, vbuf, vlen);

#if MB_DEBUG
	yuv420_raw = fopen("yuv420_mb.raw", "w");
	if (!yuv420_raw)
		printf("==>%s: fail to open yuv420_mb.raw!!\n", __func__);
	else {
		i = fwrite(buf, sizeof(__u8), image_width*image_height*3, yuv420_raw);
		if (i != image_width*image_height*3)
			printf("==>%s: fail to write yuv420_mb.raw!!\n", __func__);
		fclose(yuv420_raw);
	}
#endif
	retval = jpeg_enc_yv12(buf, image_width, image_height, quality, fp);

	free(buf);
	free(ybuf);
	free(ubuf);
	free(vbuf);

	return retval;
}

int yuv420_sep_to_jpeg(__u8 *data, int mb, int image_width, int image_height, FILE *fp, int quality)
{
	int ret;

	printf("==>%s(): mb=%d\n", __func__, mb);

	if (mb) {
		ret = yuv420_mb_to_jpeg(data, image_width, image_height, fp, quality);
	} else {
		ret = jpeg_enc_yv12(data, image_width, image_height, quality, fp);
	}
	return ret;
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

static void yuv444_planar_to_yuv422_planar(__u8 *yin, __u8 *uin, __u8 *vin,
					   __u8 *yout, __u8 *uout, __u8 *vout,
					   int width, int height)
{
	int i;
	int u_idx = 0;
	int v_idx = 0;

	memcpy(yout, yin, width*height);

	for (i = 0; i < width*height; i += 2) {
		uout[u_idx] = (uin[i] + uin[i+1]) / 2;
		vout[v_idx] = (vin[i] + vin[i+1]) / 2;

		u_idx++;
		v_idx++;
	}
}

int yuv444_packed_to_jpeg(__u8 *data, FILE *fp, struct display_info *dis, int quality)
{
	__u8 *ybuf = NULL;
	__u8 *ubuf = NULL;
	__u8 *vbuf = NULL;
	__u8 *buf = NULL;
	__u8 *y = NULL;
	__u8 *cb = NULL;
	__u8 *cr = NULL;
	int image_width = dis->width;
	int image_height = dis->height;
	int fmt = dis->fmt.format;
	int retval = -1;

#if 0
	int i;
	FILE *data_raw = fopen("data.raw", "w");
	if (!data_raw)
		printf("==>%s: fail to open data_mb.raw!!\n", __func__);
	else {
		i = fwrite(data, sizeof(__u8), image_width*image_height*3, data_raw);
		if (i != image_width*image_height*3)
			printf("==>%s: fail to write data.raw!!\n", __func__);
		fclose(data_raw);
	}

#endif
	printf("==>%s L%d: w=%d, h=%d, fmt=0x%08x\n", __func__, __LINE__, image_width, image_height, fmt);

	ybuf = malloc(image_width * image_height);
	ubuf = malloc(image_width * image_height);
	vbuf = malloc(image_width * image_height);
	buf = malloc(image_width * image_height * 3);

	if ( (ybuf == NULL) || (ubuf == NULL) || (vbuf == NULL) || (buf == NULL)) {
		printf("%s: alloc memory failed!\n", __FUNCTION__);
		return -1;
	}

	/* convert yuv444 pack to planar */
	yCbCr444_pack_to_planar(ybuf, ubuf, vbuf, image_width, image_height, data, fmt);

	y = buf;
	cb = buf + image_width * image_height;
	cr = cb + image_width * image_height;

	/* convert yuv444 planar to yuv422 planar */
	yuv444_planar_to_yuv422_planar(ybuf, ubuf, vbuf, y, cb, cr, image_width, image_height);

	/* convert yuv422 planar to jpeg */
	retval = ycbcr422_planar_to_jpeg(y, cb, cr, image_width, image_height, fp, quality);

	free(ybuf);
	free(ubuf);
	free(vbuf);
	free(buf);

	return retval;
}

int yuv444_sep_to_jpeg(__u8 *data, int image_width, int image_height, FILE *fp, int quality)
{
	__u8 *y = NULL;
	__u8 *cb = NULL;
	__u8 *cr = NULL;
	int img_size = image_width * image_height;
	int ret = 0;

	printf("==>%s() L%d\n", __func__, __LINE__);

	y = malloc(image_width * image_height);
	cb = malloc(image_width * image_height);
	cr = malloc(image_width * image_height);

	if ( (y == NULL) || (cb == NULL) || (cr == NULL) ) {
		printf("%s: alloc memory failed!\n", __FUNCTION__);
		return -1;
	}

	yuv444_planar_to_yuv422_planar(data, data+img_size, data+img_size+img_size, y, cb, cr, image_width, image_height);

	ret = ycbcr422_planar_to_jpeg(y, cb, cr, image_width, image_height, fp, quality);

	free(y);
	free(cb);
	free(cr);

	return ret;
}

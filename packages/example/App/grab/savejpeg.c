#include "savejpeg.h"
#include "jpeglib.h"
#include "pixel.h"
#include "huffman.h"

__attribute__((unused)) static void
getPictureName (char *Picture, int fmt)
{
	char temp[80];
	char *myext[] = { "pnm", "jpg" };
	time_t curdate;
	struct tm *tdate;
	memset (temp, '\0', sizeof (temp));
	time (&curdate);
	tdate = localtime (&curdate);
	snprintf (temp, 26, "P-%02d:%02d:%04d-%02d:%02d:%02d.%s",
		  tdate->tm_mon + 1, tdate->tm_mday, tdate->tm_year + 1900,
		  tdate->tm_hour, tdate->tm_min, tdate->tm_sec, myext[fmt]);

	memcpy (Picture, temp, strlen (temp));
}

int is_huffman(unsigned char *buf)
{
	unsigned char *ptbuf;
	int i = 0;
	ptbuf = buf;
	while (((ptbuf[0] << 8) | ptbuf[1]) != 0xffda){
		if(i++ > 2048)
			return 0;
		if(((ptbuf[0] << 8) | ptbuf[1]) == 0xffc4)
			return 1;
		ptbuf++;
	}
	return 0;
}

#if 0
int get_picture(unsigned char *buf,int size, char *name) {
	FILE *file;
	unsigned char *ptdeb,*ptcur = buf;
	int sizein;
	//char *name = NULL;
	//name = calloc(80,1);
	//getPictureName (name, 1);
	unsigned char *p=buf;

	file = fopen(name, "wb");
	if (file != NULL) {
		if(!is_huffman(buf)){
			printf("===>is not huffman!\n");
			ptdeb = ptcur = buf;
			while (((ptcur[0] << 8) | ptcur[1]) != 0xffc0)
				ptcur++;
			sizein = ptcur-ptdeb;
			/* cut data before 0xff 0xdb */
			while(1)
			{
				if( (*buf==0xff)&&(*(buf+1)==0xdb) )
				{
					buf -= 2;
					*buf = 0xff;
					*(buf+1) = 0xd8;
					break;
				}
				else
					buf++;
			}

			sizein -= (int)(p-buf);
			//	printf("%s  p:%p, b:%p\n",__func__,p, buf);
			/* end --treckle */
			fwrite(buf,
			       sizein, 1, file);
			fwrite(dht_data,
			       DHT_SIZE, 1, file);
			fwrite(ptcur,size-sizein,1,file);
		} else {
			/* cut data from 0xff 0xd8 to 0xff 0xdb */
			while(1)
			{
				if( (*ptcur==0xff)&&(*(ptcur+1)==0xdb) )
				{
					ptcur -= 2;
					*ptcur = 0xff;
					*(ptcur+1) = 0xd8;
					break;
				}
				else
					ptcur++;
			}

			size -= (int)(p-ptcur);
			//	printf("%s  p:%p, b:%p,s:%d\n",__func__,p, ptcur,size);
			/* end --treckle */
			fwrite(ptcur,size,1,file); /* ptcur was uninit -wsr */
		}
		fclose(file);
	}
	//if(name)
	//	free(name);
	return 0;
}
#else
int
get_picture(unsigned char *buf,int size, const char *name)
{
	FILE *file;
	unsigned char *ptdeb,*ptcur = buf;
	int sizein;
#if 0
	char *name = NULL;

	name = calloc(80,1);
	getPictureName (name, 1);
#endif
	// printf("$ly---before open\n");
	file = fopen(name, "wb");
	// printf("$ly---after open\n");
	if (file != NULL) {
		if(!is_huffman(buf)){
			ptdeb = ptcur = buf;
		 printf("$ly-test  get_picture huffman\n");
			while (((ptcur[0] << 8) | ptcur[1]) != 0xffc0)
				ptcur++;
			// printf("$ly-test2\n");
			sizein = ptcur-ptdeb;
			fwrite(buf,
			       sizein, 1, file);
			// printf("$ly-test3\n");
			fwrite(dht_data,
			       DHT_SIZE, 1, file);
			// printf("$ly-test4\n");
			fwrite(ptcur,size-sizein,1,file);
			// printf("$ly-test5\n");
		} else {
			// printf("$ly-test6\n");
		 printf("$ly-test  get_picture else\n");
			fwrite(ptcur,size,1,file); /* ptcur was uninit -wsr */
		}
		fclose(file);
	}
#if 0
	if(name)
		free(name);
#endif
	return 0;
}
#endif

int ycbcr422_planar_to_jpeg(__u8 *y, __u8 *cb, __u8 *cr,
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

void yCbCr422_pack_to_planar(__u8 *y, __u8 *cb, __u8 *cr,
			     int image_width, int image_height,
			     __u8 *src) {
	int line, col;
	int y_index = 0;
	int uv_index = 0;
	struct yuv422_sample *src_samp = (struct yuv422_sample *)src;

	for (line = 0; line < image_height; line++) {
		for (col = 0; col < image_width; col += 2) {
			y[y_index++] = src_samp->b1;
			cb[uv_index] = src_samp->b2;
			y[y_index++] = src_samp->b3;
			cr[uv_index] = src_samp->b4;

			uv_index++;
			src_samp++;
		}
	}
}

int yuv422_packed_to_jpeg(__u8 *data, __u32 fmt,
			   int image_width, int image_height,
			   FILE *fp, int quality)
{
	__u8 *ybuf = NULL;
	__u8 *ubuf = NULL;
	__u8 *vbuf = NULL;
	int retval = -1;
	/* convert yuv422 pack to planar */

	ybuf = (__u8 *)malloc(image_width * image_height);
	ubuf = (__u8 *)malloc(image_width * image_height);
	vbuf = (__u8 *)malloc(image_width * image_height);

	if ( (ybuf == NULL) || (ubuf == NULL) || (vbuf == NULL) ) {
		printf("%s: alloc memory failed!\n", __FUNCTION__);
		return -1;
	}

	yCbCr422_pack_to_planar(ybuf, ubuf, vbuf, image_width, image_height, data);
	//uyvy422_pack_to_planar(ybuf, ubuf, vbuf, image_width, image_height, data);
	retval = ycbcr422_planar_to_jpeg(ybuf, ubuf, vbuf, image_width, image_height, fp, quality);
	free(ybuf);
	free(ubuf);
	free(vbuf);

	return retval;
}

int yuv422_sep_to_jpeg(__u8 *data, int image_width, int image_height, FILE *fp, int quality)
{

	return ycbcr422_planar_to_jpeg(data, data + image_width * image_height,
				       data + image_width * image_height + image_width * image_height / 2,
				       image_width, image_height, fp, quality);
}



int get_pictureYUYV(unsigned char *buf,int width,int height, const char *name)
{
	FILE *foutpict;

	if (!name)
		return -EINVAL;

	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	//char *name = NULL;
	//name = calloc(80,1);
	//getPictureName (name, 0);
	int fd;
	fd = fopen("luo.data","w");
	fwrite(buf,640*480*2,1,fd);
	fclose(fd);

	foutpict = fopen (name, "wb");
//	fwrite(buf,width*height*2,1,foutpict);
	yuv422_packed_to_jpeg(buf, 0, width, height, foutpict, 100);
	fclose (foutpict);

	return 0;
}

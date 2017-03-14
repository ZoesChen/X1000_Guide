#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <libgen.h>
#include <linux/limits.h>

#include "jpeglib.h"
#include "pixel.h"

static int write_to_file(const char *filepath, const unsigned char *buf, unsigned long len)
{
	int fd = open(filepath, O_CREAT|O_TRUNC|O_RDWR, 0666);
	if (fd < 0) {
		printf("open %s failed\n", filepath);
		return -1;
	}
	int ret = write(fd, buf, len);
	if (ret < len) {
		printf("write file failed\n");
		return -1;
	}
	close(fd);

	return 0;
}

unsigned char* read_file_to_buf(const char *filepath, unsigned int * len)
{
	struct stat stat;
	if (lstat(filepath, &stat) < 0)
		return NULL;


	unsigned char *buf = (unsigned char *)malloc(stat.st_size + 1);
	if (!buf) return NULL;

	memset(buf, 0, stat.st_size+1);
	int fd = open(filepath, O_RDONLY);
	if (fd < 0) {
		free(buf);
		return NULL;
	}
	ssize_t whole_size = 0;
	do {
		ssize_t ret = read(fd, buf, stat.st_size);
		if (ret < 0) {
			free(buf);
			return NULL;
		}
		whole_size += ret;
	} while (whole_size < stat.st_size);

	*len = stat.st_size;
	close(fd);

	return buf;
}

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
	for(band=0; band <3; band++) {
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
	for(counter=0; cinfo.next_scanline < cinfo.image_height; counter++) {
		//buffer image copy.
		for(band=0; band <3; band++) {
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
	unsigned long long starttime,endtime;
	int grap_count = 10;
	/* convert yuv422 pack to planar */

	ybuf = malloc(image_width * image_height);
	ubuf = malloc(image_width * image_height);
	vbuf = malloc(image_width * image_height);

	if ( (ybuf == NULL) || (ubuf == NULL) || (vbuf == NULL) ) {
		printf("%s: alloc memory failed!\n", __FUNCTION__);
		return -1;
	}

	starttime = usectime();


	yCbCr422_pack_to_planar(ybuf, ubuf, vbuf, image_width, image_height, data);
	retval = ycbcr422_planar_to_jpeg(ybuf, ubuf, vbuf, image_width, image_height, fp, quality);

	endtime = usectime();
	printf("1 frame time = %lld %lldns/frame",endtime - starttime,(endtime-starttime)/1);

	free(ybuf);
	free(ubuf);
	free(vbuf);

	return retval;
}

int get_pictureYUYV(unsigned char *buf,int width,int height, char *outfile_name)
{
	FILE *foutpict;

	if (!outfile_name)
		return -EINVAL;

	foutpict = fopen (outfile_name, "wb");
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	yuv422_packed_to_jpeg(buf, 0, width, height, foutpict, 100);
	fclose (foutpict);

	return 0;
}

static struct option long_options[] = {
	{ "help", 0, 0, 'H'},
	{ "width", 1, 0, 'w' },
	{ "height", 1, 0, 'h' },
	{ "infile", 1, 0, 'i'},
	{ "outfile", 1, 0, 'o' },
	{0, 0, 0, 0}
};

static char optstring[] = "Hw:h:i:o:";

void usage(void) {
	printf("usage: raw2jpg [-w <width>] [-h <height>] -i <infile> -o <outfile>\n");
	printf("--help -H\t\tprint this message \n");
	printf("--width -w\t\tgrab width\n");
	printf("--height -h\t\tgrab height\n");
	printf("--infile -c\t\tinput raw file\n");
	printf("--outfile -r\t\toutput jpg file\n");
	printf("\n");
}

static char infile[PATH_MAX + 1];
static char outfile[PATH_MAX + 1];

int main(int argc, char *argv[]) {
	int i = 0;
	unsigned char *infile_buf;
	int infile_len;
	unsigned char *outfile_buf;
	int outfile_len;
	int width;
	int height;
	char c;

	while (1) {
		c = getopt_long(argc, argv, optstring, long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'H':
			usage();
			return 0;

		case 'w':
			width = atoi(optarg);
			break;

		case 'h':
			height = atoi(optarg);
			break;

		case 'i':
			snprintf(infile, PATH_MAX, "%s", optarg);
			printf("infile %s\n", infile);
			break;

		case 'o':
			snprintf(outfile, PATH_MAX, "%s", optarg);
			printf("outfile %s\n", outfile);
			break;

		default:
			/* do nothing */
			usage();
			return 127;
		}
	}

	infile_buf = read_file_to_buf(infile, &infile_len);
	if (!infile_buf) {
		fprintf(stderr, "read infile %s failed\n", infile);
		return -1;
	}
	printf("infile_len = %u\n", infile_len);

	get_pictureYUYV(infile_buf, width, height, outfile);

	return 0;
}



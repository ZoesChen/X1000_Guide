#include "headers.h"

/*
 * calc_frame_size
 *
 * @width:
 * @height:
 * @fmt:	fourcc code
 *
 */
static int calc_frame_size(__u32 width, __u32 height, int fmt)
{
	int y_len = width * height;
	int cb_len = 0;
	int cr_len = 0;

	printf("====>%s:%d img_width = %d img_height = %d fmt = %#0x\n",
	       __FUNCTION__, __LINE__, width, height, fmt);

	if (DF_IS_YCbCr420(fmt)) {
		printf("420..................\n");
		cb_len = y_len / 4;
		cr_len = y_len / 4;

	} else if (DF_IS_YCbCr422(fmt)) {
		printf("422..................\n");
		cb_len = y_len / 2;
		cr_len = y_len / 2;
	} else {
		printf("444....................\n");
		cb_len = y_len;
		cr_len = y_len;
	}

	return y_len + cb_len + cr_len;
}

int preview_save_raw_file(FILE *file, const __u8 *frame, struct display_info *display_info)
{
	int total_len = calc_frame_size(display_info->width, display_info->height, display_info->fmt.fourcc);
	char padding_code[HEADER_SIZE];	// 0x55
	int padding_size = HEADER_SIZE - sizeof(struct display_info);
	int ret = 0;

#if 0
	memset(padding_code, 0x55, HEADER_SIZE);	// ASCII 'U'

	ret = fwrite(display_info, sizeof(struct display_info), 1, file);
	if (ret != 1) {
		fprintf(stderr, "%s() L%d: write error!!!\n", __func__, __LINE__);
		return -1;
	}

	//padding	=>	all 'U' (ASCII 0x55)
	ret = fwrite(padding_code, sizeof(char), padding_size, file);
	if (ret != padding_size) {
		fprintf(stderr, "%s() L%d: write error!!!\n", __func__, __LINE__);
		return -1;
	}
#endif
	ret = fwrite(frame, sizeof(__u8),  total_len, file);
	if (ret != total_len) {
		fprintf(stderr, "%s() L%d: write error!!!\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}

int preview_save_raw(char *filename, const __u8 *frame, struct display_info *display_info)
{
	FILE *raw_file;
	int ret = 0;

	raw_file = fopen(filename, "w");
	if (!raw_file) {
		printf("open %s failed!!!\n", filename);
		return -1;
	}

	ret = preview_save_raw_file(raw_file, frame, display_info);

	fclose(raw_file);

	return ret;
}

static int read_raw_file(FILE *file, __u8 **frm, struct display_info *dis)
{
	__u8 *frame = NULL;
	int total_len = 0;
	int ret;

	char padding_buf[HEADER_SIZE];
	int padding_size = HEADER_SIZE - sizeof(struct display_info);

	ret = fread((void *)dis, sizeof(struct display_info), 1, file);
	if (ret != 1) {
		fprintf(stderr, "%s() L%d: read error!!!\n", __func__, __LINE__);
		return -1;
	}

	ret = fread((void *)padding_buf, sizeof(char), padding_size, file);
	if (ret != padding_size) {
		fprintf(stderr, "%s() L%d: read error!!!\n", __func__, __LINE__);
		return -1;
	}

 	total_len =  calc_frame_size(dis->width, dis->height, dis->fmt.fourcc);
	frame = (__u8 *)malloc(total_len);
	if (!frame) {
		fprintf(stderr, "%s() L%d: malloc fail!!!", __func__, __LINE__);
		return -1;
	}

	ret = fread((void *)frame, sizeof(__u8), total_len, file);
	if (ret != total_len) {
		fprintf(stderr, "%s() L%d: read error!!!\n", __func__, __LINE__);
		return -1;
	}

	*frm = frame;

	return total_len;
}

int preview_display_raw(const char *filename, struct fb_info *fb)
{
	FILE *raw_file;
	__u8 *frame = NULL;
	struct display_info dis;

	printf("display raw file %s\n", filename);

	raw_file = fopen(filename, "r");
	if (!raw_file) {
		printf("open %s failed!!!\n", filename);
		return -1;
	}

	if (read_raw_file(raw_file, &frame, &dis) < 0) {
		return -1;
	}

	preview_display(frame, &dis, fb);

	//cleanup
	free(frame);
	fclose(raw_file);

	return 0;
}

static int __convert_packed_2_planar(FILE *packed_file, FILE *planar_file)
{
	__u8 *packed_frame = NULL;
	__u8 *planar_frame = NULL;
	__u8 *y = NULL;
	__u8 *cb = NULL;
	__u8 *cr = NULL;
	struct display_info display_info;
	int total_len = 0;
	int ret = 0;

	total_len = read_raw_file(packed_file, &packed_frame, &display_info);

	if (total_len < 0)
		return -1;

	if (display_info.planar) {
		free(packed_frame);
		printf("orig file is already in planar format!!!\n");
		return 0;
	}

	planar_frame = malloc(total_len);
	y = planar_frame;
	cb = y + display_info.width * display_info.height;

	if (DF_IS_YCbCr422(display_info.fmt.fourcc)) {
		if (total_len != display_info.width * display_info.height * 2) {
			printf("===>%s:%d:  total_len(%d) error, must be %d\n",
			       __FUNCTION__, __LINE__, total_len, display_info.width * display_info.height * 2);
			return -1;
		}
		cr = cb + display_info.width * display_info.height / 2;
	} else {
		if (total_len != display_info.width * display_info.height * 3) {
			printf("===>%s:%d:  total_len(%d) error, must be %d\n",
			       __FUNCTION__, __LINE__, total_len, display_info.width * display_info.height * 3);
			return -1;
		}
		cr = cb + display_info.width * display_info.height;
	}

	yCbCr422_pack_to_planar(y, cb, cr, packed_frame, &display_info);

	display_info.planar = 1;     /* we are now planar */
	ret = preview_save_raw_file(planar_file, planar_frame, &display_info);

	free(planar_frame);

	return ret;
}

int convert_packed_2_planar(char *filename)
{
	FILE *packed_file;
	FILE *planar_file;
	char planar_fname[260] = { '\0' };
	int ret = 0;

	printf("convert %s from packed to planar......\n", filename);
	packed_file = fopen(filename, "r");
	if (!packed_file) {
		printf("open %s failed!!!\n", filename);
		return -1;
	}

	strcpy(planar_fname, filename);
	strcat(planar_fname, "_planar");
	planar_file = fopen(planar_fname, "w");
	if (!planar_file) {
		printf("open %s failed!!!\n", planar_fname);
		fclose(packed_file);
		return -1;
	}

	ret = __convert_packed_2_planar(packed_file, planar_file);

	fclose(planar_file);
	fclose(packed_file);

	return ret;
}

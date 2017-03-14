/*
 * Process frame buffer
 */
#include "headers.h"

void process_frame(void *handle, const char *filename, void *frame, struct display_info *dis, int ops)
{
	FILE *file;

	int width = dis->width;
	int height = dis->height;
	int format = dis->fmt.fourcc;

	__u8 *rgb_frame = NULL;

	file = fopen(filename, "w");
	if (!file) {
		fprintf(stderr, "--%s() L%d: open %s failed!!!\n", __func__, __LINE__, filename);
		return;
	}

	switch (ops) {
	case OPS_SAVE_BMP:
		//fixme
		rgb_frame = (__u8 *)malloc(width * height * 3);	//rgb888 bpp=24
		if (rgb_frame == NULL) {
			fprintf(stderr, "-->%s() L%d: malloc failed!\n", __func__, __LINE__);
			return;
		}
		convert_yuv_to_rgb24(frame, rgb_frame, dis);

		save_bgr_to_bmp(rgb_frame, dis, file);

		free(rgb_frame);

		break;
	case OPS_SAVE_JPG:
		//fixme
		if ( !dis->planar ) {
			if (DF_IS_YCbCr444(format))
				yuv444_packed_to_jpeg(frame, file, dis, 100);
			else if (DF_IS_YCbCr422(format))
			{
				printf("==>%s() L%d: yuv422!\n", __func__, __LINE__);
				yuv422_packed_to_jpeg(handle, frame, file, dis, 0);
			}
			else
				printf("==>%s() L%d: error format!\n", __func__, __LINE__);
		} else {
			if (DF_IS_YCbCr444(format))
				yuv444_sep_to_jpeg(frame, width, height, file, 100);
			else if (DF_IS_YCbCr422(format))
				yuv422_sep_to_jpeg(frame, width, height, file, 100);
			else if (DF_IS_YCbCr420(format))
				yuv420_sep_to_jpeg(frame, dis->mb, width, height, file, 100);
			else
				printf("==>%s() L%d: error format!\n", __func__, __LINE__);
		}

		break;
	case OPS_SAVE_RAW:
		//fixme
		preview_save_raw_file(file, frame, dis);
		break;
	default:
		fprintf(stderr, "-->%s() L%d: unsupported operation!\n", __func__, __LINE__);
		break;
	};

	fclose(file);
}



#include "headers.h"

static unsigned int fs_resize(int fmt, int size, int ratio, int ignore)
{
	if (DF_IS_YCbCr422(fmt)) {
		if ( ( (size/2) % (1<<ratio) ) && !ignore )
			return (((size/2) >> ratio) + 1) * 2;
		else
			return ((size/2) >> ratio) * 2;	
	} else {
		if ( ( size % (1<<ratio) ) && !ignore )
			return ((size >> ratio) + 1);
		else
			return (size >> ratio);	
	}
}

int cim_do_scale(int fd, int argc, char **argv, struct cim_scale *s, int format, 
			int img_width, int img_height, int cap_img_width, int cap_img_height)
{
	int ret = 0;
#if 0
	int i;
	printf("argc = %d\n", argc);
	for (i = 0; i < argc; i++)
		printf("argv[%d] = %s\n", i, argv[i]);
#endif
	// cimtest5 -S 1 1 1 -b -C -m 320 -n 240 -w 320 -h 240
	s->hs = htoi(argv[argc-3]);
	s->vs = htoi(argv[argc-2]);
	s->ignore = htoi(argv[argc-1]);

	//resize after downscale
	if (DF_IS_YCbCr422(format)) {
		img_width &= ~0x1;	//must be even for yuv422 scale
		cap_img_width &= ~0x1;
	}
	s->pre_w = fs_resize(format, img_width, s->hs, s->ignore);
	s->cap_w = fs_resize(format, cap_img_width, s->hs, s->ignore);
	s->pre_h = fs_resize(format, img_height, s->vs, 1);
	s->cap_h = fs_resize(format, cap_img_height, s->vs, 1);

	printf("\n==>%s %s() L%d: hs=%d, vs=%d, ignore=%d\n\n", __FILE__, __func__, __LINE__, s->hs, s->vs, s->ignore);
	printf("\n==>%s %s() L%d: pre_w=%d, pre_h=%d, cap_w=%d, cap_h=%d\n\n", __FILE__, __func__, __LINE__, s->pre_w, s->pre_h, s->cap_w, s->cap_h);
	sleep(1);

	ret = ioctl(fd, VIDIOC_CIM_SCALE, s);

	if (ret < 0) {
		printf("==>%s %s() L%d: ioctl fail!\n", __FILE__, __func__, __LINE__);
		perror(strerror(errno));
	}

	return ret;
}

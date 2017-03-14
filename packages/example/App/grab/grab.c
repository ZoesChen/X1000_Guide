#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "v4l2uvc.h"
#include "camera.h"
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include <string.h>

//#define DO_DCT_MATCH


extern long long  select_zong;
#ifdef DO_DCT_MATCH
typedef unsigned long long ulong64;

extern int ph_dct_imagehash_4c(const char* file,ulong64 *hash);
extern int ph_hamming_distance(const ulong64 hash1,const ulong64 hash2);

ulong64 test_count = 0ULL;
ulong64 phash_first = 0ULL;

int image_similarity1(const char *filename) {
	ulong64 tmphash;
	int distance = -1;

	if (ph_dct_imagehash_4c(filename, &tmphash) < 0) {
		printf("===>%s: ph_dct_imagehash failed\n", filename);
		return -1;
	}

	distance = ph_hamming_distance(phash_first, tmphash);

#if 0
	printf("%llu --- %llu:  dist = %d similarity=%f%%\n",
	       phash_first, tmphash,
	       distance,  (1 - distance / 64.0) * 100);
#endif

	if ( (1 - distance / 64.0) > 0.9) {
		return 1;
	}

	return -1;
}

int image_similarity2(const char *filename1, const char *filename2) {
	ulong64 tmphash1;
	ulong64 tmphash2;
	int distance = -1;

	if (ph_dct_imagehash_4c(filename1, &tmphash1) < 0) {
		printf("===>%s: ph_dct_imagehash failed\n", filename1);
		return -1;
	}

	if (ph_dct_imagehash_4c(filename2, &tmphash2) < 0) {
		printf("===>%s: ph_dct_imagehash failed\n", filename2);
		return -1;
	}

	distance = ph_hamming_distance(tmphash1, tmphash2);

	printf("%llu --- %llu:  dist = %d similarity=%f%%\n",
	       tmphash1, tmphash2,
	       distance,  (1 - distance / 64.0) * 100);

	if ( (1 - distance / 64.0) > 0.9) {
		return 1;
	}

	return 0;
}
#endif	/* DO_DCT_MATCH */

#define DEFAULT_FPS	15

static struct option long_options[] = {
	{ "help", 0, 0, 'H'},
	{ "print_formats", 0, 0, 'F' },
	{ "print_controls", 0, 0, 'C' },
	{ "device", 1, 0, 'd'},
	{ "width", 1, 0, 'w' },
	{ "height", 1, 0, 'h' },
	{ "count", 1, 0, 'c'},
	{ "rate", 1, 0, 'r' },
	{ "yuv", 0, 0, 'y' },
	{ "match", 0, 0, 'm' },
	{ "perf", 0, 0, 'p' },

	{0, 0, 0, 0}
};

static char optstring[] = "HFCd:w:h:c:r:yt:mp";

void usage(void) {
	printf("usage: uvcview [-d <device>] [-c <count>]\n");
	printf("--help -H\t\tprint this message \n");
	printf("--print_formats -F\t\tprint video device info\n");
	printf("--device -d\t\t, video device, default is /dev/video0\n");
	printf("--width -w\t\tgrab width\n");
	printf("--height -h\t\tgrab height\n");
	printf("--count -c\t\tset the count to grab\n");
	printf("--rate -r\t\tframe sample rate(fps)\n");
	printf("--yuv -y\t\tuse yuyv input format\n");
	printf("--timeout -t\t\tselect timeout\n");
	printf("--match -m\t\tdo picture match test\n");
	printf("--perf -p\t\tdo performance test\n");
}

long long usectime(void)
{
	struct timeval t;
	gettimeofday(&t,NULL);

	return (((long long) t.tv_sec) * 1000000LL) +
		((long long) t.tv_usec);
}


int main(int argc, char **argv)
{
	struct vdIn videoIn0;
        char name[50];
        int times = 0;
        int fcount = 1;
	int forever = 0;
        int width = 640, height = 480;
	int print_formats = 0;
	int print_controls = 0;
	char *device = "/dev/video0";
	int fps = DEFAULT_FPS;
	int input_yuv = 0;
	int timeout = 20;
	int do_match = 0;
	int do_perf = 0;
	char c;

	while (1) {
		c = getopt_long(argc, argv, optstring, long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'H':
			usage();
			return 0;

		case 'F':
			print_formats = 1;
			break;

		case 'C':
			print_controls = 1;
			break;

		case 'd':
			device = optarg;
			printf("use device %s\n", device);
			break;

		case 'c':
			fcount = atoi(optarg);
			printf("frame count = %d\n", fcount);
			break;

		case 'w':
			width = atoi(optarg);
			break;

		case 'h':
			height = atoi(optarg);
			break;

		case 'r':
			fps = atoi(optarg);
			break;

		case 'y':
			input_yuv = 1;
			break;

		case 't':
			timeout=atoi(optarg);
			break;

		case 'p':
			do_perf = 1;
			break;

		case 'm':
			do_match = 1;
			break;

		default:
			/* do nothing */
			usage();
			return 127;
		}
	}

	fprintf(stderr, "Waiting for device...\n");
	while (1) {
		struct stat buf;
		if (stat(device, &buf) == 0)
			break;

		sleep (1);
	}

	memset(&videoIn0, 0, sizeof(struct vdIn));

	if (print_formats)
		return query_camera_formats(&videoIn0, device);

	printf("width = %d, height = %d\n", width, height);
	printf("input_yuv = %d\n",input_yuv);

	if (!input_yuv)
		init_camera(&videoIn0, device, width, height, V4L2_PIX_FMT_MJPEG, fps, 1);
	else
		init_camera(&videoIn0, device, width, height, V4L2_PIX_FMT_YUYV, fps, 1);
	if (print_controls){
		printf("ttt\n");
		enum_controls(videoIn0.fd);
	}

	if (fcount == 0)
		forever = 1;

	fprintf(stderr, "running......\n");

	times = 0;

	printf("forever = %d,fcount=%d\n",forever,fcount);
	if(fcount == -1)
		fcount =10;

	while ( forever || (fcount--) ) {
		unsigned long long starttime,endtime;
		sprintf(name, "p-%u.jpg", times++);

		if (grab(&videoIn0, name, timeout, do_perf, times) < 0) {
			printf("grab break\n");
			exit(EXIT_FAILURE);
			break;
		}

	}
#if 0
	if (do_perf) {
		int i = 0;
		struct timeval *tv;

		for (i = 0; i < times; i++) {
			tv = videoIn0.perf_timestamp + i;
			printf("[%d] %d:%d\n", i, tv->tv_sec, tv->tv_usec);
		}
	}
#endif
 out:
        close_camera(&videoIn0);

	return 0;
}

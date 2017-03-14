#include <stdio.h>

#include "headers.h"
#include "jpeg.h"

#define DEFAULT_WIDTH		320
#define DEFAULT_HEIGHT		240
#define PREVIEW_FPS		30

int debug_level = 0;

static struct display_info disp_info;
static struct fb_info fb_info;
unsigned long long starttime,endtime;
int count = 0;
static char video_dev_name[16];
const char *fb_dev_name = "/dev/fb0";
void *jz_jpeg = NULL;

static int fd_cim = -1;

long long usectime(void)
{
	struct timeval t;
	gettimeofday(&t,NULL);

	return (((long long) t.tv_sec) * 1000000LL) +
		((long long) t.tv_usec);
}


static struct option long_options[] = {
	{ "preview", 0, 0, 'P'}, /* 0 */
	{ "capture", 0, 0, 'C'},
	{ "testing", 0, 0, 'T'},
	{ "scale", 0, 0, 'S'},
	{ "verbose", 0, 0, 'V'},
	{ "use_ipu", 0, 0, 'i'},
	{ "preview_width", 1, 0, 'w'},
	{ "preview_height", 1, 0, 'h'}, /* 10 */
	{ "preview_bpp", 1, 0, 'b' },
	{ "capture_width", 1, 0, 'x'},
	{ "capture_height", 1, 0, 'y'},
	{ "capture_bpp", 1, 0, 'z' },
	{ "save_raw", 0, 0, 's'},
	{ "display_raw", 0, 0, 'd'}, /* 15 */
	{ "file", 1, 0, 'f'},
	{ "planar", 0, 0, 'p'},
	{ "only_y", 0, 0, 'g'},	// grey
	{ "format", 1, 0, 't'},
	{ "cim_id", 1, 0, 'I'},
	{ "packing", 1, 0, 'k'},
	{ "macro_block", 0, 0, 'M'},
	{ "virtual_memory", 0, 0, 'v'},
	{"save picture and preview to fb", 0, 0, 'l'},
	{0, 0, 0, 0}
};

static char optstring[] = "PCSTVit:w:h:x:y:sf:pb:z:I:k:Mvl:";

#define print_opt_help(opt_index, help_str)				\
	do {								\
		printf("\t--%s\t\t-%c\t%s", long_options[opt_index].name, (char)long_options[opt_index].val, help_str); \
	} while (0)

void usage() {
	printf("\nUsage:\n");
	print_opt_help(0, "preview on lcd panel\n");
	print_opt_help(1, "capture in JPEG or BMP format\n");
	print_opt_help(2, "testing cim reg w/r\n");
	print_opt_help(3, "down scale\n");
	print_opt_help(4, "verbose mode\n");
	print_opt_help(5, "use IPU or not\n");
	print_opt_help(6, "preview width, multiple of 4\n");
	print_opt_help(7, "preview height, multiple of 4\n");
	print_opt_help(8, "preview bpp\n");
	print_opt_help(9, "capture width, multiple of 4\n");
	print_opt_help(10, "capture height, multiple of 4\n");
	print_opt_help(11, "capture bpp\n");
	print_opt_help(12, "save raw file\n");
	print_opt_help(13, "display raw file\n");
	print_opt_help(14, "filename\n");
	print_opt_help(15, "planar mode (separate, not package)\n");
	print_opt_help(16, "grey, ignore u & v\n");
	print_opt_help(17, "pixel format\n");
	print_opt_help(18, "cim id (0 or 1)\n");
	print_opt_help(19, "packing mode (0 ~ 7)\n");
	print_opt_help(20, "macro block mode for yuv420\n");
	print_opt_help(21, "using virtual memory based on internal tlb\n");
	print_opt_help(22, "save picture and preview to fb\n");

	printf("\n\nExamples:\n");
	printf("\t./cimtest5 -I 0 -P -w 320 -h 240\t---\tpreview in 320x240\n");
	printf("\t./cimtest5 -I 0 -P -w 320 -h 240 -C -x 640 -y 480\t---\tpreview in 320x240, capture in 640x480\n");
}

void do_cleanup(int do_test, int ret)
{
	stop_capturing();
	if(disp_info.userptr)
		jz_jpeg_encode_deinit(jz_jpeg);
	uninit_device();	//cleanup video buf
	if (fd_cim > 0)
		close(fd_cim);
	finish_framebuffer(&fb_info);
	if (disp_info.ybuf) {
		free(disp_info.ybuf);
		free(disp_info.ubuf);
		free(disp_info.vbuf);
	}

	if (ret)
		exit(0);
}

void sig_int(int signo)
{
	printf("==>%s %s() L%d\n", __FILE__, __func__, __LINE__);
	do_cleanup(0, 1);
}

__u32 cim_yuv422_formats[8] = {
	YCbCr422_CrY1CbY0,	//CIMCFG.PACK[6:4]=0
	YCbCr422_Y0CrY1Cb,	//CIMCFG.PACK[6:4]=1
	YCbCr422_CbY0CrY1,	//CIMCFG.PACK[6:4]=2
	YCbCr422_Y1CbY0Cr,	//CIMCFG.PACK[6:4]=3
	YCbCr422_Y0CbY1Cr,	//CIMCFG.PACK[6:4]=4
	YCbCr422_CrY0CbY1,	//CIMCFG.PACK[6:4]=5
	YCbCr422_Y1CrY0Cb,	//CIMCFG.PACK[6:4]=6
	YCbCr422_CbY1CrY0,	//CIMCFG.PACK[6:4]=7
};


int do_preview = 0;
int do_capture = 0;
enum options ops = OPS_PREVIEW;
char filename[64];
int raw_counter = 0;
int raw_limit = 3;

static int handle_bufs(void *tmp_buf)
{
	int i;
        if(do_preview)
	{
		display_to_fb(tmp_buf, &disp_info, &fb_info);	//preview
//	        display_direct_to_fb(tmp_buf, &disp_info, &fb_info);	//preview
	}
	if(do_capture)
	{
		if (raw_counter == raw_limit ) {
			switch (ops) {
			case OPS_SAVE_BMP :
			case OPS_SAVE_JPG :
			case OPS_SAVE_RAW :
				process_frame(jz_jpeg, filename, tmp_buf, &disp_info, ops);
	                        printf("picture taked!!!!!!!!\n");
				break;
			default :
				printf("Please set picture 's format \n");
	                        printf("picture don't taked!!!!!!!!\n");
			}
		}
	}
}




int main(int argc, char *argv[])
{
	int cim_id = 0;
	int do_test = 0;
	int do_scale = 0;
	int save_raw = 0;
	int display_raw = 0;
	int only_y = 0;
	int save_preview = 0;
	//preview resolution
	int do_display = 0;
	int pre_width = DEFAULT_WIDTH;
	int pre_height= DEFAULT_HEIGHT;
	int pre_bpp = 16;
	//capture resolution
	int cap_width = DEFAULT_WIDTH;
	int cap_height = DEFAULT_HEIGHT;
	int cap_bpp = 16;

	int packing = 4;	/* CIMCFG.PACK */
	char sep_mb = 0;	/* CIMCFG.SEP & CIMCR.MBEN */
	int rate = 15;		/* default to 15fps  */

	int format_specified = 0;
	char temp_str[16];
	char *suffix = temp_str;

	void *frame = NULL;

	int c = 0;
	int ret = 0;

	memset(filename, 0, sizeof(filename));
	signal(SIGINT, sig_int);

	disp_info.userptr = 0;	/* don't use internal userptr */
	disp_info.planar = 0;	/* planar mode */
	disp_info.mb = 0;	/* macro block (tile) mode for yuv420 */

	while (1) {
		c = getopt_long(argc, argv, optstring, long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'P':
			do_preview = 1;
			do_display = 1;		//display as default
			printf("do preview ...\n");
			break;
		case 'C':
			do_capture = 1;
			printf("do capture......\n");
			break;
		case 'S':
			do_scale = 1;
			printf("do scale ......\n");
			break;
		case 'T':
			do_test = 1;
			printf("just do test ......\n");
			break;
		case 'V':
			debug_level = 8;
			printf("verbose on ......\n");
			break;
		case 't':
			disp_info.fmt.format = str_to_fmt(optarg);
			format_specified = 1;
			printf("format = %s(%#0x)\n", optarg, disp_info.fmt.format);
			break;
		case 'i':
			disp_info.ipu = 1;
			printf("use ipu ......\n");
			break;
		case 'w':
			pre_width = atoi(optarg);
			printf("preview width = %d\n", pre_width);
			break;
		case 'h':
			pre_height = atoi(optarg);
			printf("preview height = %d\n", pre_height);
			break;
		case 'b':
			pre_bpp = atoi(optarg);
			printf("preview bpp = %d\n", pre_bpp);
			break;
		case 'x':
			cap_width = atoi(optarg);
			printf("capture width = %d\n", cap_width);
			break;
		case 'y':
			cap_height = atoi(optarg);
			printf("capture height = %d\n", cap_height);
			break;
		case 'z':
			cap_bpp = atoi(optarg);
			printf("capture bpp = %d\n", cap_bpp);
			break;
		case 's':
			save_raw = 1;
			printf("save raw ......\n");
			break;
		case 'f':
			if (strlen(optarg) > 63) {
				printf("file name too long!\n");
				usage();
				return -1;
			}
			strncpy(filename, optarg, strlen(optarg));
			break;
		case 'p':
			disp_info.planar = 1;
			printf("planar mode ......\n");
			break;
		case 'I':
			cim_id = atoi(optarg);
			printf("using cim%d ......\n", cim_id);
			break;
		case 'k':
			packing = atoi(optarg);
			printf("set CIMCFG.PACK to %d ......\n", packing);
			break;
		case 'M':
			disp_info.mb = 1;
			printf("using macro block ......\n");
			break;
		case 'v':
			disp_info.userptr = 1;
			save_jpeg_userptr = 1;     //for choosing  compress pictures model  in savepage.c
			                           //the variable is defined in savepage.c
			printf("using internal userptr ......\n");
			break;
		case 'l':
			raw_limit = atoi(optarg);
			printf("raw_limit = %d\n", raw_limit);
			break;
		default:
			usage();
			return 127;
			;
		}
	}

	/* 1. VPU initialize */
	printf("VPU initialize\n");
	jz_jpeg = jz_jpeg_encode_init(cap_width, cap_height);
	/*
	 * Set Format
	 */
	if (format_specified) {
		switch (disp_info.fmt.format) {
		case DISP_FMT_YCbCr444:
			disp_info.fmt.format = YCbCr444_YUV;	//CIMCFG.PACK[6:4]=4
			disp_info.fmt.fourcc = V4L2_PIX_FMT_YUV444;
			packing = 4;	/* CIMCFG.PACK */
			break;
		case DISP_FMT_YCbCr422:
			disp_info.fmt.format = cim_yuv422_formats[packing];
			if (disp_info.planar)
				disp_info.fmt.fourcc = V4L2_PIX_FMT_YUV422P;	//planar mode
			else
				disp_info.fmt.fourcc = V4L2_PIX_FMT_YUYV;
			break;
		case DISP_FMT_YCbCr420:
			disp_info.fmt.format = YCbCr420_FMT;
			if (disp_info.mb)
				disp_info.fmt.fourcc = V4L2_PIX_FMT_NV12;	//tile mode
			else
				disp_info.fmt.fourcc = V4L2_PIX_FMT_YUV420;
			disp_info.planar = 1;	/* always planar mode for yuv 420 */
			break;
		case DISP_FMT_ITU656P:
			disp_info.fmt.format = ITU656P_YCbCr422_CbY0CrY1;	//itu656 progressive 8-bit ycbcr422
			disp_info.fmt.fourcc = V4L2_PIX_FMT_YUYV;
			break;
		case DISP_FMT_ITU656I:
			disp_info.fmt.format = ITU656I_YCbCr422_CbY0CrY1;	//itu656 interlace 8-bit ycbcr422
			disp_info.fmt.fourcc = V4L2_PIX_FMT_YUYV;
			break;
		default:
			printf("==>%s L%d: request other format, maybe bypass is ok!\n", __func__, __LINE__);
			break;
		}
	} else {
		// yuv422 as default
		disp_info.fmt.format = cim_yuv422_formats[packing];
		disp_info.fmt.fourcc = V4L2_PIX_FMT_YUYV;
	}

	if (disp_info.mb)
		sep_mb = V4L2_FRM_FMT_TILE;	//CIMCR.MBEN=1 && CIMCFG.SEP=1
	else if (disp_info.planar)
		sep_mb = V4L2_FRM_FMT_PLANAR;	//CIMCFG.SEP=1
	else
		sep_mb = V4L2_FRM_FMT_PACK;	//CIMCFG.SEP=0

	disp_info.fmt.fmt_priv = cim_set_fmt_priv(0, 0, packing, sep_mb);

	if (filename[0]== '\0') {

		if (save_raw || display_raw)
			strcpy(filename, "test.raw");
		else
			strcpy(filename, "test.jpg");

	}

	suffix = strchr(filename, '.');
	if (! strcmp(suffix, ".bmp"))
		ops = OPS_SAVE_BMP;
	else if (! strcmp(suffix, ".jpg"))
		ops = OPS_SAVE_JPG;
	else if (! strcmp(suffix, ".raw"))
		ops = OPS_SAVE_RAW;

	strcpy(fb_info.dev_name, fb_dev_name);
	if (do_display)
		init_framebuffer(&fb_info);


	if ((cim_id != 0) && (cim_id != 1)) {
		printf("==>%s L%d: invalid cim id (%d)\n", __func__, __LINE__, cim_id);
		goto out;
	}
	snprintf(video_dev_name, 16, "/dev/video%d", cim_id);
	printf("==>%s L%d: video dev name = %s\n", __func__, __LINE__, video_dev_name);
	fd_cim = open_device(video_dev_name);
	if (fd_cim < 0) {
		printf("==>%s L%d: fail to open %s\n", __func__, __LINE__, video_dev_name);
		goto out;
	}

	if (do_test) {
		printf("==>%s %s() L%d\n", __FILE__, __func__, __LINE__);

		ret = cim_do_test(fd_cim, argc, argv);
		goto out;
	}


	if (do_preview)
	{
		disp_info.width = pre_width;
		disp_info.height = pre_height;
		disp_info.bpp = pre_bpp;

		init_device(&disp_info);
		start_capturing();

		frame_handler_init(handle_bufs);
		while (1) {
			process_framebuf();

		}
	}

	if (do_capture)
	{
		/*
		 * Note: capture resolution may be different with preview, so we've to set format again!!
		 */
#if 0
		if(!disp_info.userptr)
		{
                    printf("Please choose userptr model !!!!!!!!\n");
		    goto out;
		}
#endif
		disp_info.width = cap_width;
		disp_info.height = cap_height;
		disp_info.bpp = cap_bpp;

		init_device(&disp_info);
		start_capturing();		// VIDIOC_STREAMON

		frame_handler_init(handle_bufs);
		for ( raw_counter = 0 ; raw_counter <=  raw_limit ; raw_counter ++ )
		{
			process_framebuf();
                }
	}
	stop_capturing();
	putchar('\n');
	putchar('\n');
	printf("----------------------CIM TEST END -----------------------\n");
out:
	/* 13. Close vpu */
	jz_jpeg_encode_deinit(jz_jpeg);
//	uninit_device();	//cleanup video buf
//	jz47_free_alloc_mem();
//	do_cleanup(do_test, 0);
	return ret;
}

/*
 * V4L2 Spec Appendix B.
 *
 * Video Capture Example
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>	//getopt_long()

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>	//for videodev2.h

#include <linux/videodev2.h>

#include "headers.h"

#define CLEAR(x)	memset(&(x), 0, sizeof(x))

#if 0
#define DBG()	fprintf(stdout, "**>%s L%d\n", __func__, __LINE__)
#else
#define DBG()	do {} while(0)
#endif

typedef enum {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
} io_method;

static char *io_method_str[3] = {
	"IO_METHOD_READ",
	"IO_METHOD_MMAP",
	"IO_METHOD_USERPTR",
};

struct buffer {
	void *	start;
	size_t	length;
};

static char *		dev_name	= NULL;
static io_method	io		= IO_METHOD_MMAP;
static int		fd		= -1;
struct buffer *		buffers		= NULL;
static unsigned int	n_buffers	= 0;

typedef  int (*hand_t)(void *buf);
static hand_t frame_handler;
static void errno_exit(const char *s)
{
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

static int xioctl(int fd, int request, void *arg)
{
	int r;

	do {
		r = ioctl(fd, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}

static int read_frame(void )
{
	struct v4l2_buffer buf;
	unsigned int i;

	switch (io) {
	case IO_METHOD_READ:
		if (-1 == read(fd, buffers[0].start, buffers[0].length)) {
			switch (errno) {
			case EAGAIN:
				return 0;
			case EIO:
			default:
				errno_exit("read");
			}
		}
//		*frame = (void *)buffers[0].start;
		break;

	case IO_METHOD_MMAP:
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

		if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;
			case EIO:
			default:
				errno_exit("VIDIOC_DQBUF");
			}
		}

		assert(buf.index < n_buffers);
//		*frame = (void *)buffers[buf.index].start;
                if(frame_handler) {
	                    frame_handler((void *)buffers[buf.index].start);
		}

		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");

		break;

	case IO_METHOD_USERPTR:
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;

		if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;
			case EIO:
			default:
				errno_exit("VIDIOC_DQBUF");
			}
		}
	        int page_size = getpagesize();
	        int buf_size = (buf.length  + page_size - 1) & ~(page_size - 1);
		for (i=0; i<n_buffers; ++i) {
			if (buf.m.userptr == (unsigned long)buffers[i].start
			 && buf_size == buffers[i].length){
				break;
			}
		}

//		*frame = (void *)buf.m.userptr;

                if(frame_handler) {
	                    frame_handler((void *)buffers[buf.index].start);
		}

		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");

		break;
	}

	return 1;
}

static void mainloop(void)
{
	unsigned int count;

	count = 1;

	while (count-- > 0) {
	//	fprintf(stdout, "-->%s L%d: count=%d\n", __func__, __LINE__, count);

		for (;;) {
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			/* Timeout. */
			tv.tv_sec = 20;
			tv.tv_usec = 0;

			r = select(fd+1, &fds, NULL, NULL, &tv);

			if (-1 == r) {
				if (EINTR == errno)
					continue;

				errno_exit("select");
			}

			if (0 == r) {
				fprintf(stderr, "select timeout\n");
				exit(EXIT_FAILURE);
			}

			if (read_frame())
				break;

			/* EAGAIN - continue select loop. */
		}
	}
}

void stop_capturing(void)
{
	enum v4l2_buf_type type;

	switch (io) {
	case IO_METHOD_READ:
		/* nothing to do */
		break;
	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
			errno_exit("VIDIOC_STREAMOFF");

		break;
	}
}

void start_capturing(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	switch(io) {
	case IO_METHOD_READ:
		/* nothing to do */
		break;
	case IO_METHOD_MMAP:
		for (i=0; i<n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);

			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");

		break;

	case IO_METHOD_USERPTR:
		for (i=0; i<n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);

			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_USERPTR;
			buf.index = i;
			buf.m.userptr = (unsigned long)buffers[i].start;
			buf.length = buffers[i].length;

//			memset((void *)buf.m.userptr, 0xff, buf.length);	//force OS to allocate physical memory
//			if (-1 == xioctl(fd, VIDIOC_CIM_S_MEM, &buf))		//by ylyuan
//				errno_exit("VIDIOC_CIM_S_MEM");

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");

		break;
	}
}


static void init_read(unsigned int buffer_size)
{
	buffers = calloc(1, sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	buffers[0].length = buffer_size;
	buffers[0].start = malloc(buffer_size);

	if (!buffers[0].start) {
		fprintf(stderr, "Out of memory2\n");
		exit(EXIT_FAILURE);
	}
}

static void init_mmap(void)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count	= 2;
	req.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory	= V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support memory mapping\n", dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 1) {
		fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name);
		exit(EXIT_FAILURE);
	}

	buffers = malloc(req.count * sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers=0; n_buffers<req.count; ++n_buffers) {
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		buf.index	= n_buffers;

		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
			errno_exit("VIDIOC_QUERYBUF");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start =
			mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start)
			errno_exit("mmap");
	}
}

static void init_userp(unsigned int buffer_size)
{
	struct v4l2_requestbuffers req;
	unsigned int page_size = 0;

	page_size = getpagesize();
	buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

	CLEAR(req);

	req.count	= 2;
	req.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory	= V4L2_MEMORY_USERPTR;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support user pointer i/o\n", dev_name);
			exit(EXIT_FAILURE);
		} else
			errno_exit("VIDIOC_REQBUFS");
	}

	buffers = calloc(req.count, sizeof(*buffers));
	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		buffers[n_buffers].length = buffer_size;
		buffers[n_buffers].start = (unsigned char *)JZMalloc(128, buffer_size);

		printf("==>%s L%d: the buffers[%d].start = %x\n", __func__, __LINE__ , n_buffers,buffers[n_buffers].start);
		if (!buffers[n_buffers].start) {
			fprintf(stderr, "Out of memory\n");
			exit(EXIT_FAILURE);
		}
	}
}

static void query_cap(struct v4l2_capability *cap)
{
	if (cap->capabilities & V4L2_CAP_VIDEO_CAPTURE)
		fprintf(stdout, "Video capture device\n");
	if (cap->capabilities & V4L2_CAP_READWRITE)
		fprintf(stdout, "Read/Write systemcalls\n");
	if (cap->capabilities & V4L2_CAP_STREAMING)
		fprintf(stdout, "Streaming I/O ioctls\n");
}

static void dump_fmt(struct v4l2_format *fmt)
{
	if (fmt->type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		fprintf(stdout, "width=%d, height=%d\n", fmt->fmt.pix.width, fmt->fmt.pix.height);
		fprintf(stdout, "pixelformat=%s, field=%d\n", (char *)&fmt->fmt.pix.pixelformat, fmt->fmt.pix.field);
		fprintf(stdout, "bytesperline=%d, sizeimage=%d\n", fmt->fmt.pix.bytesperline, fmt->fmt.pix.sizeimage);
		fprintf(stdout, "colorspace=%d\n", fmt->fmt.pix.colorspace);
	}
}

int open_device(const char *dev_name)
{
	struct stat st;

	if (-1 == stat(dev_name, &st)) {
		fprintf(stderr, "Cannot identify '%s':%d, %s\n", dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "%s is not char device\n", dev_name);
		exit(EXIT_FAILURE);
	}

	fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if (-1 == fd) {
		fprintf(stderr, "Cannot open '%s':%d, %s\n", dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return fd;
}

unsigned int set_format(struct display_info *disp)
{
	struct v4l2_format fmt;
	unsigned int min;

	CLEAR(fmt);

	fmt.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
//	fmt.fmt.pix.width	= 320;
//	fmt.fmt.pix.height	= 240;
	fmt.fmt.pix.width	= disp->width;
	fmt.fmt.pix.height	= disp->height;
//	fmt.fmt.pix.pixelformat	= V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.pixelformat	= disp->fmt.fourcc;
	fmt.fmt.pix.field	= V4L2_FIELD_NONE;//V4L2_FIELD_INTERLACED;
	fmt.fmt.pix.priv	= disp->fmt.fmt_priv;


	if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
	{
	        printf("==>%s L%d: set error  !!!!!!!!!!!!! \n",__func__, __LINE__);
		errno_exit("VIDIOC_S_FMT");
        }

        xioctl(fd, VIDIOC_G_FMT, &fmt);
	if((disp->width == fmt.fmt.pix.width)&&(disp->height == fmt.fmt.pix.height))
	{
	}
        else
	{
                disp->width = fmt.fmt.pix.width;
		disp->height = fmt.fmt.pix.height;
	}

	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;

	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

//	dump_fmt(&fmt);
	printf("==>%s L%d: sizeimage=0x%x fmt.fmt.pix.width = 0x%x fmt.fmt.pix.height = 0x%x\n", __func__, __LINE__, fmt.fmt.pix.sizeimage,fmt.fmt.pix.width, fmt.fmt.pix.height);

	return fmt.fmt.pix.sizeimage;
}

void init_device(struct display_info *disp)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	unsigned int frmsize;

	if (disp->userptr == 1)
		io = IO_METHOD_USERPTR;		// user pointer I/O
	else
		io = IO_METHOD_MMAP;		// map device memory to userspace

	printf("==>%s L%d: io method: %s\n", __func__, __LINE__, io_method_str[io]);

	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is not V4L2 device\n", dev_name);
			exit(EXIT_FAILURE);
		} else
			errno_exit("VIDIOC_QUERYCAP");
	}

//	query_cap(&cap);

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is not capture device\n", dev_name);
		exit(EXIT_FAILURE);
	}

	switch (io) {
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			fprintf(stderr, "%s doesn't support read i/o\n", dev_name);
			exit(EXIT_FAILURE);
		}
		break;
	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			fprintf(stderr, "%s doesn't support streaming i/o\n", dev_name);
			exit(EXIT_FAILURE);
		}
		break;
	}

	/* select video input, video standard and tune here. */

	CLEAR(cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect;	/* reset to default */

		if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
			switch(errno) {
			case EINVAL:
				/* Cropping not supported */
				break;
			default:
				/* Errors ignored */
				break;
			}
		}
	} else {
		/* Errors ignored */
	}

	frmsize = set_format(disp);

	switch (io) {
	case IO_METHOD_READ:
		init_read(frmsize);
		break;
	case IO_METHOD_MMAP:
		init_mmap();
		break;
	case IO_METHOD_USERPTR:
		init_userp(frmsize);
		break;
	}
}

void uninit_device(void)
{
	unsigned int i;

	switch(io) {
	case IO_METHOD_READ:
		free(buffers[0].start);
		break;
	case IO_METHOD_MMAP:
		for (i=0; i<n_buffers; ++i)
			if (-1 == munmap(buffers[i].start, buffers[i].length))
				errno_exit("munmap");
		break;
	case IO_METHOD_USERPTR:
		for (i=0; i<n_buffers; ++i)
		{
//			printf("uninit_device i = %d \n  n_buffers = %d buffers[%d].start = 0x%x\n", i , n_buffers, i, buffers[i].start);
//	                jz47_free_alloc_mem();
		}
		break;
	}

	free(buffers);
}

int process_framebuf(void)
{

	mainloop();

	return 0;
}


void frame_handler_init(void *handle)
{
        frame_handler = handle;
}

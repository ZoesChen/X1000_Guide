/*******************************************************************************
#	 	uvcview: Sdl video Usb Video Class grabber           .         #
#This package work with the Logitech UVC based webcams with the mjpeg feature. #
#All the decoding is in user space with the embedded jpeg decoder              #
#.                                                                             #
# 		Copyright (C) 2005 2006 Laurent Pinchart &&  Michel Xhaard     #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "videodev.h"
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "v4l2uvc.h"
//#include "utils.h"

static int debug = 0;
long long  select_zong = 0;


static int init_v4l2(struct vdIn *vd);

int enum_frame_intervals(int dev, __u32 pixfmt, __u32 width, __u32 height)
{
	int ret;
	struct v4l2_frmivalenum fival;

	memset(&fival, 0, sizeof(fival));
	fival.index = 0;
	fival.pixel_format = pixfmt;
	fival.width = width;
	fival.height = height;
	printf("\tTime interval between frame: ");
	while ((ret = ioctl(dev, VIDIOC_ENUM_FRAMEINTERVALS, &fival)) == 0) {
		if (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
			printf("%u/%u, ",
			       fival.discrete.numerator, fival.discrete.denominator);
		} else if (fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
			printf("{min { %u/%u } .. max { %u/%u } }, ",
			       fival.stepwise.min.numerator, fival.stepwise.min.numerator,
			       fival.stepwise.max.denominator, fival.stepwise.max.denominator);
			break;
		} else if (fival.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
			printf("{min { %u/%u } .. max { %u/%u } / "
			       "stepsize { %u/%u } }, ",
			       fival.stepwise.min.numerator, fival.stepwise.min.denominator,
			       fival.stepwise.max.numerator, fival.stepwise.max.denominator,
			       fival.stepwise.step.numerator, fival.stepwise.step.denominator);
			break;
		}
		fival.index++;
	}
	printf("\n");
	if (ret != 0 && errno != EINVAL) {
		printf("ERROR enumerating frame intervals: %d\n", errno);
		return errno;
	}

	return 0;
}
int enum_frame_sizes(int dev, __u32 pixfmt)
{
	int ret;
	struct v4l2_frmsizeenum fsize;

	memset(&fsize, 0, sizeof(fsize));
	fsize.index = 0;
	fsize.pixel_format = pixfmt;
	while ((ret = ioctl(dev, VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0) {
		if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
			printf("{ discrete: width = %u, height = %u }\n",
			       fsize.discrete.width, fsize.discrete.height);
			ret = enum_frame_intervals(dev, pixfmt,
						   fsize.discrete.width, fsize.discrete.height);
			if (ret != 0)
				printf("  Unable to enumerate frame sizes.\n");
		} else if (fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
			printf("{ continuous: min { width = %u, height = %u } .. "
			       "max { width = %u, height = %u } }\n",
			       fsize.stepwise.min_width, fsize.stepwise.min_height,
			       fsize.stepwise.max_width, fsize.stepwise.max_height);
			printf("  Refusing to enumerate frame intervals.\n");
			break;
		} else if (fsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
			printf("{ stepwise: min { width = %u, height = %u } .. "
			       "max { width = %u, height = %u } / "
			       "stepsize { width = %u, height = %u } }\n",
			       fsize.stepwise.min_width, fsize.stepwise.min_height,
			       fsize.stepwise.max_width, fsize.stepwise.max_height,
			       fsize.stepwise.step_width, fsize.stepwise.step_height);
			printf("  Refusing to enumerate frame intervals.\n");
			break;
		}
		fsize.index++;
	}

	if (ret != 0 && errno != EINVAL) {
		printf("ERROR enumerating frame sizes: %d\n", errno);
		return errno;
	}

	return 0;
}

int enum_frame_formats(int dev)
{
	int ret;
	struct v4l2_fmtdesc fmt;

	memset(&fmt, 0, sizeof(fmt));
	fmt.index = 0;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while ((ret = ioctl(dev, VIDIOC_ENUM_FMT, &fmt)) == 0) {
		fmt.index++;
		printf("{ pixelformat = '%c%c%c%c', description = '%s' }\n",
		       fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF,
		       (fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,
		       fmt.description);
		ret = enum_frame_sizes(dev, fmt.pixelformat);
		if (ret != 0)
			printf("  Unable to enumerate frame sizes.\n");
	}
	if (errno != EINVAL) {
		printf("ERROR enumerating frame formats: %d\n", errno);
		return errno;
	}

	return 0;
}

int check_videoIn(struct vdIn *vd, char *device)
{
	int ret;
	struct v4l2_capability cap;

	if (vd == NULL || device == NULL)
		return -1;

 	snprintf(vd->videodevice, VIDEO_DEV_NAME_MAX, "%s", device);

	printf("video %s \n", vd->videodevice);
//edit by mlfeng
	if ((vd->fd = open(vd->videodevice, O_RDWR| O_NONBLOCK)) == -1) {
//	if ((vd->fd = open(vd->videodevice, O_RDWR)) == -1) {
		perror("ERROR opening V4L interface \n");
		return -1;
	}

	memset(&cap, 0, sizeof(struct v4l2_capability));
	ret = ioctl(vd->fd, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		printf("Error opening device %s: unable to query device.\n",
		       vd->videodevice);
		goto fatal;
	}

	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
		printf("Error opening device %s: video capture not supported.\n",
		       vd->videodevice);
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		printf("%s does not support streaming i/o\n", vd->videodevice);
	}

	if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
		printf("%s does not support read i/o\n", vd->videodevice);
	}

	enum_frame_formats(vd->fd);
 fatal:
	close(vd->fd);
	//free(vd->videodevice);
	return 0;
}

int enum_controls(int vd) //struct vdIn *vd)
{
	struct v4l2_queryctrl queryctrl;
	struct v4l2_querymenu querymenu;
	struct v4l2_control   control_s;
	struct v4l2_input*    getinput;

	//Name of the device
	getinput=(struct v4l2_input *)malloc(sizeof(struct v4l2_input));
	memset(getinput, 0, sizeof(struct v4l2_input));
	getinput->index=0;
	ioctl(vd,VIDIOC_ENUMINPUT , getinput);
	printf ("Available controls of device '%s' (Type 1=Integer 2=Boolean 3=Menu 4=Button)\n", getinput->name);

	//subroutine to read menu items of controls with type 3
	void enumerate_menu (void) {
		printf ("  Menu items:\n");
		memset (&querymenu, 0, sizeof (querymenu));
		querymenu.id = queryctrl.id;
		for (querymenu.index = queryctrl.minimum;
		     querymenu.index <= queryctrl.maximum;
		     querymenu.index++) {
			if (0 == ioctl (vd, VIDIOC_QUERYMENU, &querymenu)) {
				printf ("  index:%d name:%s\n", querymenu.index, querymenu.name);
				//SDL_Delay(10);
			} else {
				printf ("error getting control menu");
				break;
			}
		}
	}

	//predefined controls
	printf ("V4L2_CID_BASE         (predefined controls):\n");
	memset (&queryctrl, 0, sizeof (queryctrl));
	for (queryctrl.id = V4L2_CID_BASE;
	     queryctrl.id < V4L2_CID_LASTP1;
	     queryctrl.id++) {
		if (0 == ioctl (vd, VIDIOC_QUERYCTRL, &queryctrl)) {
			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				continue;
			control_s.id=queryctrl.id;
			ioctl(vd, VIDIOC_G_CTRL, &control_s);
			//SDL_Delay(10);
			printf (" index:%-10d name:%-32s type:%d min:%-5d max:%-5d step:%-5d def:%-5d now:%d \n",
				queryctrl.id, queryctrl.name, queryctrl.type, queryctrl.minimum,
				queryctrl.maximum, queryctrl.step, queryctrl.default_value, control_s.value);
			if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
				enumerate_menu ();
		} else {
			if (errno == EINVAL)
				continue;
			printf ("error getting base controls");
			goto fatal_controls;
		}
	}

	//driver specific controls
	printf ("V4L2_CID_PRIVATE_BASE (driver specific controls):\n");
	for (queryctrl.id = V4L2_CID_PRIVATE_BASE;;
	     queryctrl.id++) {
		if (0 == ioctl (vd, VIDIOC_QUERYCTRL, &queryctrl)) {
			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				continue;
			control_s.id=queryctrl.id;
			ioctl(vd, VIDIOC_G_CTRL, &control_s);
			//SDL_Delay(20);
			printf (" index:%-10d name:%-32s type:%d min:%-5d max:%-5d step:%-5d def:%-5d now:%d \n",
				queryctrl.id, queryctrl.name, queryctrl.type, queryctrl.minimum,
				queryctrl.maximum, queryctrl.step, queryctrl.default_value, control_s.value);
			if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
				enumerate_menu ();
		} else {
			if (errno == EINVAL)
				break;
			perror ("error getting private base controls");
			goto fatal_controls;
		}
	}
	return 0;
 fatal_controls:
	return -1;
}

int save_controls(int vd)
{
	struct v4l2_queryctrl queryctrl;
	struct v4l2_control   control_s;
	FILE *configfile;
	memset (&queryctrl, 0, sizeof (queryctrl));
	memset (&control_s, 0, sizeof (control_s));
	configfile = fopen("grab.cfg", "w");
	if ( configfile == NULL) {
		printf( "saving configfile grab.cfg failed, errno = %d (%s)\n", errno, strerror( errno));
	}
	else {
		fprintf(configfile, "id         value      # grab control settings configuration file\n");
		for (queryctrl.id = V4L2_CID_BASE;
		     queryctrl.id < V4L2_CID_LASTP1;
		     queryctrl.id++) {
			if (0 == ioctl (vd, VIDIOC_QUERYCTRL, &queryctrl)) {
				if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
					continue;
				control_s.id=queryctrl.id;
				ioctl(vd, VIDIOC_G_CTRL, &control_s);
				//SDL_Delay(10);
				fprintf (configfile, "%-10d %-10d # name:%-32s type:%d min:%-5d max:%-5d step:%-5d def:%d\n",
					 queryctrl.id, control_s.value, queryctrl.name, queryctrl.type, queryctrl.minimum,
					 queryctrl.maximum, queryctrl.step, queryctrl.default_value);
				printf ("%-10d %-10d # name:%-32s type:%d min:%-5d max:%-5d step:%-5d def:%d\n",
					queryctrl.id, control_s.value, queryctrl.name, queryctrl.type, queryctrl.minimum,
					queryctrl.maximum, queryctrl.step, queryctrl.default_value);
				//SDL_Delay(10);
			}
		}
		for (queryctrl.id = V4L2_CID_PRIVATE_BASE;;
		     queryctrl.id++) {
			if (0 == ioctl (vd, VIDIOC_QUERYCTRL, &queryctrl)) {
				if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
					continue;
				if ((queryctrl.id==134217735) || (queryctrl.id==134217736))
					continue;
				control_s.id=queryctrl.id;
				ioctl(vd, VIDIOC_G_CTRL, &control_s);
				//SDL_Delay(10);
				fprintf (configfile, "%-10d %-10d # name:%-32s type:%d min:%-5d max:%-5d step:%-5d def:%d\n",
					 queryctrl.id, control_s.value, queryctrl.name, queryctrl.type, queryctrl.minimum,
					 queryctrl.maximum, queryctrl.step, queryctrl.default_value);
				printf ("%-10d %-10d # name:%-32s type:%d min:%-5d max:%-5d step:%-5d def:%d\n",
					queryctrl.id, control_s.value, queryctrl.name, queryctrl.type, queryctrl.minimum,
					queryctrl.maximum, queryctrl.step, queryctrl.default_value);
			} else {
				if (errno == EINVAL)
					break;
			}
		}
		fflush(configfile);
		fclose(configfile);
		//SDL_Delay(100);
	}

	return 0;
}


int load_controls(int vd) //struct vdIn *vd)
{
#if 0
	struct v4l2_control   control;
	FILE *configfile;
	memset (&control, 0, sizeof (control));
	configfile = fopen("grab.cfg", "r");
	if ( configfile == NULL) {
		printf( "configfile grab.cfg open failed, errno = %d (%s)\n", errno, strerror( errno));
	}
	else {
		printf("loading controls from grab.cfg \n");
		char buffer[512];
		fgets(buffer, sizeof(buffer), configfile);
		while (NULL !=fgets(buffer, sizeof(buffer), configfile) )
		{
			sscanf(buffer, "%i%i", &control.id, &control.value);
			if (ioctl(vd, VIDIOC_S_CTRL, &control))
				printf("ERROR id:%d val:%d \n", control.id, control.value);
			else
				printf("OK    id:%d val:%d \n", control.id, control.value);
			//SDL_Delay(20);
		}
		fclose(configfile);
	}
#endif

	return 0;
}

int init_videoIn(struct vdIn *vd, char *device, int width, int height, int fps,
		 int format, int grabmethod)
{
	if (vd == NULL || device == NULL)
		return -1;

	if (width == 0 || height == 0)
		return -1;

	if (grabmethod < 0 || grabmethod > 1)
		grabmethod = 1;		//mmap by default;

	snprintf(vd->videodevice, VIDEO_DEV_NAME_MAX, "%s", device);
	printf("video %s \n", vd->videodevice);

	vd->signalquit = 1;
	vd->width = width;
	vd->height = height;
	vd->fps = fps;
	vd->formatIn = format;
	vd->grabmethod = grabmethod;
	vd->fileCounter = 0;
	vd->rawFrameCapture = 0;

	if (init_v4l2(vd) < 0) {
		printf(" Init v4L2 failed !! exit fatal \n");
		goto error;;
	}

	/* alloc a temp buffer to reconstruct the pict */
	vd->framesizeIn = (vd->width * vd->height << 1);
	switch (vd->formatIn) {
	case V4L2_PIX_FMT_MJPEG:
		vd->tmpbuffer =
			(unsigned char *)malloc((size_t) vd->framesizeIn);
		if (!vd->tmpbuffer)
			goto error;
		vd->framebuffer =
			(unsigned char *)malloc((size_t) vd->width * (vd->height + 8) * 2);
		break;
	case V4L2_PIX_FMT_YUYV:
		vd->framebuffer = (unsigned char *)malloc((size_t) vd->framesizeIn);
		break;
	default:
		printf(" should never arrive exit fatal !!\n");
		goto error;
		break;
	}
	if (!vd->framebuffer)
		goto error;

	return 0;

 error:
	//free(vd->videodevice);
	close(vd->fd);
	return -1;
}

#if 1
void get_fmt(struct vdIn *vd)
{
	struct v4l2_fmtdesc fmt;
	int ret=0;

	memset(&fmt, 0, sizeof(fmt));
	fmt.index = 0;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while ((ret = ioctl(vd->fd, VIDIOC_ENUM_FMT, &fmt)) == 0) {

	fmt.index++;
	printf("{ pixelformat = ''%c%c%c%c'', description = ''%s'' }\n",
				fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF,
				(fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,
				fmt.description);
	}
}
#endif

static int init_v4l2(struct vdIn *vd)
{
	int i;
	int ret = 0;
	struct v4l2_requestbuffers rb;
	struct v4l2_format fmt;
	struct v4l2_capability cap;





#if 1
//edit by mlfeng
	if ((vd->fd = open(vd->videodevice, O_RDWR| O_NONBLOCK)) == -1) {
//	if ((vd->fd = open(vd->videodevice, O_RDWR)) == -1) {
		perror("ERROR opening V4L interface \n");
		return -1;
	}
#else
	if ((vd->fd = open(vd->videodevice, O_RDWR)) == -1) {
		perror("ERROR opening V4L interface \n");
		return -1;
	}
#endif

	memset(&cap, 0, sizeof(struct v4l2_capability));
	ret = ioctl(vd->fd, VIDIOC_QUERYCAP, &cap);//查询驱动功能
	if (ret < 0) {
		printf("Error opening device %s: unable to query device.\n",
		       vd->videodevice);
		goto fatal;
	}

	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
		printf("Error opening device %s: video capture not supported.\n",
		       vd->videodevice);
		goto fatal;;
	}

	if (vd->grabmethod) {
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			printf("%s does not support streaming i/o\n", vd->videodevice);
			goto fatal;
		}
	} else {
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			printf("%s does not support read i/o\n", vd->videodevice);
			goto fatal;
		}
	}
	printf("coc --1\n");
	get_fmt(vd);

	printf("coc --2\n");

		/* set format in */
	memset(&fmt, 0, sizeof(struct v4l2_format));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = vd->width;
	fmt.fmt.pix.height = vd->height;
	fmt.fmt.pix.pixelformat = vd->formatIn;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;
	ret = ioctl(vd->fd, VIDIOC_S_FMT, &fmt);//设置驱动显示格式
	if (ret < 0) {
		printf("Unable to set format: %d.\n", errno);
		goto fatal;
	}

	if ((fmt.fmt.pix.width != vd->width) ||
	    (fmt.fmt.pix.height != vd->height)) {
		printf(" format asked(%dx%d) unavailable, get %dx%d \n",
		       vd->width, vd->height, fmt.fmt.pix.width, fmt.fmt.pix.height);
		vd->width = fmt.fmt.pix.width;
		vd->height = fmt.fmt.pix.height;
		/* look the format is not part of the deal ??? */
		vd->formatIn = fmt.fmt.pix.pixelformat;
	}


	fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(vd->fd, VIDIOC_G_FMT, &fmt);



	printf("dddddddddddddddCurrent data format information:\n\twidth:%d\n\theight:%d\n",
				fmt.fmt.pix.width,fmt.fmt.pix.height);
#if 1
        /* set framerate */
	{
		//struct v4l2_streamparm* setfps;
		//setfps=(struct v4l2_streamparm *) calloc(1, sizeof(struct v4l2_streamparm));
		//memset(setfps, 0, sizeof(struct v4l2_streamparm));
		//setfps->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		//setfps->parm.capture.timeperframe.numerator = 1;
		//setfps->parm.capture.timeperframe.denominator = vd->fps;

		struct v4l2_streamparm setfps;
		memset(&setfps, 0, sizeof(struct v4l2_streamparm));
		setfps.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;//V4L2_BUF_TYPE_VIDEO_CAPTURE;
		setfps.parm.capture.timeperframe.numerator = 1;
		setfps.parm.capture.timeperframe.denominator = vd->fps;

		ret = ioctl(vd->fd, VIDIOC_S_PARM, &setfps);//设置参数
	}
#endif
	/* request buffers */
	memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
	rb.count = NB_BUFFER;
	rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rb.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(vd->fd, VIDIOC_REQBUFS, &rb);//分配内存
	if (ret < 0) {
		printf("Unable to allocate buffers: %d.\n", errno);
		goto fatal;
	}

	/* map the buffers */
	for (i = 0; i < NB_BUFFER; i++) {
		memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
		vd->buf.index = i;
		vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vd->buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(vd->fd, VIDIOC_QUERYBUF, &vd->buf);//把VIDIOC_REQBUFS中分配的数据缓存转换成物理地址
		if (ret < 0) {
			printf("Unable to query buffer (%d).\n", errno);
			goto fatal;
		}
		if (debug)
			printf("length: %u offset: %u\n", vd->buf.length,
			       vd->buf.m.offset);
		vd->mem[i] = mmap(NULL /* start anywhere */ ,
				  vd->buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, vd->fd,
				  vd->buf.m.offset);
		if (vd->mem[i] == MAP_FAILED) {
			printf("Unable to map buffer (%d)\n", errno);
			goto fatal;
		}
		vd->memlen[i] = vd->buf.length;
		if (debug)
			printf("Buffer mapped at address %p.\n", vd->mem[i]);
	}

	/* Queue the buffers. */
	for (i = 0; i < NB_BUFFER; ++i) {
		memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
		vd->buf.index = i;
		vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vd->buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(vd->fd, VIDIOC_QBUF, &vd->buf);//把数据从缓存中读取出来
		if (ret < 0) {
			printf("Unable to queue buffer (%d).\n", errno);
			goto fatal;;
		}
	}
	return 0;

 fatal:
	return -1;

}

static int video_enable(struct vdIn *vd)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret;
	int starttime,endtime = 0;

	starttime = usectime();
	ret = ioctl(vd->fd, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		printf("Unable to %s capture: %d.\n", "start", errno);
		return ret;
	}
	vd->isstreaming = 1;
	endtime = usectime();
	printf("ooooooooooo 120 frame time = %lld %lldns/frame\n",endtime - starttime,(endtime-starttime)/120);
	return 0;
}

static int video_disable(struct vdIn *vd)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret;

	ret = ioctl(vd->fd, VIDIOC_STREAMOFF, &type);
	if (ret < 0) {
		printf("Unable to %s capture: %d.\n", "stop", errno);
		return ret;
	}
	vd->isstreaming = 0;
	return 0;
}

static void errno_exit (const char * s)
{
	fprintf (stderr, "%s error %d, %s/n",s, errno, strerror (errno));
	exit (EXIT_FAILURE);
}

static struct timeval gt1, gt2;
static void start_timer(void) {
	//gettimeofday(&gt1, NULL);
}

static void stop_timer(void) {
#if 0
	double elapsedTime;

	gettimeofday(&gt2, NULL);

	elapsedTime = (gt2.tv_sec - gt1.tv_sec) * 1000.0;      // sec to ms
	elapsedTime += (gt2.tv_usec - gt1.tv_usec) / 1000.0;   // us to ms

	printf("===>elapsedTime: %fms\n", elapsedTime);
#endif
}

int uvcGrab(struct vdIn *vd, int timeout, int do_perf, int idx)
{
#define HEADERFRAME1 0xaf
	int ret;
	volatile int retries;
	int endtime,starttime;
//	int grap_count = 10;
	if (!vd->isstreaming)
		if (video_enable(vd))
			goto err;

	starttime = usectime();
	start_timer();

 again:
//	while(grap_count--) {
#if 1
	for (retries = 0; retries < 3; retries++) {
		fd_set fds;
		struct timeval tv;
		int r;
		FD_ZERO (&fds);
		FD_SET (vd->fd, &fds);


		//printf("===>timeout = %d\n", timeout);
		tv.tv_sec = timeout;
		tv.tv_usec = 0;

//		printf("$debug------------%s,%d,retries==%d\n",__func__,__LINE__,retries);
		//start_timer();
		starttime = usectime();
		// r = select (vd->fd + 1, &fds, NULL, NULL, NULL);//wait block
		r = select (vd->fd + 1, &fds, NULL, NULL, &tv);
		endtime = usectime();
//		printf("the select time  = %lld %lldns/frame\n",endtime - starttime,(endtime-starttime)/120);
		select_zong += endtime - starttime;
//		printf("$debug------------%s,%d,\n",__func__,__LINE__);
		//stop_timer();
		if (-1 == r) {
			if (EINTR == errno)
				continue;

			fprintf(stderr, "select errno = %d(%s)\n", errno, strerror(errno));
			errno_exit ("select");
		}

		if (0 == r) {
			fprintf (stderr, "select timeout11\n");
#if 0
			if (init_v4l2(vd) < 0) {
				printf(" Init v4L2 failed !! exit fatal \n");
				goto again;;
			}
#endif
			exit (EXIT_FAILURE);
		}
		break;
	}

	if (retries == 3)
		return -1;
#endif

	memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
	vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vd->buf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(vd->fd, VIDIOC_DQBUF, &vd->buf);
	stop_timer();
	if (ret < 0) {
		printf("Unable to dequeue buffer, ret=%d (%d: %s).\n", ret, errno, strerror(errno));
		if (errno == EAGAIN)
			goto again;
		else
			goto err;
	}

//	printf("+");
	if (!do_perf) {
		switch (vd->formatIn) {
		case V4L2_PIX_FMT_MJPEG:
			if(vd->buf.bytesused <= HEADERFRAME1) {	/* Prevent crash on empty image */
				printf("Ignoring empty buffer ...\n");
				return 0;
			}
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			/*	if (jpeg_decode(&vd->framebuffer, vd->tmpbuffer, &vd->width,
				&vd->height) < 0) {
				printf("jpeg decode errors\n");
				goto err;
				} */ //treckle
			if (debug)
				printf("bytes in used %d \n", vd->buf.bytesused);
			break;
		case V4L2_PIX_FMT_YUYV:
			if (vd->buf.bytesused > vd->framesizeIn)
				memcpy(vd->framebuffer, vd->mem[vd->buf.index],
						(size_t) vd->framesizeIn);
			else
				memcpy(vd->framebuffer, vd->mem[vd->buf.index],
						(size_t) vd->buf.bytesused);
			break;
		default:
			goto err;
			break;
		}
	} else {
		memcpy(vd->perf_timestamp + idx, &vd->buf.timestamp, sizeof(struct timeval));
	}

	ret = ioctl(vd->fd, VIDIOC_QBUF, &vd->buf);
	if (ret < 0) {
		printf("Unable to requeue buffer (%d).\n", errno);
		goto err;
	}
//	}
	endtime = usectime();
//	printf("the one frame time = %lld %lldns/frame\n",endtime - starttime,(endtime-starttime)/120);
	return 0;
 err:
	vd->signalquit = 0;
	return -1;
}

int close_v4l2(struct vdIn *vd)
{
	int i;
	int ret;

	if (vd->isstreaming)
		video_disable(vd);

	if (vd->tmpbuffer)
		free(vd->tmpbuffer);

	for (i = 0; i < NB_BUFFER; i++) {
		ret = munmap(vd->mem[i], vd->memlen[i]);
		//fprintf(stderr, "====>%s:%d ret = %d\n", __func__, __LINE__, ret);
	}

	vd->tmpbuffer = NULL;
	free(vd->framebuffer);
	vd->framebuffer = NULL;

	fprintf(stderr, "close fd %d\n", vd->fd);
	close(vd->fd);
	close(vd->fd);
	close(vd->fd);
	close(vd->fd);

	return 0;
}

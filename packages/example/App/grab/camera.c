/*******************************************************************************
#	 	luvcview: Sdl video Usb Video Class grabber           .        #
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include "videodev.h"
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include "v4l2uvc.h"
#include "savejpeg.h"


static const char version[] = VERSION;

int query_camera_formats(struct vdIn *videoIn, const char *videodevice) {
	return check_videoIn(videoIn,(char *) videodevice);
}

int init_camera(struct vdIn *videoIn, const char *videodevice,
		int width, int height, int format,
		int fps, int grabmethod)
{
	int readconfigfile = 0;

	int enableRawFrameCapture = 0;

	printf("grab version %s \n", version);

	if (videodevice == NULL || *videodevice == 0) {
		videodevice = "/dev/video0";
	}

	if (init_videoIn(videoIn, (char *) videodevice,
			 width, height, fps,
			 format, grabmethod) < 0)
		return -1;

	/* if we're supposed to read the control settings from a configfile, do that now */
#if 0// ly del from pri
	if ( readconfigfile )
		load_controls(videoIn->fd);
	if (enableRawFrameCapture)
		videoIn->rawFrameCapture = enableRawFrameCapture;
#endif
	//initLut();

	return 0;
}

void close_camera(struct vdIn * vd)
{
	close_v4l2(vd);
	//freeLut();
}

extern int get_pictureYUYV(unsigned char *buf,int width,int height, const char *name);
int grab (struct vdIn * videoIn, const char *filename, int timeout, int do_perf, int idx)
{

#if 1
	if(!videoIn->signalquit)
	{
		printf("camera not init!\n");
		return -1;
	}

#if 1
	if (uvcGrab(videoIn, timeout, do_perf, idx) < 0) {
		printf("Error grabbing \n");
		return -1;
	}
#else
ok:
		printf("my-test-little\n");
	if (uvcGrab(videoIn, timeout, do_perf, idx) < 0) {
		printf("Error grabbing \n");
		return -1;
	}
	sleep (1);
	goto ok;
#endif

	if (do_perf) goto out;

#endif

#if 1
	switch(videoIn->formatIn){
	case V4L2_PIX_FMT_MJPEG:
		printf("jpeg\n");
		get_picture(videoIn->tmpbuffer,videoIn->buf.bytesused, filename);
		break;
	case V4L2_PIX_FMT_YUYV:
		printf("yuv\n");
	    get_pictureYUYV(videoIn->framebuffer,videoIn->width,videoIn->height, filename);
		break;
	default:
		break;
	}
#endif

#if 0
	printf("11111111\n");
	int fd,starttime,endtime;
	int buf[640*480*4] ={0};

	fd = fopen("yuvbuf.dat","w");
	starttime = usectime();
	fread(buf,640*480*2,1,fd);
	get_pictureYUYV(buf,640,480, "a.jpg");
	endtime = usectime();
	printf("the one frame time = %lld %lldns/frame\n",endtime - starttime,(endtime-starttime)/120);
#endif

	//printf("===>enter %s:%d\n", __func__, __LINE__);
	//printf("\b");
	//fflush(stdout);

 out:
	return videoIn->buf.bytesused;
}

void
SetBrightness (struct vdIn *vdin, __u8 bright)
{
	struct v4l2_control control_s;
	int err;
	/* bright(0-255), val(-16-+16) */
	int val = bright/8-16;

        control_s.id = V4L2_CID_BRIGHTNESS;
        control_s.value = val;
        if ((err = ioctl(vdin->fd, VIDIOC_S_CTRL, &control_s)) < 0) {
		printf("ioctl set control error\n");
		return;
        }
}

void
SetContrast (struct vdIn *vdin, __u8 contrast)
{
	struct v4l2_control control_s;
	int err;
	/* constrast(0-255), val(1-32) */
	int val = contrast/8;
        control_s.id = V4L2_CID_CONTRAST;
        control_s.value = val;
        if ((err = ioctl(vdin->fd, VIDIOC_S_CTRL, &control_s)) < 0) {
		printf("ioctl set control error\n");
		return;
        }
}

void
SetSaturation (struct vdIn *vdin, __u8 Saturation)
{
        struct v4l2_control control_s;
	int err;

        control_s.id = V4L2_CID_SATURATION;
        control_s.value = Saturation;
        if ((err = ioctl(vdin->fd, VIDIOC_S_CTRL, &control_s)) < 0) {
		printf("ioctl set control error\n");
		return;
        }
}


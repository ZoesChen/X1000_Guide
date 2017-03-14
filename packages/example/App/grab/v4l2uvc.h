/*******************************************************************************
#	 	luvcview: Sdl video Usb Video Class grabber          .         #
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

#ifndef __V4L2UVC_H__
#define __V4L2UVC_H__

#include "videodev.h"

//#define NB_BUFFER 4
#define NB_BUFFER 30
#define DHT_SIZE 432

#define VIDEO_DEV_NAME_MAX	16

struct vdIn {
	int fd;
	char videodevice[VIDEO_DEV_NAME_MAX];
	struct v4l2_buffer buf;
	void *mem[NB_BUFFER]; /* mmap base address */
	int memlen[NB_BUFFER]; /* mmap length */
	unsigned char *tmpbuffer;
	unsigned char *framebuffer;
	int isstreaming;
	int grabmethod;
	int width;
	int height;
	int fps;
	int formatIn;
	int framesizeIn;
	int signalquit;

	/* raw frame capture(save the orignal data to a file) */
	int rawFrameCapture;
	unsigned int fileCounter;

	struct timeval *perf_timestamp;
};


int init_videoIn(struct vdIn *vd, char *device, int width, int height, int fps,
	     int format, int grabmethod);
int close_v4l2(struct vdIn *vd);

int uvcGrab(struct vdIn *vd, int timeout, int do_perf, int idx);

int enum_controls(int vd);
int check_videoIn(struct vdIn *vd, char *device);

int save_controls(int vd);
int load_controls(int vd);

#endif /* __V4L2UVC_H__ */

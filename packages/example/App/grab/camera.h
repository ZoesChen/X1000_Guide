#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "v4l2uvc.h"

extern int query_camera_formats(struct vdIn *videoIn, const char *videodevice);

extern int init_camera(struct vdIn *videoIn, const char *videodevice,
		int width, int height, int format,
		int fps, int grabmethod);
extern void close_camera(struct vdIn * vd);

extern int grab (struct vdIn * videoIn, const char *filename, int timeout, int do_perf, int idx);

extern void SetBrightness (struct vdIn *vdin, __u8 bright);

extern void SetContrast (struct vdIn *vdin, __u8 contrast);

extern void SetSaturation (struct vdIn *vdin, __u8 Saturation);


#endif /* __CAMERA_H__ */

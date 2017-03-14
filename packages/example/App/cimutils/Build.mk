LOCAL_PATH := $(my-dir)
#==================================================
# build cimutils
include $(CLEAR_VARS)
LOCAL_MODULE=cimutils
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= main.c      \
	misc.c      \
	signal.c    \
	convert_soft.c    \
	raw/saveraw.c   \
	bmp/savebmp.c   \
	jpg/savejpeg.c  \
	lcd/framebuffer.c   \
	cim/cim_fmt.c   \
	cim/video.c \
	cim/process.c   \
	cim/convert.c   \
	cim/regs.c  \
	cim/scale.c \
	cim/preview_display.c

LOCAL_CFLAGS := -mmxu
# LOCAL_CFLAGS := -mmxu -DUSE_V4L2
LOCAL_LDLIBS := -lc -ljpeg-hw -ljpeg

LOCAL_C_INCLUDES:= include

LOCAL_DEPANNER_MODULES := CameraDemo.sh libjpeg-hw libjpeg

include $(BUILD_EXECUTABLE)
#===================================================
# copy the LOCAL_SRC_FILES
#
include $(CLEAR_VARS)
LOCAL_MODULE := CameraDemo.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)/CameraDemo
LOCAL_COPY_FILES := CameraDemo.sh:CameraDemo.sh
include $(BUILD_PREBUILT)
#

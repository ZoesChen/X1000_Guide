LOCAL_PATH := $(my-dir)
#==================================================
#build grub
include $(CLEAR_VARS)
LOCAL_MODULE=grab
LOCAL_MODULE_TAGS:=optional
LOCAL_SRC_FILES:= camera.c  \
				grab.c  \
				savejpeg.c  \
				v4l2uvc.c

LOCAL_CFLAGS := -Wall -O -DVERSION=\"0.1.4\"
LOCAL_LDLIBS := -lc

LOCAL_STATIC_LIBRARIES := libjpeg.a
LOCAL_DEPANNER_MODULES:= libjpeg
#depend on the file (basename)

include $(BUILD_EXECUTABLE)


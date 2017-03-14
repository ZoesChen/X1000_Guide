LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE=libsuspend
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES := autosuspend.c \
				autosuspend_autosleep.c \
				autosuspend_earlysuspend.c \
				autosuspend_wakeup_count.c

LOCAL_LDLIBS := -lc -lpthread
LOCAL_EXPORT_C_INCLUDE_FILES:= include/suspend/autosuspend.h
LOCAL_C_INCLUDES := include
LOCAL_SHARED_LIBRARIES := libdlog.so #depend on the file (basename)
LOCAL_DEPANNER_MODULES := dlog

LOCAL_CFLAGS := -O2 -Wall -fPIC -D_GNU_SOURCE
include $(BUILD_SHARED_LIBRARY)


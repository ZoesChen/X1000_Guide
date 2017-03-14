LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE=libashmem
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= ashmem-dev.c strlcpy.c

LOCAL_EXPORT_C_INCLUDE_FILES:=  ashmem.h strlcpy.h
LOCAL_C_INCLUDES := .
LOCAL_CFLAGS := -O2 -Wall -fPIC
include $(BUILD_SHARED_LIBRARY)


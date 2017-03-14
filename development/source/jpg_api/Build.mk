LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE=libjpeg-hw
LOCAL_MODULE_TAGS:=optional

LOCAL_SRC_FILES:= jpeg_encode2.c    \
		  jz_mem.c          \
	          vpu_common.c      \

LOCAL_EXPORT_C_INCLUDE_FILES:=  include/head.h          \
				include/ht.h            \
				include/jpeg.h          \
				include/jpeg_private.h  \
				include/jz_mem.h        \
				include/jzm_jpeg_enc.h  \
				include/qt.h            \
				include/vpu_common.h  #export for others to use
LOCAL_C_INCLUDES := include
LOCAL_CFLAGS := -O2 -Wall -fPIC
include $(BUILD_SHARED_LIBRARY)


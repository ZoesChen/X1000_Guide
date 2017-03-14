LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE=libjpeg
LOCAL_MODULE_TAGS:=optional

LOCAL_MODULE_GEN_SHARED_FILES=.libs/libjpeg.so .libs/libjpeg.so.62 .libs/libjpeg.so.62.0.0 .libs/libturbojpeg.so
LOCAL_MODULE_GEN_STATIC_FILES=.libs/libjpeg.a
#LOCAL_MODULE_GEN_BINRARY_FILES=.libs/cjpeg .libs/djpeg
LOCAL_MODULE_CONFIG_FILES:=Makefile
LOCAL_MODULE_CONFIG=./autogen.sh;./configure --host=$(DEVICE_COMPILER_PREFIX)
LOCAL_MODULE_COMPILE=make -j$(MAKE_JLEVEL)
LOCAL_MODULE_COMPILE_CLEAN=make distclean

LOCAL_EXPORT_C_INCLUDE_FILES := jpeglib.h \
								jconfig.h  \
								jmorecfg.h \
								jerror.h

include $(BUILD_THIRDPART)

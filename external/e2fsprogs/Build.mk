LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)
#vversion e2fsprogs-1.42.9
LOCAL_MODULE=e2fsprogs
LOCAL_MODULE_TAGS:=optional

LOCAL_MODULE_GEN_BINRARY_FILES=e2fsck/e2fsck \
								misc/tune2fs \

LOCAL_MODULE_CONFIG_FILES:=Makefile
LOCAL_MODULE_CONFIG=./configure
LOCAL_MODULE_COMPILE=make -j$(MAKE_JLEVEL)
LOCAL_MODULE_COMPILE_CLEAN=make distclean

include $(BUILD_HOST_THIRDPART)

#######################
# Copy the genext2fs.sh
include $(CLEAR_VARS)

LOCAL_MODULE := HOST-genext2fs.sh
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(OUT_HOST_BINRARY_DIR)
LOCAL_COPY_FILES := genext2fs.sh:genext2fs.sh

include $(BUILD_PREBUILT)




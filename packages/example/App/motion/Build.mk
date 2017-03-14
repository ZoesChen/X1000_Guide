LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE=motion
LOCAL_MODULE_TAGS:=optional

LOCAL_MODULE_PATH:=$(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)/$(LOCAL_MODULE)

LOCAL_MODULE_GEN_BINRARY_FILES := motion

LOCAL_DEPANNER_MODULES := libjpeg-hw libjpeg

LOCAL_C_INCLUDES := include

LOCAL_MODULE_CONFIG :=./configure CC=mips-linux-gnu-gcc --host=mips-linux-gnu --without-ffmpeg CFLAGS=-I\ $(TOP_DIR)/$(OUT_DEVICE_INCLUDE_DIR) LDFLAGS=-L\ $(TOP_DIR)/$(OUT_DEVICE_SHARED_DIR)\ -I\ $(TOP_DIR)/$(OUT_DEVICE_INCLUDE_DIR)\ -ljpeg\ -ljpeg-hw --prefix=$(TOP_DIR)/$(LOCAL_PATH)/install

LOCAL_MODULE_CONFIG_FILES:=Makefile

LOCAL_MODULE_COMPILE := make -j$(MAKE_JLEVEL)

LOCAL_MODULE_COMPILE_CLEAN := make distclean

include $(BUILD_THIRDPART)



##############copy motion.conf
include $(CLEAR_VARS)
LOCAL_MODULE := motion.conf
LOCAL_MODULE_TAGS:=optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)/motion
LOCAL_COPY_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE=libhardware
LOCAL_MODULE_TAGS:=optional
LOCAL_MODULE_GEN_SHARED_FILES:=libhardware.so
LOCAL_SRC_FILES:=hardware.c
LOCAL_CFLAGS := -Wa,-mips32r2 -O2 -G 0 -Wall -fPIC -shared
LOCAL_C_INCLUDES:=include
LOCAL_SHARED_LIBRARIES := libdlog.so
LOCAL_DEPANNER_MODULES:= dlog hardware.sh hardware_inc
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := hardware.sh
LOCAL_MODULE_TAGS :=optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/etc/profile.d
LOCAL_COPY_FILES := hardware.sh
include $(BUILD_PREBUILT)


include $(CLEAR_VARS)

LOCAL_MODULE := hardware_inc
LOCAL_MODULE_TAGS :=optional
LOCAL_MODULE_CLASS:=DIR
LOCAL_MODULE_PATH := $(OUT_DEVICE_INCLUDE_DIR)
LOCAL_MODULE_DIR := include/hardware
include $(BUILD_MULTI_PREBUILT)

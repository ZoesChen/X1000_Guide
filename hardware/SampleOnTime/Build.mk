LOCAL_PATH:= $(call my-dir)
# HAL module implemenation, not prelinked and stored in
# hw/<COPYPIX_HARDWARE_MODULE_ID>.<ro.board.platform>.so
include $(CLEAR_VARS)

LOCAL_MODULE := SampleOnTime.$(TARGET_BOOTLOADER_BOARD_NAME)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := SampleOnTime.c \
					circ_buf.c

LOCAL_CFLAGS :=  -Wall -fPIC
#LOCAL_C_INCLUDES := include
LOCAL_MODULE_PATH := $(OUT_DEVICE_SHARED_DIR)/hw
LOCAL_LDLIBS :=   -lpthread    -lasound
LOCAL_SHARED_LIBRARIES := libhardware.so
LOCAL_DEPANNER_MODULES:= hardware_inc alsa-include


include $(BUILD_SHARED_LIBRARY)




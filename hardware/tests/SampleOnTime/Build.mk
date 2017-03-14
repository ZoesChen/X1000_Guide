LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := SampleOnTimeTest
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := SampleOnTimeTest.c
LOCAL_CFLAGS := -Werror -fpic
LOCAL_C_INCLUDES := include
LOCAL_LDLIBS := -lc -ldl -lasound
LOCAL_SHARED_LIBRARIES := libhardware.so dlog
LOCAL_DEPANNER_MODULES:= libhardware alsa-include SampleOnTime.$(TARGET_BOOTLOADER_BOARD_NAME)
include $(BUILD_EXECUTABLE)


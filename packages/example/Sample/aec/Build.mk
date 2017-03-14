LOCAL_PATH:= $(call my-dir)
# HAL module implemenation, not prelinked and stored in
# hw/<COPYPIX_HARDWARE_MODULE_ID>.<ro.board.platform>.so
include $(CLEAR_VARS)

LOCAL_MODULE := aec_test
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := aec_test.c \
					webrtc_aec.cpp

LOCAL_CFLAGS :=  -Wall -fPIC
LOCAL_C_INCLUDES := include
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)/aec_test
LOCAL_LDLIBS :=  -L $(LOCAL_PATH)/lib -lpthread    -lasound -lc -lstdc++

# build libwebrtc_audio_preprocessing.so at external/webrtc/
LOCAL_SHARED_LIBRARIES := libhardware libwebrtc_audio_preprocessing libdlog
LOCAL_DEPANNER_MODULES:= hardware_inc alsa-include libhardware


include $(BUILD_EXECUTABLE)


ifeq (0, 1)
include $(CLEAR_VARS)
LOCAL_MODULE :=webrtc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)

LOCAL_COPY_FILES:= \
		usr/lib/libwebrtc_audio_processing.so.0:lib/libwebrtc_audio_processing.so.0

include $(BUILD_MULTI_PREBUILT)
endif

LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libgpio
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := gpio_device.cpp
LOCAL_DOC_FILES := gpio_device.h
LOCAL_EXPORT_C_INCLUDE_FILES := gpio_device.h
LOCAL_LDLIBS := -lc -lstdc++
LOCAL_CPPFLAGS := -fPIC

include $(BUILD_SHARED_LIBRARY)


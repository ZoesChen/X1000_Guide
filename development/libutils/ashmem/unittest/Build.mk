LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := ashmem_unittest
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ashmem_unittest.cpp
LOCAL_MODULE_PATH := $(LOCAL_PATH)
LOCAL_CPPFLAGS := -g  -Wall -Wextra -pthread
LOCAL_LDFLAGS := -lpthread
LOCAL_SHARED_LIBRARIES := libashmem.so
LOCAL_DEPANNER_MODULES := libashmem libgtest_target

include $(BUILD_NATIVE_TEST)
#include $(BUILD_HOST_NATIVE_TEST)

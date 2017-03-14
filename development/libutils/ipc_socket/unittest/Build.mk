LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := socket_local_unittest
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := socket_local_unittest.cpp
LOCAL_MODULE_PATH := $(LOCAL_PATH)
LOCAL_CPPFLAGS := -g  -Wall -Wextra -pthread
LOCAL_LDFLAGS := -lpthread
LOCAL_SHARED_LIBRARIES := libipc_socket.so

include $(BUILD_NATIVE_TEST)
#include $(BUILD_HOST_NATIVE_TEST)

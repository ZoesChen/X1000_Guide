LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := uart_sample
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH:=$(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)/$(LOCAL_MODULE)
LOCAL_SRC_FILES := uart_demo.cpp
LOCAL_LDLIBS := -lc -lstdc++
LOCAL_SHARED_LIBRARIES := libuart.so # source code : developmen/source
LOCAL_DEPANNER_MODULES := libuart

include $(BUILD_EXECUTABLE)


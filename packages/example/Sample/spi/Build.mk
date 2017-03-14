LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := spi_sample
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH:=$(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)/$(LOCAL_MODULE)
LOCAL_SRC_FILES := NorTest.cpp
LOCAL_LDLIBS := -lc -lstdc++
LOCAL_SHARED_LIBRARIES := libspi.so #source code : development/source
LOCAL_DEPANNER_MODULES := libspi

include $(BUILD_EXECUTABLE)


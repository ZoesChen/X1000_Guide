LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := i2c_sample
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH:=$(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)/$(LOCAL_MODULE)
LOCAL_SRC_FILES := PmuDevice.cpp
LOCAL_LDLIBS := -lc -lstdc++
LOCAL_SHARED_LIBRARIES := libi2c.so #source code : development/source
LOCAL_DEPANNER_MODULES := libi2c

include $(BUILD_EXECUTABLE)

# #####################
# eeprom Demo
#######################
LOCAL_MODULE := eeprom_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH:=$(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)/$(LOCAL_MODULE)
LOCAL_SRC_FILES := EEpromDevice.cpp
LOCAL_LDLIBS := -lc -lstdc++
LOCAL_SHARED_LIBRARIES := libi2c.so #source code : development/source
LOCAL_DEPANNER_MODULES := libi2c

include $(BUILD_EXECUTABLE)


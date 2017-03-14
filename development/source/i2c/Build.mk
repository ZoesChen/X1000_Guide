LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libi2c
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := I2cAdapter.cpp
LOCAL_DOC_FILES := I2cAdapter.h i2c-dev.h i2c.h
LOCAL_EXPORT_C_INCLUDE_FILES := I2cAdapter.h \
								i2c.h \
								i2c-dev.h
LOCAL_LDLIBS := -lc -lstdc++
LOCAL_CPPFLAGS := -fPIC

include $(BUILD_SHARED_LIBRARY)


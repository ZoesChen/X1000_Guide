LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libspi
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := Spi.cpp
LOCAL_DOC_FILES := Spi.cpp
LOCAL_EXPORT_C_INCLUDE_FILES := spi.h \
								spidev.h
LOCAL_LDLIBS := -lc -lstdc++
LOCAL_CPPFLAGS := -fPIC

include $(BUILD_SHARED_LIBRARY)


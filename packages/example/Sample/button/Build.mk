LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := button
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := button.c
LOCAL_CFLAGS := -DFAKE_LOG_DEVICE=1 -Werror -DHAVE_SYS_UIO_H -fpic
LOCAL_LDLIBS := -lc

LOCAL_DEPANNER_MODULES := ButtonDemo.sh
include $(BUILD_EXECUTABLE)

#===========================
#copy
#
include $(CLEAR_VARS)
LOCAL_MODULE := ButtonDemo.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)/ButtonDemo
LOCAL_COPY_FILES := ButtonDemo.sh:ButtonDemo.sh
include $(BUILD_PREBUILT)
#



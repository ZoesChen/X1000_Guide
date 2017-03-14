LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE=minigui_demo
LOCAL_MODULE_TAGS:=optional
LOCAL_SRC_FILES:=helloworld.c

LOCAL_CFLAGS := -Wall
LOCAL_LDLIBS := -lminigui_ths -lpthread -ldl -lm -lc
LOCAL_MODULE_PATH:=$(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)/$(LOCAL_MODULE)
LOCAL_DEPANNER_MODULES:=minigui_ths ingenic.bmp minigui_resource MiniGUI.cfg

include $(BUILD_EXECUTABLE)



##############copy ingenic.bmp
include $(CLEAR_VARS)
LOCAL_MODULE := ingenic.bmp
LOCAL_MODULE_TAGS:=optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)/minigui_demo
LOCAL_COPY_FILES := ingenic.bmp:$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

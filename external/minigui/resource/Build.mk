LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := minigui_resource
LOCAL_MODULE_TAGS :=optional
LOCAL_MODULE_CLASS:=DIR
LOCAL_MODULE_PATH := $(OUT_DEVICE_LOCAL_DIR)
LOCAL_MODULE_DIR := share

include $(BUILD_MULTI_PREBUILT)

####################copy Minigui.cfg
include $(CLEAR_VARS)
LOCAL_MODULE := MiniGUI.cfg
LOCAL_MODULE_TAGS :=optional
LOCAL_MODULE_PATH :=$(TARGET_FS_BUILD)/etc
LOCAL_COPY_FILES := MiniGUI.cfg:MiniGUI.cfg
include $(BUILD_PREBUILT)

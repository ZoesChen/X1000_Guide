# LOCAL_PATH needed null.
LOCAL_PATH :=

#===================================================
# copy the gdbserver
# #
include $(CLEAR_VARS)
LOCAL_MODULE := gdbserver
LOCAL_MODULE_TAGS :=userdebug
LOCAL_MODULE_PATH :=$(OUT_DEVICE_BINRARY_DIR)
LOCAL_COPY_FILES := gdbserver:$(COMPILER_PATH)/mips-linux-gnu/libc/usr/bin/gdbserver

include $(BUILD_PREBUILT)

LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE=minigui_ths
LOCAL_MODULE_TAGS :=optional
LOCAL_MODULE_GEN_SHARED_FILES= src/.libs/libminigui_ths.so            \
			       src/.libs/libminigui_ths-3.0.so.12     \
			       src/.libs/libminigui_ths-3.0.so.12.0.0

LOCAL_MODULE_CONFIG_FILES:= config.log
LOCAL_MODULE_CONFIG:=./configure --enable-shared --host=$(DEVICE_COMPILER_PREFIX)


LOCAL_MODULE_COMPILE=make -j$(MAKE_JLEVEL)
LOCAL_MODULE_COMPILE_CLEAN=make distclean

LOCAL_EXPORT_C_INCLUDE_FILES:= include/common.h          \
			       include/minigui.h         \
			       include/gdi.h             \
			       include/window.h		 \
			       include/endianrw.h	 \
			       mgconfig.h

include $(BUILD_THIRDPART)



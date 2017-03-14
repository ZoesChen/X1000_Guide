LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE=alsa-utils
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_GEN_BINRARY_FILES := .install/usr/bin/aplay	\
				  .install/usr/bin/arecord	\
				  .install/usr/bin/amixer

LOCAL_MODULE_CONFIG_FILES:=Makefile

LOCAL_MODULE_CONFIG=mkdir .install; ./configure --host=$(DEVICE_COMPILER_PREFIX) --with-alsa-prefix=$(PWD)/$(OUT_DEVICE_SHARED_DIR) --with-alsa-inc-prefix=$(PWD)/$(OUT_DEVICE_INCLUDE_DIR) --without-curses --disable-alsamixer CFLAGS=-Os --disable-alsatest --disable-nls --disable-alsaloop --disable-xmlto --disable-dependency-tracking
LOCAL_MODULE_COMPILE=make -j$(MAKE_JLEVEL); make DESTDIR=$(TOP_DIR)/$(LOCAL_PATH)/.install install-strip

LOCAL_DEPANNER_MODULES := alsa-include
LOCAL_MODULE_COMPILE_CLEAN=make distclean

include $(BUILD_THIRDPART)



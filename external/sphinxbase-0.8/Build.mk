LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := sphinxbase-lib-build
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_GEN_SHARED_FILES:=\
	.install/lib/libsphinxad.so \
	.install/lib/libsphinxad.so.0 \
	.install/lib/libsphinxad.so.0.0.1 \
	.install/lib/libsphinxbase.la \
	.install/lib/libsphinxbase.so \
	.install/lib/libsphinxbase.so.1 \
	.install/lib/libsphinxbase.so.1.1.1 \
	util/libsndfile.so

LOCAL_MODULE_CONFIG_FILES := \
	Makefile \
	config.log
LOCAL_MODULE_CONFIG:=touch *;./autogen.sh --prefix=$(TOP_DIR)/$(LOCAL_PATH)/.install --host=$(DEVICE_COMPILER_PREFIX) AR=$(TARGET_AR) CC=$(TARGET_GCC) CXX=$(TARGET_GXX)  --enable-static=no --without-python --with-sysroot=$(COMPILER_PATH)/mips-linux-gnu/libc CFLAGS="--sysroot=$(COMPILER_PATH)/mips-linux-gnu/libc -I$(ABS_DEVICE_INCLUDE_DIR)" LDFLAGS="-L$(TOP_DIR)/$(LOCAL_PATH)/util -lsndfile -L$(TOP_DIR)/$(OUT_DEVICE_SHARED_DIR)"

LOCAL_MODULE_COMPILE:=make ; make install
LOCAL_MODULE_COMPILE_CLEAN :=make distclean; rm .install -rf
LOCAL_DEPANNER_MODULES := alsa-utils alsa-lib alsa-include
include $(BUILD_THIRDPART)



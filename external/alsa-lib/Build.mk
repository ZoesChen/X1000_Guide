LOCAL_PATH := $(my-dir)

#===================================================
#  build alsa lib
#
include $(CLEAR_VARS)
LOCAL_MODULE := alsa-lib-build
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_GEN_SHARED_FILES := \
								 .install/lib/libasound.so.2.0.0 \
								 .install/lib/libasound.so.2	\
								 .install/lib/libasound.so
LOCAL_MODULE_CONFIG_FILES:=Makefile \
		.install/include
LOCAL_MODULE_CONFIG=touch *;mkdir .install;./configure --prefix=''  --host=mips AR=$(TARGET_AR) CC=$(TARGET_GCC) CXX=$(TARGET_GXX) CPP=mips-linux-gnu-cpp --host=$(DEVICE_COMPILER_PREFIX) --disable-python  --enable-shared=yes --with-configdir=/usr/share --with-plugindir=/usr/share --with-pkgconfdir=/usr/share CFLAGS=-Os
LOCAL_MODULE_COMPILE=make -j$(MAKE_JLEVEL); make DESTDIR=$(TOP_DIR)/$(LOCAL_PATH)/.install install-strip
LOCAL_MODULE_COMPILE_CLEAN :=make distclean; rm .install -rf
include $(BUILD_THIRDPART)

#===================================================
# copy alsa include dir
#
include $(CLEAR_VARS)
LOCAL_MODULE :=alsa-include
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS:=DIR
LOCAL_MODULE_PATH := $(OUT_DEVICE_INCLUDE_DIR)
LOCAL_MODULE_DIR := .install/include/alsa/
LOCAL_DEPANNER_MODULES := alsa-lib-build
include $(BUILD_MULTI_COPY)
#===================================================
# copy config file to system root
#
include $(CLEAR_VARS)
LOCAL_MODULE :=alsa-lib
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)
LOCAL_COPY_FILES :=	\
		usr/share/cards/aliases.conf:.install/usr/share/cards/aliases.conf	\
		usr/share/alsa.conf:.install/usr/share/alsa.conf	\
		usr/share/pcm/default.conf:.install/usr/share/pcm/default.conf	\
		usr/share/pcm/iec958.conf:.install/usr/share/pcm/iec958.conf	\
		usr/share/pcm/dmix.conf:.install/usr/share/pcm/dmix.conf	\
		usr/share/pcm/surround41.conf:.install/usr/share/pcm/surround41.conf	\
		usr/share/pcm/surround40.conf:.install/usr/share/pcm/surround40.conf	\
		usr/share/pcm/surround51.conf:.install/usr/share/pcm/surround51.conf	\
		usr/share/pcm/center_lfe.conf:.install/usr/share/pcm/center_lfe.conf	\
		usr/share/pcm/rear.conf:.install/usr/share/pcm/rear.conf	\
		usr/share/pcm/surround50.conf:.install/usr/share/pcm/surround50.conf	\
		usr/share/pcm/dsnoop.conf:.install/usr/share/pcm/dsnoop.conf	\
		usr/share/pcm/front.conf:.install/usr/share/pcm/front.conf	\
		usr/share/pcm/hdmi.conf:.install/usr/share/pcm/hdmi.conf	\
		usr/share/pcm/modem.conf:.install/usr/share/pcm/modem.conf	\
		usr/share/pcm/surround71.conf:.install/usr/share/pcm/surround71.conf	\
		usr/share/pcm/side.conf:.install/usr/share/pcm/side.conf	\
		usr/share/pcm/dpl.conf:.install/usr/share/pcm/dpl.conf
LOCAL_DEPANNER_MODULES := alsa-lib-build
include $(BUILD_MULTI_COPY)


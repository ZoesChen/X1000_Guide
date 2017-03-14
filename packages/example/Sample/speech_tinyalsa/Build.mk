LOCAL_PATH := $(my-dir)
#==================================================
# build aiengine
include $(CLEAR_VARS)

use_ula = 0

ifeq ($(use_ula), 1)
LOCAL_MODULE=aiengine-tinyalsa-4mic
else
LOCAL_MODULE=aiengine-tinyalsa-1mic
endif

LOCAL_MODULE_TAGS:=optional

LOCAL_SRC_FILES:= ./src/main.c \
				./src/local_aec.c \
				./src/cloud_sem.c \
				./src/cloud_syn.c \
				./src/cJSON.c \
				./src/alsa_sound_dev.c

LOCAL_CFLAGS := -O3 -fpic -Werror -g
LOCAL_LDFLAGS := -Wl,-rpath=$(OUT_DEVICE_SHARED_DIR)

LOCAL_LDLIBS := -lc -lpthread -lm -ltinyalsa

LOCAL_C_INCLUDES:= include

LOCAL_MODULE_PATH:=$(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)/$(LOCAL_MODULE)

ifeq ($(use_ula), 1)
LOCAL_LDLIBS += -lecho_wakeup -laiengine -lula
else
LOCAL_LDLIBS += -lecho_wakeup -laiengine
endif

LOCAL_DEPANNER_MODULES := aiengine-lib create_data libtinyalsa

include $(BUILD_EXECUTABLE)

#==================================================
# copy the lib
include $(CLEAR_VARS)
LOCAL_MODULE := aiengine-tinyalsa-lib
LOCAL_MODULE_TAGS :=optional
LOCAL_MODULE_PATH :=$(OUT_DEVICE_SHARED_DIR)
LOCAL_PREBUILT_LIBS := libaiengine.so:./lib/libaiengine.so libecho_wakeup.so:./lib/libecho_wakeup.so libula.so:./lib/x1000-4mic/libula.so
include $(BUILD_MULTI_PREBUILT)


#==================================================
# copy the bin
include $(CLEAR_VARS)
ifeq ($(use_ula), 1)
LOCAL_MODULE=aiengine-tinyalsa-4mic
else
LOCAL_MODULE=aiengine-tinyalsa-1mic
endif
LOCAL_MODULE_TAGS :=optional
LOCAL_MODULE_CLASS:=DIR
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)/$(LOCAL_MODULE)
LOCAL_MODULE_DIR := bin
include $(BUILD_MULTI_PREBUILT)

#==================================================
# create the data for filesystem
include $(CLEAR_VARS)
LOCAL_MODULE=create_data-tinyalsa
LOCAL_MODULE_TAGS :=optional
LOCAL_MODULE_CLASS:=DIR
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/usr
LOCAL_MODULE_DIR := data
include $(BUILD_MULTI_PREBUILT)

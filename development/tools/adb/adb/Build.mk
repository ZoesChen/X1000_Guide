# Copyright 2005 The Android Open Source Project
#
# Android.mk for adb
#

LOCAL_PATH:= $(call my-dir)

# adb host tool
# =========================================================
# Default to a virtual (sockets) usb interface
# =========================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	adb.c \
	backup_service.c \
	fdevent.c \
	transport.c \
	transport_local.c \
	transport_usb.c \
	adb_auth_client.c \
	sockets.c \
	services.c \
	file_sync_service.c \
	jdwp_service.c \
	framebuffer_service.c \
	remount_service.c \
	usb_linux_client.c \
	log_service.c	\
	utils.c

LOCAL_CFLAGS := -O2 -DADB_HOST=0 -Wall -Wno-unused-parameter
LOCAL_CFLAGS += -D_XOPEN_SOURCE -D_GNU_SOURCE -DADB_TRACING -DALLOW_ADBD_ROOT -D__LINUX__ -DALLOW_ADBD_ROOT=1
LOCAL_LDLIBS = -lpthread -lc
LOCAL_C_INCLUDES := ./ ../include
LOCAL_MODULE := adbd
LOCAL_MODULE_TAGS := eng

LOCAL_STATIC_LIBRARIES := libcutils.a
LOCAL_SHARED_LIBRARIES :=libdlog
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/sbin
include $(BUILD_EXECUTABLE)




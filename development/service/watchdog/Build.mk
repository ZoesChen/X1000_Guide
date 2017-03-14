# Copyright 2005 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	watchdogd.c

LOCAL_MODULE:= watchdogd
LOCAL_C_INCLUDES := ./
LOCAL_DEPANNER_MODULES := klog_h
LOCAL_CFLAGS :=  -Wall -O2
LOCAL_LDLIBS := -lc
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libcutils.a
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/sbin
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := watchdogd_sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/etc/init.d
LOCAL_COPY_FILES := watchdogd.sh:watchdogd.sh S90watchdog:S90watchdog

include $(BUILD_MULTI_PREBUILT)

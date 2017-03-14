#
# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(my-dir)


include $(CLEAR_VARS)
COMMON_SRC_FILES := \
	array.c \
	hashmap.c \
	atomic.c \
	native_handle.c \
	buffer.c \
	socket_inaddr_any_server.c \
	socket_local_client.c \
	socket_local_server.c \
	socket_loopback_client.c \
	socket_loopback_server.c \
	socket_network_client.c \
	sockets.c \
	config_utils.c \
	cpu_info.c \
	load_file.c \
	list.c \
	open_memstream.c \
	strdup16to8.c \
	strdup8to16.c \
	record_stream.c \
	process_name.c \
	properties.c \
	qsort_r_compat.c \
	threads.c \
	sched_policy.c \
	iosched_policy.c \
	str_parms.c \
	debugger.c

LOCAL_SRC_FILES := android_reboot.c \
	klog.c \
	partition_utils.c \
	qtaguid.c \
	trace.c \
	memory.c
LOCAL_MODULE := libcutils
LOCAL_SRC_FILES := $(COMMON_SRC_FILES) $(LOCAL_SRC_FILES)
LOCAL_C_INCLUDES := bionic_time.h include
LOCAL_SHARED_LIBRARIES := libdlog.a
LOCAL_CFLAGS += -DANDROID_SMP=0 -DHAVE_PTHREADS -DHAVE_SYS_UIO_H -DFAKE_LOG_DEVICE=1 -fPIC
LOCAL_MODULE_TAGS :=optional
include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := klog_h
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(OUT_DEVICE_INCLUDE_DIR)
LOCAL_COPY_FILES := cutils/klog.h:include/cutils/klog.h
include $(BUILD_PREBUILT)


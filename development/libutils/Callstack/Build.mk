LOCAL_PATH:= $(call my-dir)

commonSources:= \
	ProcessCallStack.cpp \
	Printer.cpp \
	Static.cpp \
	String8.cpp \
	Unicode.cpp \
	CallStack.cpp \
	VectorImpl.cpp \
	String16.cpp \
	SharedBuffer.cpp \
	logd_write.c \

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= $(commonSources)
LOCAL_MODULE:= libprocesscallstack
LOCAL_MODULE_TAGS := optional
LOCAL_CPPFLAGS := -std=gnu++11
LOCAL_SHARED_LIBRARIES := libdlog.so libbacktrace
LOCAL_DEPANNER_MODULES := dlog libbacktrace
LOCAL_MODULE_GEN_SHARED_FILES:=libprocesscallstack.so
LOCAL_C_INCLUDES := include
LOCAL_CPP_INCLUDES := include
LOCAL_CFLAGS += -Wa,-mips32r2 -O2 -G 0 -Wall  -fPIC -shared -D_GNU_SOURCE=1 -DHAVE_CONFIG_H
LOCAL_LDLIBS += -lc -lstdc++ -lpthread -lrt -lbacktrace -ldlog
include $(BUILD_SHARED_LIBRARY)


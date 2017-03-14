LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE=libipc_socket
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= socket_local_client.c socket_local_server.c

LOCAL_EXPORT_C_INCLUDE_FILES:=  socket_local.h
#LOCAL_C_INCLUDES := include
LOCAL_CFLAGS := -O2 -Wall -fPIC
include $(BUILD_SHARED_LIBRARY)


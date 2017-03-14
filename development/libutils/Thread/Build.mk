LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE=libpthread_demo
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= src/atomic.cpp 	\
		  src/Condition.cpp	\
		  src/c_pthread.cpp	\
		  src/mThread.cpp	\
		  src/Mutex.cpp  	\
		  src/RefBase.cpp

LOCAL_C_INCLUDES:= include
#LOCAL_EXPORT_C_INCLUDE_FILES:=  socket_local.h
LOCAL_CPPFLAGS := -O2 -Wall -fPIC -fpermissive

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := thread_c_demo
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := thread_c_demo.c
LOCAL_C_INCLUDES:= include
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)
LOCAL_CFLAGS := -g  -Wall -Wextra -pthread
LOCAL_LDFLAGS := -lpthread_demo
LOCAL_LDLIBS := -lpthread -lstdc++ -lc
LOCAL_SHARED_LIBRARIES := libpthread_demo.so

include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)

LOCAL_MODULE := thread_c++_demo
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := thread_c++_demo.cpp
LOCAL_C_INCLUDES:= include
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)
LOCAL_CPPFLAGS := -g  -Wall -Wextra -pthread -fpermissive
LOCAL_LDFLAGS := -lpthread_demo
LOCAL_LDLIBS = -lpthread -lc -lstdc++
LOCAL_SHARED_LIBRARIES := libpthread_demo.so

include $(BUILD_EXECUTABLE)




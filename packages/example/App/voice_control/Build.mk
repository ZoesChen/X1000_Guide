LOCAL_PATH := $(my-dir)

#====================
# iot_client
#
include $(CLEAR_VARS)
LOCAL_MODULE=iot_client
LOCAL_MODULE_TAGS:=eng
LOCAL_SRC_FILES:= \
	Client/Client.cpp \
	Client/gpio_ctrl.cpp \
	common/cjson.c   \
	common/ActionBase.cpp \
	common/ActionOK.cpp \
	common/ActionRegister.cpp \
	common/ActionRegisterAck.cpp \
	common/ActionSendSpeechCommand.cpp \
	common/ClientConfig.cpp \
	common/IniParser.cpp

LOCAL_LDLIBS := -lstdc++ -lrt -lpthread -lm -lc -lgpio
LOCAL_C_INCLUDES := common
LOCAL_DEPANNER_MODULES:= libgpio
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/sbin
include $(BUILD_EXECUTABLE)
#===================================================
# iot_server
#
include $(CLEAR_VARS)
LOCAL_MODULE=iot_server
LOCAL_MODULE_TAGS:=eng
LOCAL_SRC_FILES:=  \
	Server/Server.cpp \
	common/cjson.c \
	Server/CClient.cpp \
	Server/ActionRegisterHandler.cpp \
	common/ActionBase.cpp \
	common/ActionOK.cpp \
	common/ActionRegister.cpp \
	common/ActionRegisterAck.cpp \
	common/ClientConfig.cpp \
	common/ActionHandlerBase.cpp

LOCAL_LDLIBS := -lstdc++ -lrt -lpthread -lm -lc
LOCAL_C_INCLUDES := common Server
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/sbin
include $(BUILD_EXECUTABLE)
#====================
# iot_speechClient
#
include $(CLEAR_VARS)
LOCAL_MODULE=iot_speechClient
LOCAL_MODULE_TAGS:=eng
LOCAL_SRC_FILES:= \
	speechCommandClient/srEngine.cpp \
	speechCommandClient/speechClient.cpp \
	speechCommandClient/scClient.cpp \
	common/cjson.c \
	common/ActionBase.cpp \
	common/ActionOK.cpp \
	common/ActionRegister.cpp \
	common/ActionRegisterAck.cpp \
	common/ActionSendSpeechCommand.cpp \
	common/ClientConfig.cpp

LOCAL_LDLIBS := -lstdc++ -lrt -lpthread -lm -lc
LOCAL_C_INCLUDES := common speechCommandClient
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/sbin
include $(BUILD_EXECUTABLE)
#-------
# copy file
#
include $(CLEAR_VARS)
LOCAL_MODULE :=iot_config
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)
LOCAL_COPY_FILES := \
	sbin/sr_cmd.conf:./speechCommandClient/sr_cmd.conf \
	sbin/config.ini:./Client/config.ini
include $(BUILD_MULTI_PREBUILT)


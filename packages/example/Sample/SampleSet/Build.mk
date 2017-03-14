LOCAL_PATH := $(my-dir)
#====copy runtest.sh ==========================================
include $(CLEAR_VARS)
LOCAL_MODULE := RuntestScript
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)
LOCAL_COPY_FILES := runtest.sh:runtest.sh
LOCAL_DEPANNER_MODULES := bash
include $(BUILD_MULTI_COPY)
#= copy bash====================================================
include $(CLEAR_VARS)
LOCAL_MODULE := bash
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/bin/
LOCAL_COPY_FILES := $(LOCAL_MODULE):$(LOCAL_MODULE)
include $(BUILD_MULTI_COPY)

#===wifi connect Demo===============================================
include $(CLEAR_VARS)
LOCAL_MODULE := WificonnectScript
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)
LOCAL_COPY_FILES := Wifi-connect-Demo.sh:Wifi-connect-Demo.sh
include $(BUILD_MULTI_COPY)
#
#===DMIC test Demo=================================================
include $(CLEAR_VARS)
LOCAL_MODULE := DmicScript
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)
LOCAL_COPY_FILES := Dmic-Demo.sh:Dmic-Demo.sh
include $(BUILD_MULTI_COPY)
#
#===TF CARD demo===================================================
include $(CLEAR_VARS)
LOCAL_MODULE := TfcardcopyScript
LOCAL_MODULE_TAGS :=optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)
LOCAL_COPY_FILES :=Tfcard-Demo.sh:Tfcard-Demo.sh
include $(BUILD_MULTI_COPY)
#
#===AMIC test Demo==================================================
include $(CLEAR_VARS)
LOCAL_MODULE := AmicScript
LOCAL_MODULE_TAGS :=optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)
LOCAL_COPY_FILES := Amic-Demo.sh:Amic-Demo.sh
LOCAL_DEPANNER_MODULES:= tinycap tinyplay tinymix
include $(BUILD_MULTI_COPY)
#
#===nand nor copy test Demo==================================================
include $(CLEAR_VARS)
LOCAL_MODULE := RWstabilityTestScript
LOCAL_MODULE_TAGS :=optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)
LOCAL_COPY_FILES := Nand-Nor-Test.sh
include $(BUILD_MULTI_COPY)
#
#===CLEAN test Demo================================================
include $(CLEAR_VARS)
LOCAL_MODULE := CleanfileScript
LOCAL_MODULE_TAGS :=optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)/$(TARGET_TESTSUIT_DIR)
LOCAL_COPY_FILES := Clean-testfile-Demo.sh:Clean-testfile-Demo.sh
include $(BUILD_MULTI_COPY)
#


__local_export_compiler_path := $(if $(EXPORT_COMPILER_PATH),$(EXPORT_COMPILER_PATH);,)

__local_module_path:=$(if $(LOCAL_MODULE_PATH),$(LOCAL_MODULE_PATH),$(OUT_$(BUILD_MODE)_$(1)_DIR))
__module_install:=$(__local_module_path)/$(notdir $(THIRDPART_MODULE))
__module:=$(LOCAL_PATH)/$(THIRDPART_MODULE)
$(__module_install):PRIVATE_CONFIG_COMMAND:=$(__local_export_compiler_path)cd $(LOCAL_PATH);$(LOCAL_MODULE_CONFIG)
$(__module_install):PRIVATE_COMPILER_COMMAND:=$(__local_export_compiler_path)cd $(LOCAL_PATH);$(LOCAL_MODULE_COMPILE)

$(__module_install):$(__module) $(ALL_INCLUDE_DEPANNER)
	$(hide) mkdir -p $(dir $(@))
	$(hide) cp -d $< $@
$(__module):$(THIRDPART_DEPANNER_MODULES) $(THIRDPART_MODULE_CONFIG_FILE) $(THIRDPART_LOCAL_SHARED_LIBRARIES) $(THIRDPART_LOCAL_STATIC_LIBRARIES)
	$(PRIVATE_COMPILER_COMMAND)

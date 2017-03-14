###########################################################
## Standard rules for copying files that are prebuilt
##
## Additional inputs from base_rules.make:
## None.
##
###########################################################

__local_path := $(LOCAL_PATH)
include $(BUILD_SYSTEM)/choose_src_target_path.mk

$(call choose_src_target_path, $(LOCAL_COPY_FILES))

__out_module_file := $(LOCAL_MODULE_PATH)/$(LOCAL_TARGET_FILES)
__local_module_file :=$(LOCAL_PATH)/$(LOCAL_SRC_FILES)

CLEAN_DEP_FILES.$(LOCAL_MODULE_BUILD) += $(__out_module_file)

include $(BUILD_SYSTEM)/module_install.mk

ifneq ($(LOCAL_FILTER_MODULE),)
ALL_PREBUILT_MODULES += $(__out_module_file)
endif

include $(BUILD_SYSTEM)/mma_build.mk


ifeq ($(LOCAL_MODULE_CLASS),DIR)
__local_module_dir := $(__out_module_file).dir
$(LOCAL_MODULE_BUILD):$(__local_module_dir)
$(__local_module_dir):$(__local_module_file)
	$(hide) mkdir -p $(subst .dir,,$@)
	$(hide) cp -adrf $< $(dir $@)
else
$(LOCAL_MODULE_BUILD):$(__out_module_file)
$(__out_module_file):$(__local_module_file)
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -adr $< $@
endif

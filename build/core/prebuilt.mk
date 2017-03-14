###########################################################
## Standard rules for copying files that are prebuilt
##
## Additional inputs from base_rules.make:
## None.
##
###########################################################

LOCAL_MODULE_BUILD := $(LOCAL_MODULE)
ALL_MODULES_CLEAN += $(LOCAL_MODULE)
__local_path := $(LOCAL_PATH)

ifneq ($(strip x$(LOCAL_DEPANNER_MODULES)),x)
$(warning "Prebuilt can't have depanner modules.")
endif

include $(BUILD_SYSTEM)/choose_src_target_path.mk

$(call choose_src_target_path, $(LOCAL_COPY_FILES))

__out_module_file := $(LOCAL_MODULE_PATH)/$(LOCAL_TARGET_FILES)
__local_module_file :=$(LOCAL_PATH)/$(LOCAL_SRC_FILES)

CLEAN_DEP_FILES.$(LOCAL_MODULE_BUILD) := $(__out_module_file)
$(LOCAL_MODULE_BUILD)-clean:PRIVATE_MODULE_FILES_CLEAN:=$(CLEAN_DEP_FILES.$(LOCAL_MODULE_BUILD))

include $(BUILD_SYSTEM)/module_install.mk

ifneq ($(LOCAL_FILTER_MODULE),)
ALL_PREBUILT_MODULES += $(__out_module_file)
endif

include $(BUILD_SYSTEM)/mma_build.mk

$(LOCAL_MODULE_BUILD):$(__out_module_file)

$(__out_module_file):$(__local_module_file)
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -adr $< $@

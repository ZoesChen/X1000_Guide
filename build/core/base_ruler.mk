#
# This file is processed by the compiler dependent'macro
#

BUILD_MODE:=
ifeq ($(strip x$(HOST)),xtrue)
BUILD_MODE:=HOST
PREFIX:=HOST-
else
BUILD_MODE:=DEVICE
PREFIX:=
endif
LOCAL_MODULE_BUILD:=$(PREFIX)$(LOCAL_MODULE)
PREFIX_LOCAL_DEPANNER_MODULES :=$(if $(LOCAL_DEPANNER_MODULES),$(addprefix $(PREFIX),$(LOCAL_DEPANNER_MODULES)),)

ALL_MODULES_CLEAN += $(PREFIX)$(LOCAL_MODULE)

__local_compile:=$(BUILD_MODE)_COMPILER
__local_out_obj_dir:=$(OUT_$(BUILD_MODE)_OBJ_DIR)
__local_out_static_dir:=$(OUT_$(BUILD_MODE)_STATIC_DIR)

__global_target_includes := $(OUT_$(BUILD_MODE)_INCLUDE_DIR)
__global_target_cflags := $(GLOBAL_$(BUILD_MODE)_CFLAGS)
__local_module_c_include:=$(addprefix $(LOCAL_PATH)/,$(LOCAL_C_INCLUDES) $(LOCAL_CPP_INCLUDES))

define BUILD_TYPE_FILE
$(2):PRIVATE_BUILD:=$($(__local_compile).$(1))
$(2):PRIVATE_MODULE_C_INCLUDES:=$(__local_module_c_include)
$(2):PRIVATE_MODULE_CFLAGS:=$(LOCAL_CFLAGS) $(LOCAL_CPPFLAGS)
$(2):PRIVATE_TARGET_INCLUDES:=$(__global_target_includes)
$(2):PRIVATE_TARGET_CFLAGS:=$(__global_target_cflags)
endef

define SPLIT_FILES
$(eval _src_files := $(filter %.$(1),$(LOCAL_SRC_FILES)))
$(eval MODULE_OBJ_FILES.$(1) := $(addprefix $(__local_out_obj_dir)/$(3)/,$(_src_files:.$(1)=.o)))
$(foreach f,$(filter %.$(1),$(LOCAL_SRC_FILES)), \
	$(eval $(call BUILD_TYPE_FILE,$(1),$(__local_out_obj_dir)/$(3)/$(f:.$(1)=.o),$(LOCAL_PATH)/$(f))) \
	)
endef

$(eval LOCAL_MODULE_OBJS:=$(foreach v,$(BUILD_TYPE), \
	$(eval $(call SPLIT_FILES,$(v),$(LOCAL_MODULE),$(LOCAL_PATH))) \
	$(MODULE_OBJ_FILES.$(v)) \
	))
$(LOCAL_MODULE_BUILD):PRIVATE_LOCAL_LDFLAGS:=$(LOCAL_LDFLAGS)
$(LOCAL_MODULE_BUILD):PRIVATE_LOCAL_LDLIBS:=$(LOCAL_LDLIBS)

$(LOCAL_MODULE_BUILD):PRIVATE_STATIC_LIBRARIES:=$(if $(LOCAL_STATIC_LIBRARIES),$(addprefix $(__local_out_static_dir)/,$(LOCAL_STATIC_LIBRARIES)),)
$(LOCAL_MODULE_BUILD):PRIVATE_SHARED_LIBRARIES:=$(if $(LOCAL_SHARED_LIBRARIES),$(addprefix -l,$(subst lib,,$(basename $(LOCAL_SHARED_LIBRARIES)))),)



LOCAL_OUT_MODULE_BUILD:=$(__local_out_obj_dir)/$(LOCAL_PATH)/$(LOCAL_MODULE)$(LOCAL_EXT_NAME)

__local_libraries_modules := $(basename $(LOCAL_STATIC_LIBRARIES)) $(basename $(LOCAL_SHARED_LIBRARIES))

define DEPANNER_LIBS
$(1): | $(2)
endef

ifeq ($(strip x$(SDK_BUILD)),x)
__local_libraries_deppennar:=$(if $(__local_libraries_modules), $(eval $(call DEPANNER_LIBS ,$(LOCAL_OUT_MODULE_BUILD),$(basename $(__local_libraries_modules)))),)
endif

ifeq ($(strip x$(SDK_BUILD)),x)
ifeq (x$(strip $(LOCAL_MODULE_PATH)),x)
LOCAL_INSTALL_MODULE:=$(LOCAL_OUT_INSTALL_DIR)/$(LOCAL_MODULE)$(LOCAL_EXT_NAME)
else
LOCAL_INSTALL_MODULE:= $(LOCAL_MODULE_PATH)/$(LOCAL_MODULE)$(LOCAL_EXT_NAME)
endif
else
LOCAL_INSTALL_MODULE:=$(LOCAL_PATH)/$(LOCAL_MODULE)$(LOCAL_EXT_NAME)
endif

CLEAN_DEP_MODULES.$(LOCAL_MODULE_BUILD):=$(sort $(basename $(LOCAL_STATIC_LIBRARIES)) $(basename $(LOCAL_SHARED_LIBRARIES)) $(PREFIX_LOCAL_DEPANNER_MODULES))
CLEAN_DEP_FILES.$(LOCAL_MODULE_BUILD):= $(LOCAL_INSTALL_MODULE)

$(LOCAL_MODULE_BUILD)-clean:PRIVATE_MODULE_FILES_CLEAN:=$(CLEAN_DEP_FILES.$(LOCAL_MODULE_BUILD))
$(LOCAL_MODULE_BUILD)-clean:PRIVATE_MODULE_DIR_CLEAN:=$(__local_out_obj_dir)/$(LOCAL_PATH)

ifeq ($(strip x$(SDK_BUILD)),x)
include $(BUILD_SYSTEM)/module_install.mk
include $(BUILD_SYSTEM)/mma_build.mk
endif


ALL_MODULE_PATH.$(LOCAL_MODULE) := $(LOCAL_PATH)
ALL_DOC_FILES.$(LOCAL_MODULE) := $(addprefix $(LOCAL_PATH)/,$(LOCAL_DOC_FILES))

ifeq ($(strip x$(SDK_BUILD)),x)
ALL_MODULES += $(LOCAL_FILTER_MODULE)
ALL_BUILD_MODULES += $(LOCAL_FILTER_MODULE)
ifneq ($(strip x$(LOCAL_FILTER_MODULE)),x)
ALL_BUILD_MODULES += $(CLEAN_DEP_MODULES.$(LOCAL_MODULE_BUILD))
endif
else
ALL_MODULES += $(LOCAL_MODULE_BUILD)
endif

define dump_my_var
$(info "base ruler var dump:")
$(info LOCAL_MODULE_BUILD: = $(LOCAL_MODULE_BUILD))
$(info LOCAL_OUT_MODULE_BUILD: = $(LOCAL_OUT_MODULE_BUILD))
$(info LOCAL_INSTALL_MODULE: = $(LOCAL_INSTALL_MODULE))
endef

define tranform-c-to-o
@echo $@ "->" $^
$(call host-mkdir,$(dir $@))
$(hide) $(PRIVATE_BUILD) \
		$(addprefix -I , $(PRIVATE_TARGET_INCLUDES)) \
		$(addprefix -I , $(PRIVATE_MODULE_C_INCLUDES)) \
		-c \
		$(PRIVATE_TARGET_CFLAGS) \
		$(PRIVATE_MODULE_CFLAGS) \
		-MD -MF $(@:.o=.d) -c -o $@ $<
endef

ifeq ($(strip x$(SDK_BUILD)),x)
include $(BUILD_SYSTEM)/install_inc.mk

__local_depanner := $(if $(strip $(ALL_INCLUDE_DEPANNER)), $(strip $(ALL_INCLUDE_DEPANNER)),) \
	$(if $(strip $(PREFIX_LOCAL_DEPANNER_MODULES)), $(strip $(PREFIX_LOCAL_DEPANNER_MODULES)),)
__local_depanner := $(if $(strip $(__local_depanner)), | $(strip $(__local_depanner)),)
else
__local_depanner :=
endif

ALL_INSTALL_MODULES += $(LOCAL_INSTALL_MODULE)
$(LOCAL_MODULE_BUILD): $(LOCAL_INSTALL_MODULE)

$(MODULE_OBJ_FILES.c):$(__local_out_obj_dir)/$(LOCAL_PATH)/%.o:$(LOCAL_PATH)/%.c $(__local_depanner)
	$(tranform-c-to-o)

$(MODULE_OBJ_FILES.cc):$(__local_out_obj_dir)/$(LOCAL_PATH)/%.o:$(LOCAL_PATH)/%.cc $(__local_depanner)
	$(tranform-c-to-o)
$(MODULE_OBJ_FILES.cpp):$(__local_out_obj_dir)/$(LOCAL_PATH)/%.o:$(LOCAL_PATH)/%.cpp $(__local_depanner)
	$(tranform-c-to-o)

$(MODULE_OBJ_FILES.S):$(__local_out_obj_dir)/$(LOCAL_PATH)/%.o:$(LOCAL_PATH)/%.S $(__local_depanner)
	$(tranform-c-to-o)


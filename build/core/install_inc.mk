
PREFIX_INCLUDE_FILES_TAR:= $(addprefix $(OUT_$(BUILD_MODE)_INCLUDE_DIR)/,\
			$(subst include/,,$(filter include/%,$(LOCAL_EXPORT_C_INCLUDE_FILES))))
INCLUDE_FILES_TAR:=$(addprefix $(OUT_$(BUILD_MODE)_INCLUDE_DIR)/,\
			$(filter-out include/%,$(LOCAL_EXPORT_C_INCLUDE_FILES)))

PREFIX_INCLUDE_DIRS_TAR:= $(addprefix $(OUT_$(BUILD_MODE)_INCLUDE_DIR)/,\
			$(subst include/,,$(filter include/%,$(LOCAL_EXPORT_C_INCLUDE_DIRS))))

INCLUDE_DIRS_TAR:= $(addprefix $(OUT_$(BUILD_MODE)_INCLUDE_DIR)/,\
	$(filter-out include/%,$(LOCAL_EXPORT_C_INCLUDE_DIRS)))


ALL_INCLUDE_DEPANNER:= $(strip $(PREFIX_INCLUDE_FILES_TAR) $(INCLUDE_FILES_TAR) $(PREFIX_INCLUDE_DIRS_TAR) $(INCLUDE_DIRS_TAR))

$(PREFIX_INCLUDE_FILES_TAR):$(OUT_$(BUILD_MODE)_INCLUDE_DIR)/%.h:$(LOCAL_PATH)/include/%.h
	$(hide)mkdir -p $(dir $@)
	$(hide)cp -df $< $@

$(INCLUDE_FILES_TAR):$(OUT_$(BUILD_MODE)_INCLUDE_DIR)/%.h:$(LOCAL_PATH)/%.h
	$(hide)mkdir -p $(dir $@)
	$(hide)cp -df $< $@

$(PREFIX_INCLUDE_DIRS_TAR):$(OUT_$(BUILD_MODE)_INCLUDE_DIR)/%:$(LOCAL_PATH)/include/%
	$(hide)mkdir -p $(dir $@)
	$(hide)cp -drf $< $(dir $@)

$(INCLUDE_DIRS_TAR):$(OUT_$(BUILD_MODE)_INCLUDE_DIR)/%:$(LOCAL_PATH)/%
	$(hide)mkdir -p $(dir $@)
	$(hide)cp -drf $< $(dir $@)

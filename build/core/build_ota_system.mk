#if the model has system,use it;or use the common fs.
#test function: exit ----> 0
#				no	 ----> 1
TARGET_FS:= $(TARGET_COMMON_FS_PATH)

define patch_target_fs
$(if $(shell if [ -f $(TARGET_MODEL_PATH)/system_rm_redundancyOTA.sh ];then echo True;fi),$(shell $(TARGET_MODEL_PATH)/system_rm_redundancyOTA.sh))
$(if $(shell if [ -d $(TARGET_MODEL_PATH)/systemOTA.patch ];then echo True;fi),$(shell cp -rf $(TARGET_MODEL_PATH)/systemOTA.patch/* $(TARGET_INSTALL_PATH)/systemOTA))
$(if $(shell if [ -f $(TARGET_MODEL_PATH)/system_rm_redundancy.sh ];then echo True;fi),$(shell $(TARGET_MODEL_PATH)/system_rm_redundancy.sh))
$(if $(shell if [ -d $(TARGET_MODEL_PATH)/system.patch ];then echo True;fi),$(shell cp -rf $(TARGET_MODEL_PATH)/system.patch/* $(TARGET_FS_BUILD)))
endef


rootfs:$(TARGET_INSTALL_PATH)/systemOTA $(TARGET_FS_BUILD)
	$(patch_target_fs)
$(TARGET_INSTALL_PATH)/systemOTA:
	mkdir -p $@
	cp -adr $(TARGET_FS)/* $@
$(TARGET_FS_BUILD):
	mkdir -p $@
	cp -adr $(TARGET_FS)/* $@


systemimage:$(OUT_IMAGE_DIR)/update_fs.cramfs $(OUT_IMAGE_DIR)/user_fs.cramfs
$(OUT_IMAGE_DIR)/update_fs.$(FILE_SYSTEM_TYPE):$(ALL_MODULES)
	$(if $(shell if [ -f $(TARGET_MODEL_PATH)/system_patchOTA.sh ];then echo True;fi),$(shell $(TARGET_MODEL_PATH)/system_patchOTA.sh))
ifeq ($(strip $(FILE_SYSTEM_TYPE)),cramfs)
	mkfs.cramfs $(TARGET_INSTALL_PATH)/systemOTA $@
endif

$(OUT_IMAGE_DIR)/user_fs.$(FILE_SYSTEM_TYPE):$(ALL_MODULES)
	$(if $(shell if [ -f $(TARGET_MODEL_PATH)/system_patch.sh ];then echo True;fi),$(shell $(TARGET_MODEL_PATH)/system_patch.sh))
ifeq ($(strip $(FILE_SYSTEM_TYPE)),cramfs)
	mkfs.cramfs $(TARGET_FS_BUILD) $@
endif

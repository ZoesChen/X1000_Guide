define build_filesystem_to_jffs2
mkfs.jffs2 -e $(ROOTFS_JFFS2_NORFLASH_ERASESIZE) \
			-p $(ROOTFS_JFFS2_SIZE) \
			-d $(TARGET_FS_BUILD) \
			-o $@
endef
define build_filesystem_to_ubi
mkfs.ubifs -e $(ROOTFS_UBIFS_LEBSIZE) \
			-c $(ROOTFS_UBIFS_MAXLEBCNT) \
			-m $(ROOTFS_UBIFS_MINIOSIZE) \
			-d $(TARGET_FS_BUILD) \
			-o $@
endef

define build_filesystem_to_ext4
$(MKEXT4)  -N $(ROOTFS_EXT4_INODES)  \
			-b $(ROOTFS_EXT4_BLOCK) \
			-d $(TARGET_FS_BUILD) \
			-o $@
endef

#if the model has system,use it;or use the common fs.
#test function: exist ----> 0
#               no   ----> 1
#TARGET_FS:= $(if $(shell test -d $(TARGET_FS_SRC_PATH)),$(TARGET_FS_SRC_PATH),$(TARGET_COMMON_FS_PATH))
TARGET_FS:= $(TARGET_COMMON_FS_PATH)

define patch_target_fs
$(if $(shell if [ -f $(TARGET_MODEL_PATH)/system_rm_redundancy.sh ];then echo True;fi),$(shell $(TARGET_MODEL_PATH)/system_rm_redundancy.sh))
$(if $(shell if [ -d $(TARGET_MODEL_PATH)/system.patch ];then echo True;fi),$(shell cp -rf $(TARGET_MODEL_PATH)/system.patch/* $(TARGET_FS_BUILD)))
endef


rootfs:$(TARGET_FS_BUILD)
	$(patch_target_fs)
$(TARGET_FS_BUILD):
	mkdir -p $@
	cp -adr $(TARGET_FS)/* $@

systemimage:$(OUT_IMAGE_DIR)/system.$(FILE_SYSTEM_TYPE) moduledesc
$(OUT_IMAGE_DIR)/system.$(FILE_SYSTEM_TYPE):$(ALL_MODULES)
	$(if $(shell if [ -f $(TARGET_MODEL_PATH)/system_patch.sh ];then echo True;fi),$(shell $(TARGET_MODEL_PATH)/system_patch.sh))
ifeq ($(strip $(FILE_SYSTEM_TYPE)),jffs2)
	$(build_filesystem_to_jffs2)
endif
ifeq ($(strip $(FILE_SYSTEM_TYPE)),ubi)
	$(build_filesystem_to_ubi)
endif

ifeq ($(strip $(FILE_SYSTEM_TYPE)),ext4)
	$(build_filesystem_to_ext4)
endif

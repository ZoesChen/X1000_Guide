PRODUCT:= product
UBOOT_BUILD_DIR := $(TOPDIR)u-boot
KERNEL_BUILD_DIR := $(TOPDIR)kernel
KERNEL_COFIG_FILE := $(KERNEL_BUILD_DIR)/.config
MODEL := model1

################### make -j config  #######################
MAKE_JLEVEL := 4

############### hardware config #########################
TARGET_BOOTLOADER_BOARD_NAME:=$(TARGET_DEVICE)
TARGET_PRODUCT_BOARD:=$(TARGET_DEVICE)
TARGET_BOARD_PLATFORM:= "x1000"
TARGET_BOARD_ARCH:="mips"

################  target_device config ###################
ifeq ($(strip $(TARGET_STORAGE_MEDIUM)),norflash)
#nor flash config
UBOOT_BUILD_CONFIG := halley2_v10_uImage_sfc_nor
KERNEL_BUILD_CONFIG := halley2_nor_v10_linux_defconfig
KERNEL_TARGET_IMAGE :=uImage
KERNEL_IMAGE_PATH:=arch/mips/boot

UBOOT_TARGET_FILE:= u-boot-with-spl.bin

FILE_SYSTEM_TYPE:=jffs2
ROOTFS_JFFS2_NORFLASH_ERASESIZE:= 0x8000
ROOTFS_JFFS2_SIZE:=  0xc80000

ifeq ($(strip $(TARGET_EXT_SUPPORT)),ota) #OTA
OTA:=y
MODEL:=model2
FILE_SYSTEM_TYPE:=cramfs
UBOOT_BUILD_CONFIG := halley2_v10_uImage_sfc_nor
KERNEL_IMAGE_PATH := arch/mips/boot/compressed
KERNEL_TARGET_IMAGE := xImage
endif #OTA
endif #norflash

ifeq ($(strip $(TARGET_STORAGE_MEDIUM)),spinand)
#nand flash config
UBOOT_BUILD_CONFIG :=  halley2_v10_uImage_sfc_nand
KERNEL_BUILD_CONFIG := halley2_linux_sfcnand_ubi_defconfig
KERNEL_TARGET_IMAGE :=uImage
KERNEL_IMAGE_PATH:=arch/mips/boot

UBOOT_TARGET_FILE:= u-boot-with-spl.bin

FILE_SYSTEM_TYPE:=ubi
ROOTFS_UBIFS_LEBSIZE   := 0x1f000
ROOTFS_UBIFS_MAXLEBCNT := 2048
ROOTFS_UBIFS_MINIOSIZE := 0x800

endif #spinand

ifeq ($(strip $(TARGET_STORAGE_MEDIUM)),mmc)
#mmc config
UBOOT_BUILD_CONFIG :=  halley2_v10_uImage_msc0
KERNEL_BUILD_CONFIG :=
KERNEL_TARGET_IMAGE :=uImage
KERNEL_IMAGE_PATH:=arch/mips/boot

UBOOT_TARGET_FILE:= u-boot-with-spl-mbr-gpt.bin

FILE_SYSTEM_TYPE:=ext4
ROOTFS_EXT4_INODES:=0
ROOTFS_EXT4_BLOCK:=0
endif #spinand

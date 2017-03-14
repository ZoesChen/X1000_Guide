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
UBOOT_BUILD_CONFIG := yak_xImage_nor_spl_boot
KERNEL_BUILD_CONFIG := yak_linux_defconfig
KERNEL_TARGET_IMAGE :=xImage
KERNEL_IMAGE_PATH:=arch/mips/boot/compressed

UBOOT_TARGET_FILE:= u-boot-with-spl.bin

FILE_SYSTEM_TYPE:=jffs2
ROOTFS_JFFS2_NORFLASH_ERASESIZE:= 0x8000
ROOTFS_JFFS2_SIZE:=  0xc80000

endif #norflash


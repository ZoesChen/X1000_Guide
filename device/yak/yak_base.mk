#Tools and libraries necessary by program

#The host



PRODUCT_MODULES += $(KERNEL_TARGET_IMAGE) \
		   $(UBOOT_TARGET_FILE)

TINYALSA_UTILS := tinyplay tinycap tinymix tinypcminfo

#the device applications

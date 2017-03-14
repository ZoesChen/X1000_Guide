#Tools and libraries necessary by program

#kernel & uboot
PRODUCT_MODULES += $(KERNEL_TARGET_IMAGE) \
		  		   $(UBOOT_TARGET_FILE)


TINYALSA_UTILS := tinyplay tinycap tinymix tinypcminfo
RUNTESTDEMO_UTILS := RuntestScript AmicScript WificonnectScript bash CleanfileScript  DmicScript TfcardcopyScript
CAMERA_UTILS := CameraScript cimutils

#the device applications & test demo
PRODUCT_MODULES += $(RUNTESTDEMO_UTILS)\
				   $(CAMERA_UTILS)\

#Demo
#PRODUCT_MODULES += gpio_demo \
				   i2c_demo \
				   spi_demo


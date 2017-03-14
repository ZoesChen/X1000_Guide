# ----------------------------------------------------------------------------
#
# Copyright (C) 2008 Ingenic Semiconductor Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# Just a invoke entry, and will include each .mak file if needed
#
# Version,	Date, 		Engineer,	Mark
# v1.0, 	2014-04-13,	bliu,		Init version
#
# ----------------------------------------------------------------------------

BUILD_DIR := .

CURDIR	:= $(BUILD_DIR)

SRCTREE	:= $(CURDIR)/src
OUTTREE	:= $(CURDIR)/out

CROSS_COMPILE	:= mips-linux-gnu-

include $(CURDIR)/board_config.mk

CC      := $(CROSS_COMPILE)gcc
STRIP	:= $(CROSS_COMPILE)strip

#DFLAGS	+= -DTEST_OTA
#DFLAGS	+= -DDEBUG

CFLAGS	:= -mips32r2 -O2 -EL -Wall -Werror

INC	:= $(CURDIR)/include
SRCS	:= $(SRCTREE)/read_partition.c $(SRCTREE)/nv_area.c
SIGN_SRCS	:= $(SRCTREE)/rsa.c \
		$(SRCTREE)/rsa_e_3.c \
		$(SRCTREE)/rsa_e_f4.c \
		$(SRCTREE)/sha.c \
		$(SRCTREE)/verifier.c

SRCS_1	:= $(SRCTREE)/update.c $(SRCS) $(SIGN_SRCS)
SRCS_2	:= $(SRCTREE)/wr_flag.c $(SRCS)
SRCS_3	:= $(SRCTREE)/nv_ro_demo.c $(SRCS)
HCONFIG	:= $(CURDIR)/include/config.h

MKPYTHON := $(SRCTREE)/mkupdate.py
UZIPSH := $(SRCTREE)/unzip.sh

inkeys	:= $(CURDIR)/$(CONFIG_PUBLIC_KEYS)
outkeys := $(OUTTREE)/res/keys
all:	config update wr_flag nv_ro_demo $(outkeys)

nv_ro_demo:	$(SRCS_3) $(INC)
	$(CC) -I$(INC) $(CFLAGS) $(DFLAGS) -o $(OUTTREE)/$@ $(SRCS_3)
	$(STRIP) $(OUTTREE)/$@

update:	$(SRCS_1) $(INC)
	@echo "WARNING: current NOR blocksize is $(CONFIG_BLOCKSIZE)!!!!!!!"
	$(CC) -I$(INC) $(CFLAGS) $(DFLAGS) -o $(OUTTREE)/$@ $(SRCS_1)
	$(STRIP) $(OUTTREE)/$@
	@cp $(UZIPSH) $(OUTTREE)/
wr_flag: $(SRCS_2) $(INC)
	$(CC) -I$(INC) $(CFLAGS) $(DFLAGS) -o $(OUTTREE)/$@ $(SRCS_2)
	$(STRIP) $(OUTTREE)/$@

$(outkeys): libmincrypt/tools/dumpkey.jar
	@mkdir -p $(dir $(outkeys))
	java -jar $(CURDIR)/libmincrypt/tools/dumpkey.jar $(inkeys) > $(outkeys)

libmincrypt/tools/dumpkey.jar:
	cd $(CURDIR)/libmincrypt/tools/;./build.sh

ota:
	@python $(MKPYTHON)
config:  board_config.mk
	@echo "#define CONFIG_UBOOT_NAME \"$(CONFIG_UBOOT_NAME)\"" > $(HCONFIG)
	@echo "#define CONFIG_NVRO_NAME \"$(CONFIG_NVRO_NAME)\"" >> $(HCONFIG)
	@echo "#define CONFIG_NVRW_NAME \"$(CONFIG_NVRW_NAME)\"" >> $(HCONFIG)
	@echo "#define CONFIG_KERNEL_NAME \"$(CONFIG_KERNEL_NAME)\"" >> $(HCONFIG)
	@echo "#define CONFIG_UPDATEFS_NAME \"$(CONFIG_UPDATEFS_NAME)\"" >> $(HCONFIG)
	@echo "#define CONFIG_USERFS_NAME \"$(CONFIG_USERFS_NAME)\"" >> $(HCONFIG)
	@echo "#define CONFIG_BLOCKSIZE $(CONFIG_BLOCKSIZE)" >> $(HCONFIG)

clean:
	rm -fr $(OUTTREE)/update $(OUTTREE)/wr_flag $(OUTTREE)/nv_ro_demo $(outkeys)
distclean:
	@make clean
	rm -fr $(OUTTREE)/unzip.sh update_*

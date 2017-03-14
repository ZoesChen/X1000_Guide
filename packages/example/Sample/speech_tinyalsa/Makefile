#
#	sphere uca algorithm
#
#

arch ?= mips
use_ula ?= 0

ifeq ($(arch), mips)
TOOLCHAIN_DIR := /pdev/chains/mips-gcc472-glibc216/bin/
PREFIX := mips-linux-gnu-
UCLIBC_DIR := $(TOOLCHAIN_DIR)../mips-linux-gnu/libc/uclibc
else
TOOLCHAIN_DIR :=
PREFIX :=
UCLIBC_DIR :=
endif

CC := $(TOOLCHAIN_DIR)$(PREFIX)gcc
STRIP := $(TOOLCHAIN_DIR)$(PREFIX)strip
PWD	:= $(shell pwd)

CFLAGS := -O3 -fpic -Werror -g
CFLAGS += -I. -I./include
LDFLAGS  := -lpthread -lm


#CFLAGS += -I/home1/pzqi/git/cross_build/out/include
#LDFLAGS += -L/home1/pzqi/git/cross_build/out/lib -lasound

ifeq ($(use_ula), 1)
#CFLAGS += -DUSE_ULA
#LDFLAGS  += -Wl,-rpath=./libs:./libs/x1000-4mic -L./libs -lecho_wakeup -laiengine -L./libs/x1000-4mic -lula
else
LDFLAGS  += -Wl,-rpath=./libs -L./libs -lecho_wakeup -laiengine
endif

#CFLAGS += -DAI_DBG


SRC_DIR := ./src
#AIENGINE_SRC := ./main.c
AIENGINE_SRC := $(shell find $(SRC_DIR) -name "*.c")

AIENGINE_OBJ := $(patsubst %.c,%.o,${AIENGINE_SRC})

ifeq ($(use_ula), 1)
AIENFINE_TARGET := aiengine-4mic
else
AIENFINE_TARGET := aiengine-1mic
endif

all: $(AIENFINE_TARGET)

$(AIENFINE_TARGET): $(AIENGINE_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	$(STRIP) -s $@

clean:
	rm -fr $(AIENGINE_OBJ) $(AIENFINE_TARGET)

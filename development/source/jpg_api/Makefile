
CURDIR = .
#OUTDIR		:= $(CURDIR)/../bin

CROSS_COMPILE	:= mips-linux-gnu-

CC  	:= $(CROSS_COMPILE)gcc

#CFLAGS	:= -Wa,-mips32r2 -O2 -G 0 -Wall -static
CFLAGS	:= -Wa,-mips32r2 -O2 -G 0 -Wall -fPIC -shared
CFLAGS	+= -I$(CURDIR)/include #-DCHECK_RESULT
LDFLAGS +=

SOURCES := vpu_common.c \
	jpeg_encode2.c \
	jz_mem.c

OBJS	:= $(addsuffix .o , $(basename $(notdir $(SOURCES))))

APP	:= libjpeg.so
#APP2	:= libjpeg2.so

all:	 $(APP)

$(APP): $(OBJS)
	$(CC) $(CFLAGS)  $(OBJS) -o $(APP) $(LDFLAGS) -lpthread

$(OBJS):%.o:%.c
	$(CC) $(CFLAGS)  $< -c -o $@

clean:
	rm -rf $(APP) $(OBJS)

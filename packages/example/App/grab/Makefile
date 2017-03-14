VERSION = 0.1.4
TESTNAME        := grab

CC  := mips-linux-gnu-gcc
STRIP   := $(CROSS_COMPILE)strip

CFLAGS = -Wall -O2 --static -DVERSION=\"$(VERSION)\"
CFLAGS += -I./jpeg -L./jpeg
LIBLAGS = -ljpeg

OBJS     =  grab.o camera.o v4l2uvc.o savejpeg.o
TARGET = grab


all:$(TARGET)
$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(TYPEFLAGS) $(SRCFILES) $(LOCALFLAGS) $? -o $@ $(LIBLAGS)

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY:all clean



CFLAGS ?= -Wall
LDFLAGS ?=
INC = ./include
OBJECTS = guide.o key.o queue.o play.o location.o
LIB = libtinyalsa.a
SHLIB = libtinyalsa.so
CROSS_COMPILE = mips-linux-gnu-
CC := gcc
STRIP = strip

all: $(SHLIB) guide
	cp guide ../../../../out/product/halley2/system/bin/
guide: $(SHLIB) $(OBJECTS)
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) $(OBJECTS) -lpthread -L. -ltinyalsa -o guide
	$(CROSS_COMPILE)$(STRIP) $@
	
$(SHLIB): mixer.o pcm.o
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) -shared mixer.o pcm.o -o $(SHLIB)
	$(CROSS_COMPILE)$(STRIP) $@

%.o: %.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -fPIC -c $^ -I$(INC) -o $@

.PHONY:clean
clean:
	rm -rf *.o guide $(SHLIB)

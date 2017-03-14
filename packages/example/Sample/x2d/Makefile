CROSS   := mips-linux-gnu-
CFLAGS  := -Iinclude -Wall -O2 -mips32r2

OBJS := main.o		\
	dmmu.o          \
	x2d_api.o       \
	x2d.o           \

x2d_process: $(OBJS)
	$(CROSS)gcc $^  -o $@

.c.o:
	$(CROSS)gcc $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) x2d_process

install: x2d_process
	cp x2d_process /nfsroot/ylyuan/xixi-root/

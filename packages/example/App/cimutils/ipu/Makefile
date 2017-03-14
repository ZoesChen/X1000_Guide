CROSS   := mipsel-linux-
CFLAGS  := -Wall -O2 -mips32

LIB_OBJS := test_ipu.o jz47_soc_mem.o jz47_soc_rsize.o

libipu.a(test_ipu.o jz47_soc_mem.o jz47_soc_rsize.o): $(LIB_OBJS)
	$(CROSS)ar rcs $@ $(LIB_OBJS)

.c.o:
	$(CROSS)gcc $(CFLAGS) -c $< -o $@

exec:
	mipsel-linux-gcc -DTEST test_ipu.c jz47_soc_mem.c jz47_soc_rsize.c yuv_decode.c jz4750_ipu_regops.h jz47_iputype.h jz4750_ipu.h -o test_ipu
clean:
	@rm -rf *.o libipu.a test_ipu

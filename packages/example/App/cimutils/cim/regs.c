
#include "headers.h"

//read & write cim registers
int cim_do_test(int fd, int argc, char **argv)
{
	struct cim_reg_info info;
	int ret = 0;

	printf("==>%s %s() L%d: cim read/write registers testing...\n", __FILE__, __func__, __LINE__);
/*
	printf("argv = %p\n", argv);

	int i;
	printf("argc = %d\n", argc);
	for (i = 0; i < argc; i++)
		printf("argv[%d] = %s\n", i, argv[i]);
*/
	info.reg = htoi(argv[argc-3]);
	info.val = htoi(argv[argc-2]);
	info.ops = htoi(argv[argc-1]);

	printf("==>%s %s() L%d: reg=0x%08x, val=0x%08x\n", __FILE__, __func__, __LINE__, info.reg, info.val);
	sleep(1);

	ret = ioctl(fd, VIDIOC_CIM_RW_REG, &info);

	if (ret < 0) {
		printf("==>%s %s() L%d: ioctl fail!\n", __FILE__, __func__, __LINE__);
		perror(strerror(errno));
	}

	return ret;
}

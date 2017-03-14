#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <common.h>

int main(int argc, char *argv[])
{
	struct partition *parti;
	unsigned int blocksize;
	char *buf;
	int fd, ret;

	ret = get_partitions();
	if(ret < 0) {
		printf("====get_partition_info failed===\n");
		return -1;
	}

	blocksize = get_blocksize();
	parti = get_partition_info(CONFIG_NVRO_NAME);
	if(parti == NULL) {
		printf("not match %s partition\n", CONFIG_NVRO_NAME);
		return -1;
	}
	/* printf("parti->name = %s\n", parti->name); */
	/* printf("parti->charname = %s\n", parti->charname); */
	/* printf("parti->blockname = %s\n", parti->blockname); */
	/* printf("parti->name = %s\n", parti->name); */

	buf = malloc(blocksize);
	if(!buf) {
		printf("malloc fail\n");
		return -1;
	}

	fd = nv_open(parti, O_RDONLY);
	if(fd < 0) {
		free(buf);
		printf("open /dev/%s fail\n", parti->charname);
		return -1;
	}

	nv_read(fd, buf, blocksize);
	printf("--- %x\n", (unsigned int)(buf[0]));

	free(buf);
	nv_close(fd);
	return 0;
}

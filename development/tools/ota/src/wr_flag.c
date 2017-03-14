#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <common.h>

#define INFO_NAME "/tmp/update_info"
#define WGET_INFO_NAME "update_info"

static int get_info(struct nv_area_wr *nv_wr)
{
	FILE *fp;
	char line[1024];
	char *tmp;

	fp = fopen(INFO_NAME, "r");
	if(fp == NULL) {
		printf("open %s fail\n", INFO_NAME);
		return -1;
	}

	while (!feof(fp)) {
		memset(line, 0, 1024);
		if(fgets(line, 1024, fp) == NULL)
			continue;
		tmp = strtok(line, "=");
		tmp = strtok(NULL, "=");

		if (strstr(line, "version")) {
			nv_wr->nv_start.update_version = strtoull(tmp, NULL, 10);
			/* printf("nv_wr->nv_start.update_version = %lld\n", nv_wr->nv_start.update_version); */
		}
		if(strstr(line, "url")) {
			memcpy(nv_wr->nv_start.url_1, tmp, strlen(tmp) - 1);
			/* printf("%s", nv_wr->nv_start.url_1); */
		}
		if(strstr(line, "mode")) {
			if (atoi(tmp) == 1)
				nv_wr->nv_start.sfb.update_fs_finish = 1;
			else
				nv_wr->nv_start.sfb.update_fs_finish = 0;
		}
		if(strstr(line, "kernel_offset")) {
			nv_wr->nv_start.new_kernel_offset = strtoul(tmp, NULL, 10);
			/* printf("nv_wr->nv_start.new_kernel_offset = %u\n", nv_wr->nv_start.new_kernel_offset); */
		}
	}

	fclose(fp);
	return 0;
}
static void fix_url(struct nv_area_wr *nv_wr, char *url_1, int cmd)
{
	unsigned int size = 512 * sizeof(char);

	memset(nv_wr->nv_start.url_1, 0, size);

	switch(cmd) {
	case 1:
		memcpy(nv_wr->nv_start.url_1, url_1, strlen(url_1));
		break;
	case 2:
		memcpy(nv_wr->nv_start.url_1, nv_wr->nv_start.url_2, strlen(nv_wr->nv_start.url_2));
		break;
	default:
		printf("not support cmd %d\n", cmd);
	}
}
static int check_url(char *url_1, char *url_2)
{
	int ret;
	int timeout;
	char command[512];

	timeout = 10;
	snprintf(command, 512, "wget -c %s/%s -O %s\n", url_1, WGET_INFO_NAME, INFO_NAME);
	ret = system(command);
	while(timeout != 0 && ret != 0) {
		sleep(2);
		ret = system(command);
		timeout--;
	}
	if(!ret) {
		ret = 1;
		goto url_ok;
	}
	if(!timeout) {
		timeout = 10;
		snprintf(command, 512, "wget -c %s/%s -O %s\n", url_2, WGET_INFO_NAME, INFO_NAME);
		ret = system(command);
		while(timeout !=0 && ret !=0) {
			sleep(2);
			ret = system(command);
			timeout--;
		}
		if(!ret) {
			ret = 2;
			goto url_ok;
		}
	}
	if(!timeout)
		return -1;

url_ok:
	snprintf(command, 512, "rm -rf  %s\n", INFO_NAME);
	system(command);
	return ret;
}

int main(int argc, char *argv[])
{
	struct nv_area_wr *nv_wr;
	struct partition *parti;
	char command[512];
	int fd, ret = 0;
	int timeout = 1800;
	int blocksize;
#ifdef TEST_OTA
	printf("***************************************************************\n\n\n\n");
	printf("---------------------------------OTA TEST----------------------------\n");
	printf("\n\n\n***************************************************************\n");
#endif
	printf("argc=%d   argv[1]=%s\n", argc, argv[1]);

	ret = get_partitions();
	if(ret < 0) {
		printf("====get_partition_info failed===\n");
		return -1;
	}
	parti = get_partition_info(CONFIG_NVRW_NAME);
	if(parti == NULL) {
		printf("not match %s partition\n", CONFIG_NVRW_NAME);
		return -1;
	}

	blocksize = get_blocksize();
	if(blocksize < 0) {
		printf("read blocksize = %d\n", blocksize);
		return -1;
	}
	if(blocksize != CONFIG_BLOCKSIZE) {
		printf("read blocksize %d != CONFIG_BLOCKSIZE %d\n", blocksize, CONFIG_BLOCKSIZE); \
		return -1;
	}

	nv_wr = malloc(blocksize);
	if(!nv_wr) {
		printf("malloc fail\n");
		return -1;
	}

	fd = nv_open(parti, O_RDWR);
	if(fd < 0) {
		free(nv_wr);
		printf("open /dev/%s fail\n", parti->charname);
		return -1;
	}

	/* lseek(fd, 0, SEEK_SET); */

	ret = nv_read(fd, nv_wr, blocksize);
	if(ret < 0) {
		printf("read error\n");
		goto read_fail;
	}

	if(argc == 2) {
		ret = check_url(argv[1], nv_wr->nv_start.url_2);
		if(ret == -1) {
			printf("check url file\n");
			goto read_fail;
		}
		fix_url(nv_wr, argv[1], ret);
	}

	while(timeout) {
		ret = check_url(nv_wr->nv_start.url_1, nv_wr->nv_start.url_2);
		if(ret == -1) {
			printf("check url file\n");
			goto read_fail;
		}
		if(ret == 2)
			fix_url(nv_wr, NULL, ret);

		snprintf(command, 512, "wget -c %s/%s -O %s\n", nv_wr->nv_start.url_1, WGET_INFO_NAME, INFO_NAME);
		ret = system(command);
		if(ret == 0)
			break;
		sleep(2);
		timeout--;
	}
	if(!timeout) {
		ret = -1;
		goto read_fail;
	}

	ret = get_info(nv_wr);
	if(ret < 0) {
		printf("get info error\n");
		goto read_fail;
	}

	if(nv_wr->nv_start.current_version < nv_wr->nv_start.update_version) {
		nv_wr->nv_start.sfb.ota_start = 1;
		nv_wr->nv_start.sfb.load_new_fs = 0;
		nv_wr->nv_start.sfb.user_fs_finish = 1;
		nv_wr->nv_start.block_current_count = 0;
		nv_wr->nv_start.block_sum_count = 0;
		ret = nv_write(fd, nv_wr, blocksize);
		if(ret < 0) {
			printf("write error\n");
			goto read_fail;
		}
	}

	system("reboot");
	sleep(10);

read_fail:
	free(nv_wr);
	nv_close(fd);

	return ret;
}

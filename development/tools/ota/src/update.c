#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <common.h>
#include "verifier.h"


#define TMPFILE_NAME "/tmp/tmpfile"
#define PUBLIC_KEYS_FILE "/usr/res/keys"
#define UPDATE_DATE_DIR "/tmp"

#define KERNEL_SIZE 1
#define UPDATEFS_SIZE 2
#define USRFS_SIZE 3

static int fd = 0;
static int blocksize;

/* get sum from update.script */
static int get_sum()
{
	int fd_sh, sum;
	char buffer[256];
	char buf1[20], buf2[20], buf3[20];

	fd_sh = open(TMPFILE_NAME, O_RDONLY);
	if(fd_sh < 0) {
		printf("%s: not open %s\n", __func__, TMPFILE_NAME);
		return -1;
	}
	read(fd_sh, buffer, sizeof(buffer));
	sscanf(buffer,"%[sum=]%[0-9]%s", buf1, buf2, buf3);
	sum = atoi(buf2);
	close(fd_sh);

	return sum;
}

static int get_reboot()
{
	int fd_sh, reboot;
	char buffer[512];
	char buf1[20], buf2[20], buf3[20], buf4[20], buf5[20];

	fd_sh = open(TMPFILE_NAME, O_RDONLY);
	if(fd_sh < 0) {
		printf("%s: not open %s\n", __func__, TMPFILE_NAME);
		return -1;
	}
	read(fd_sh, buffer, sizeof(buffer));
	sscanf(buffer,"%[sum=]%[0-9]%s", buf1, buf2, buf3);
	sscanf(buf3,"%[reboot=]%[0-9]", buf4, buf5);
	reboot = atoi(buf5);

	return reboot;
}
/* get kernel,upade_fs usrfs sise */
static int get_size(int name)
{
	int fd_sh, size = 0, align;
	char buffer[512];
	char buf1[20], buf2[20];

	fd_sh = open(TMPFILE_NAME, O_RDONLY);
	if(fd_sh < 0) {
		printf("%s: not open %s\n", __func__, TMPFILE_NAME);
		return -1;
	}
	read(fd_sh, buffer, sizeof(buffer));
	switch(name){
	case KERNEL_SIZE:
		sscanf(buffer,"%*s %*s %[kernel_size=]%[0-9]", buf1, buf2);
		size = atoi(buf2);
		break;
	case UPDATEFS_SIZE:
		sscanf(buffer,"%*s %*s %*s %[update_size=]%[0-9]", buf1, buf2);
		size = atoi(buf2);
		break;
	case USRFS_SIZE:
		sscanf(buffer,"%*s %*s %*s %*s %[usrfs_size=]%[0-9]", buf1, buf2);
		size = atoi(buf2);
		break;
	}
	align = 512 * 1024;
	size = (size + align-1) & ~(align-1);
	size = size / 1024;
	close(fd_sh);

	return size;

}
static int ota_update_finish(struct nv_area_wr *nv_wr)
{
	int ret = -1;
#ifndef TEST_OTA
	nv_wr->nv_start.current_version = nv_wr->nv_start.update_version;
	nv_wr->nv_start.update_version = 0;
	nv_wr->nv_start.update_status = 0;
	nv_wr->nv_start.block_sum_count = 0;
	ret = nv_write(fd, nv_wr, blocksize);
	if(ret < 0) {
		printf("clear NV error\n");
		return -1;
	}
	while(1) {
		system("reboot");
		sleep(10);
	}
#else
	nv_wr->nv_end.ota_num++;
	ret = nv_write(fd, nv_wr, blocksize);
	if(ret < 0) {
		printf("clear NV error\n");
		return -1;
	}
	printf("***************************************************************\n\n\n\n");
	printf("current ota num %d\n", nv_wr->nv_end.ota_num);
	printf("***************************************************************\n\n\n\n");
	while(1) {
		system("wr_flag");
		sleep(10);
	}
#endif
	return ret;
}

static int check_update(const char *path)
{
	int numKeys;
	int err;
	RSAPublicKey* loadedKeys = load_keys(PUBLIC_KEYS_FILE, &numKeys);
	if (loadedKeys == NULL) {
		printf("Failed to load keys\n");
		return -1;
	}

	/* printf("%d key(s) loaded from %s\n", numKeys, PUBLIC_KEYS_FILE); */

	err = verify_file(path, loadedKeys, numKeys);
	free(loadedKeys);
	/* printf("verify_file returned %d\n", err); */
	if (err != VERIFY_SUCCESS) {
		printf("signature verification failed\n");
		return -1;
	}
	return err;
}
static int get_and_check_update(struct nv_area_wr *nv_wr, char *url)
{
	char command[1024];
	int ret;
	int timeout = 10000;

retry:
	sprintf(command, "wget -c %s/update_%lld/updatezip%d.zip -P %s",
		url, nv_wr->nv_start.update_version, nv_wr->nv_start.block_current_count, UPDATE_DATE_DIR);
	/* printf("wget command = %s\n", command); */

	while (timeout) {
		ret = system(command);
		if (ret == 0)
			break;
		printf("wget error ret = %d\n", ret);
		sprintf(command, "rm -rf %s/updatezip%d.zip",
			UPDATE_DATE_DIR, nv_wr->nv_start.block_current_count);
		ret = system(command);
		timeout --;
		sleep(2);
	}

	memset(command, 0, 1024);
	sprintf(command, "%s/updatezip%d.zip",
		UPDATE_DATE_DIR, nv_wr->nv_start.block_current_count);
	/* printf("path command = %s\n", command); */
	ret = check_update(command);
	if(ret != VERIFY_SUCCESS)
		return -1;

	memset(command, 0, 1024);
	sprintf(command, "unzip.sh %d", nv_wr->nv_start.block_current_count);
	ret = system(command);
	if (ret != 0) {
		printf("unzip error ret = %d\n", ret);
		timeout --;
		sleep(2);
		goto retry;
	}

	return ret;
}
static int wget_userfs(struct nv_area_wr *nv_wr, char *url)
{
	int ret = 0;

next_update:
	if(get_and_check_update(nv_wr, url) < 0)
		return -1;

	if(nv_wr->nv_start.block_current_count == 0) {
		nv_wr->nv_start.block_sum_count = get_sum();
		if(nv_wr->nv_start.block_sum_count == -1)
			return -1;
		ret = nv_write(fd, nv_wr, blocksize);
		if(ret < 0) {
			printf("write user fs block_sum_count error\n");
			return -1;
		}
	}
	if(nv_wr->nv_start.block_current_count != (nv_wr->nv_start.block_sum_count - 1)) {
		nv_wr->nv_start.block_current_count++;
		ret = nv_write(fd, nv_wr, blocksize);
		if(ret >= 0)
			goto next_update;
	} else {
		/* last block */
		nv_wr->nv_start.sfb.user_fs_finish = 0;
		/* change nv and write to nor */
		ret = nv_write(fd, nv_wr, blocksize);
		if(ret < 0) {
			printf("write user fs error\n");
			return -1;
		}
		while(1) {
			system("reboot");
			sleep(10);
		}
	}

	return ret;
}
static int wget_kernel_and_updatefs(struct nv_area_wr *nv_wr, char *url)
{
	int ret = -1;

next_update:
	if(get_and_check_update(nv_wr, url) < 0)
		return -1;

	/* wget update and write to extern memory */
	/* printf("nv_wr->nv_start.block_current_count = %x\n",nv_wr->nv_start.block_current_count); */
	if(nv_wr->nv_start.block_current_count == 0) {
		nv_wr->nv_start.block_sum_count = get_sum();
		if(nv_wr->nv_start.block_sum_count == -1)
			return -1;
		nv_wr->nv_start.kernel_size = get_size(KERNEL_SIZE);
		if(nv_wr->nv_start.kernel_size == -1)
			return -1;
		/* printf("nv_wr->nv_start.kernel_size = %d\n",nv_wr->nv_start.kernel_size); */
		nv_wr->nv_start.update_size = get_size(UPDATEFS_SIZE);
		if(nv_wr->nv_start.update_size == -1)
			return -1;
		/* printf("nv_wr->nv_start.update_size = %d\n",nv_wr->nv_start.update_size); */
	}
	ret = get_reboot();
	if(!ret) {
		nv_wr->nv_start.block_current_count++;
		ret = nv_write(fd, nv_wr, blocksize);
		if(ret >= 0)
			goto next_update;
	} else if(ret > 0) {
		nv_wr->nv_start.sfb.load_new_fs = 1;
		ret = nv_write(fd, nv_wr,blocksize);
		if(ret < 0)
			goto fail;
		while(1) {
			system("reboot");
			sleep(10);
		}
	}
fail:
	return ret;
}
static int write_kernel_and_updatefs(struct nv_area_wr *nv_wr)
{
	/*
	 * unsigned int ota_start:1;			//start
	 * unsigned int load_new_fs:1;			//new_flag
	 * unsigned int update_fs_finish:1;	//update_kernel_finish
	 * unsigned int user_fs_finish:1;		//user_fs_finish
	 */
	char command[1024];
	int ret = -1;
	unsigned int xImage_offset;
	struct partition *parti_userfs;
	struct partition *parti_xImage;
	struct partition *parti_updatefs;

	parti_xImage = get_partition_info(CONFIG_KERNEL_NAME);
	if(parti_xImage == NULL) {
		printf("not match %s partition\n", CONFIG_KERNEL_NAME);
		return -1;
	}
	parti_updatefs = get_partition_info(CONFIG_UPDATEFS_NAME);
	if(parti_updatefs == NULL) {
		printf("not match %s partition\n", CONFIG_UPDATEFS_NAME);
		return -1;
	}
	parti_userfs = get_partition_info(CONFIG_USERFS_NAME);
	if(parti_userfs == NULL) {
		printf("not match %s partition\n", CONFIG_USERFS_NAME);
		return -1;
	}

	/*
	 * write new kernel and updatefs
	 * to old kernel and updatefs region
	 */
	xImage_offset = parti_updatefs->size + nv_wr->nv_start.new_kernel_offset;
	snprintf(command, 1024, "flash_erase -q /dev/%s 0 %d\n", parti_xImage->charname, parti_xImage->size / blocksize);
	system(command);
	snprintf(command, 1024,"dd if=/dev/%s of=/dev/%s skip=%d bs=512 count=%d\n",
		 parti_userfs->blockname, parti_xImage->blockname, xImage_offset/512, nv_wr->nv_start.kernel_size*2);
	system(command);

	snprintf(command, 1024, "flash_erase -q /dev/%s 0 %d\n", parti_updatefs->charname, parti_updatefs->size / blocksize);
	system(command);
	snprintf(command, 1024,"dd if=/dev/%s of=/dev/%s skip=0 bs=512 count=%d\n",
		 parti_userfs->blockname, parti_updatefs->blockname, nv_wr->nv_start.update_size*2);
	system(command);



	nv_wr->nv_start.sfb.load_new_fs = 0;
	nv_wr->nv_start.sfb.update_fs_finish = 0;
	nv_wr->nv_start.block_current_count++;
	/* change nv and write to nor */
	ret = nv_write(fd, nv_wr, blocksize);
	if(ret < 0) {
		printf("%s: write error\n", __func__);
		return -1;
	}

	while(1) {
		system("reboot");
		sleep(10);
	}
	/* never get here */
	return 0;
}
#define UPDATE_FINISHED 0x0
#define UPDATE_FINISHING 0x1
#define UPDATE_USERFS 0x9
#define UPDATE_KERNEL_AND_UPDATEFS 0xd
#define UPDATE_WRITE_NEW_KERNEL 0xf

static int start_wget_and_update(struct nv_area_wr *nv_wr, char *url)
{
	/*
	 * unsigned int user_fs_finish
	 * 1: update userfs
	 * 0: update finish userfs
	 *
	 * unsigned int update_fs_finish
	 * 1: update kernel and updatefs to userfs partition
	 * 0: update finish write kernel and updatefs to userfs partition
	 *
	 * unsigned int load_new_fs
	 * 1: write new_kernel and updatefs to kernel and updatefs partiotion
	 * 0: finish kernel and updatefs update
	 *
	 * unsigned int ota_start
	 * 1: start ota update
	 * 0: finish ota update
	 */

	unsigned int update_status = nv_wr->nv_start.update_status;
	int ret = -1;

	switch(update_status) {
	case UPDATE_FINISHING:
		ret = ota_update_finish(nv_wr);
		break;
	case UPDATE_USERFS:
		ret = wget_userfs(nv_wr, url);
		break;
	case UPDATE_KERNEL_AND_UPDATEFS:
		ret = wget_kernel_and_updatefs(nv_wr, url);
		break;
	case UPDATE_WRITE_NEW_KERNEL:
		ret = write_kernel_and_updatefs(nv_wr);
		break;
	}

	return ret;
}
int main(int argv,char *argc[])
{
	struct nv_area_wr *nv_wr;
	struct partition *parti;
	int ret;
	char *url;
	char command[512];

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
		printf("read blocksize %d != CONFIG_BLOCKSIZE %d\n", blocksize, CONFIG_BLOCKSIZE);
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
		printf("open %s fail\n", parti->charname);
		return -1;
	}

	while(1) {
		ret = nv_read(fd, nv_wr, blocksize);
		if(ret < 0) {
			printf("read error\n");
			goto fail;
		} else {
			printf("start:%d new:%d update:%d user:%d\n",
			       nv_wr->nv_start.sfb.ota_start, nv_wr->nv_start.sfb.load_new_fs, \
			       nv_wr->nv_start.sfb.update_fs_finish, nv_wr->nv_start.sfb.user_fs_finish);

			if(nv_wr->nv_start.sfb.ota_start == UPDATE_FINISHED) {
				parti = get_partition_info(CONFIG_USERFS_NAME);
				if(parti == NULL) {
					printf("not match %s partition\n", CONFIG_USERFS_NAME);
					return -1;
				}
				snprintf(command, 512, "/dev/%s", parti->blockname);
				while(access(command, W_OK | R_OK))
					sleep(2);
				snprintf(command, 512, "mount -t cramfs /dev/%s /%s\n", parti->blockname, CONFIG_USERFS_NAME);
				ret = system(command);
				if(ret != 0) {
					printf("mount %s fail!!!!!!!!\n", CONFIG_USERFS_NAME);
					system("poweroff");
					//exit(1);
					sleep(10);
				}
				goto fail;
			}
			if(nv_wr->nv_start.url_1)
				url = nv_wr->nv_start.url_1;
			else if(nv_wr->nv_start.url_2)
				url = nv_wr->nv_start.url_2;
			else {
				printf("url_1 and url_2 is all null\n");
				goto fail;
			}
			printf("url = %s\n", url);
			ret = start_wget_and_update(nv_wr, url);
			if(ret < 0)
				printf("start wget or update error\n");
		}
	}

fail:
	free(nv_wr);
	nv_close(fd);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <error.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <linux/types.h>
#include <mtd/mtd-abi.h>
#include <common.h>

typedef struct erase_info_user erase_info_t;

struct nv_devices {
	int nv_fd;
	unsigned int nv_count;
	unsigned int nv_offset;
	unsigned int nv_blockszie;
	unsigned int nv_erasesize;
	unsigned int nv_ebcnt;
	unsigned int nv_ro;
};

#define DEFAULT_START_FLAG (0x5a5a5a5a)
#define DEFAULT_END_FLAG (0xa5a5a5a5)

#define WRITE_FLAG 0
#define READ_FLAG 1

static struct nv_devices *nv_dev;

int nv_open(struct partition *parti, int flags)
{
	int err = 0;
	char cname[64];

	nv_dev = malloc(sizeof(struct nv_devices));
	if (!nv_dev) {
		printf("error: nv_dev cannot allocate memory\n");
		err = -1;
		return err;
	}
	nv_dev->nv_count = 0;
	nv_dev->nv_offset = 0;
	nv_dev->nv_blockszie = parti->erasesize;
	nv_dev->nv_erasesize = parti->erasesize;
	nv_dev->nv_ebcnt = parti->size / parti->erasesize;

	snprintf(cname, 64, "/dev/%s", parti->charname);
	nv_dev->nv_fd = open(cname, flags);
	if(nv_dev->nv_fd < 0) {
		printf("open %s failed\n", cname);
		err = -1;
		goto open_fail;
	}
	if(!strncmp(parti->name, CONFIG_NVRO_NAME, sizeof(CONFIG_NVRO_NAME)))
		nv_dev->nv_ro = 1;
	return nv_dev->nv_fd;

open_fail:
	free(nv_dev);
	return err;
}
int nv_close(int fd)
{
	if(nv_dev)
		free(nv_dev);

	close(fd);
	return 0;
}

static int nv_read_area(int fd, char *buf, size_t count, off_t offset)
{
	int err;
	off_t off;

	/* printf("read offset = 0x%x\n", (unsigned int)offset); */
	off = lseek(fd, offset, SEEK_SET);
	err = read(fd, (u_char *)buf, count);
	if (err != count) {
		printf("error: read failed at 0x%llx\n", (unsigned long long)off);
		err = -1;
	}
	return err;
}
static off_t nv_map_area(int fd, int flag)
{
	unsigned int buf[3][2];
	unsigned int current_nv_num = 0, i;
	int tmp, err;
	off_t offset;

	tmp = -1;
	for(i = 0; i < nv_dev->nv_ebcnt; i++) {
		offset = i * nv_dev->nv_blockszie;
		err = nv_read_area(fd, (char *)buf[i], 4, offset);
		if(err < 0) {
			printf("read error offset = %x\n", (unsigned int)offset);
			continue;
		}
		if(buf[i][0] == DEFAULT_START_FLAG) {
			err = nv_read_area(fd, (char *)buf[i], 8, offset + nv_dev->nv_blockszie - 8);
			if(err < 0)
				continue;
			if(buf[i][1] == DEFAULT_END_FLAG) {
				tmp = 0;
				if(nv_dev->nv_count <= buf[i][0]) {
					nv_dev->nv_count = buf[i][0];
					current_nv_num = i;
				}
			}
		}
	}

	if(tmp) {
		for(i = 0; i < nv_dev->nv_ebcnt; i++) {
			if(buf[i][0] == 0xffffffff || buf[i][0] == 0)
				continue;
			break;
		}
		if(i < 3) {
			printf("WARN: not found right nv wr region!!!!!buf[%d][0] = %x\n", i, buf[i][0]);
			return tmp;
		}
	}

	for(i = 0; i < nv_dev->nv_ebcnt; i++) {
		if(flag == READ_FLAG) {
			tmp = (current_nv_num + i) % 3;
		} else if(flag == WRITE_FLAG) {
			tmp = (current_nv_num + i + 1) % 3;
		}
		nv_dev->nv_offset = tmp * nv_dev->nv_blockszie;
		err = nv_read_area(fd, (char *)buf[i], 4, nv_dev->nv_offset);
		if(err < 0)
			continue;
		break;
	}
	if(i == 3) {
		printf("WARN: ALL nv wr region read error\n");
		return -1;
	}
	if(flag == WRITE_FLAG)
		nv_dev->nv_count++;

	return nv_dev->nv_offset;
}
static int nv_map_and_read_area(int fd, char *buf, size_t count)
{
	off_t offset;
	int err = 0;

	offset = nv_map_area(fd, READ_FLAG);
	if(offset < 0) {
		printf("ERROR:----read offset = %x\n", (unsigned int)offset);
		return offset;
	}
	err = nv_read_area(fd, buf, count, offset);
	return err;
}
ssize_t nv_read(int fd, void *buf, size_t count)
{
	int err = 0;
	off_t off;

	if(count > nv_dev->nv_blockszie) {
		printf("error:  read count 0x%x more than nv block size 0x%x\n",
			count, nv_dev->nv_blockszie);
		err = -1;
		return err;
	}
	off = lseek(fd, 0, SEEK_CUR);
	off %= nv_dev->nv_blockszie;
	if(off + count > nv_dev->nv_blockszie) {
		printf("error:  read current location %llx plus count 0x%x more than nv max size 0x%x\n",
		       (unsigned long long )off, count, nv_dev->nv_blockszie);
		err = -1;
		return err;
	}
	if(!nv_dev->nv_ro)
		err = nv_map_and_read_area(fd, (char *)buf, count);
	else
		err = read(fd, (char *)buf, count);

	return err;
}

static int erase_eraseblock(int fd, unsigned int addr, unsigned int count)
{
	int err = 0;
	erase_info_t ei;

	ei.start = addr;
	ei.length = count;
	ioctl(fd, MEMUNLOCK, &ei);
	err = ioctl(fd, MEMERASE, &ei);
	if (err < 0)
		printf("MEMERASE------------faild %d\n", err);

	return err;
}
static int nv_map_and_write_area(int fd, const char *buf, size_t count)
{
	unsigned int addr;
	off_t offset;
	int err;

	offset = nv_map_area(fd, WRITE_FLAG);
	if(offset < 0) {
		printf("ERROR:------offset = %x\n", (unsigned int)offset);
		return offset;
	}

	addr = offset;
	err = erase_eraseblock(fd, addr, nv_dev->nv_blockszie);
	if (err < 0) {
		printf("error: erase failed at 0x%llx\n",
		       (long long)addr);
		return err;
	}

	lseek(fd, addr, SEEK_SET);

	*(unsigned int *)(&buf[0]) = DEFAULT_START_FLAG;
	*(unsigned int *)(&buf[nv_dev->nv_blockszie - 8]) = nv_dev->nv_count;
	*(unsigned int *)(&buf[nv_dev->nv_blockszie - 4]) = DEFAULT_END_FLAG;
	err = write(fd, buf, count);
	if (err != count) {
		printf("error: write failed at 0x%llx\n",
		       (long long)addr);
		printf("error: write failed err = %d count = %d\n",
		       err, count);
		return err;
	}
	return err;
}
ssize_t nv_write(int fd, const void *buf, size_t count)
{
	int err = 0;
	off_t off;
	if(count > nv_dev->nv_blockszie) {
		printf("error:  write count 0x%x must equl 0x%x\n",
			count, nv_dev->nv_blockszie);
		err = -1;
		return err;
	}
	off = lseek(fd, 0, SEEK_CUR);
	off %= nv_dev->nv_blockszie;
	if(off + count > nv_dev->nv_blockszie) {
		printf("error:  write current location %llx plus count 0x%x more than nv max size 0x%x\n",
		       (unsigned long long )off, count, nv_dev->nv_blockszie);
		lseek(fd, 0, SEEK_SET);
	}

	err = nv_map_and_write_area(fd, (char *)buf, count);
	return err;
}

off_t nv_lseek(int fd, off_t offset, int whence)
{
	return lseek(fd, offset, whence);
}

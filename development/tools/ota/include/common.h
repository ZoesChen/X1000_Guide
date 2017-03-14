#ifndef __COMMON_H__
#define __COMMON_H__

#include <config.h>

#define DRIVER_NAME "/dev/nv_wr"

#define CMD_WRITE_URL_1 _IOW('N', 0, char)
#define CMD_WRITE_URL_2 _IOW('N', 1, char)
#define CMD_READ_URL_1  _IOR('N', 3, char)
#define CMD_READ_URL_2  _IOR('N', 4, char)
#define CMD_EARASE_ALL  _IOR('N', 5, int)

#define UPDATE_START_FLAG 	(1 << 0)/* has update.zip status flag */
#define UPDATE_NEW_FS_FLAG      (1 << 1)
#define UPDATE_UPDATE_FS_FLAG   (1 << 2)
#define UPDATE_USER_FS_FLAG     (1 << 3)

struct wr_info {
	char *wr_buf[512];
	unsigned int size;
	unsigned int offset;
};

struct status_flag_bits {
	unsigned int ota_start:1;			//start
	unsigned int load_new_fs:1;			//new_flag
	unsigned int update_fs_finish:1;	//update_kernel_finish
	unsigned int user_fs_finish:1;		//update_fs_finish
};
struct nv_start
{
	unsigned int write_start;
	union {
		unsigned int update_status;
		struct status_flag_bits sfb;
	};
	char url_1[512];
	char url_2[512];
	unsigned long long current_version;
	unsigned long long update_version;
	unsigned int block_sum_count;
	unsigned int block_current_count;
	unsigned int kernel_size;
	unsigned int update_size;
	unsigned int new_kernel_offset;

};
struct nv_end
{
#ifdef TEST_OTA
	unsigned int ota_num;
#endif
	unsigned int write_count;
	unsigned int write_end;
};

#define RESEVER_SIZE (CONFIG_BLOCKSIZE - sizeof(struct nv_start) - sizeof(struct nv_end))
struct nv_area_wr {
	struct nv_start nv_start;
	char resever[RESEVER_SIZE];
	struct nv_end nv_end;
};

struct partition {
	char name[32];
	unsigned int offset;
	unsigned int size;
	unsigned int erasesize;
	char charname[32];
	char blockname[32];
};
int get_blocksize();
struct partition *get_partition_info(char *name);
int get_partitions();

int nv_open(struct partition *parti, int flags);
int nv_close(int fd);
ssize_t nv_read(int fd, void *buf, size_t count);
ssize_t nv_write(int fd, const void *buf, size_t count);
off_t nv_lseek(int fd, off_t offset, int whence);

#endif	/* __COMMON_H__ */

#ifndef __PARAM_H__
#define __PARAM_H__
#include <string>
using  std::string;
/*this is nand param struct but now not use we read uboot get param directly*/
#if 0
struct jz_spi_support {
        unsigned int id_manufactory;
        unsigned char id_device;
        char name[32];
        int page_size;
        int oobsize;
        int sector_size;
        int block_size;
        int size;
        int page_num;

        /* MAX Busytime for page read, unit: us */
        int tRD_maxbusy;
        /* MAX Busytime for Page Program, unit: us */
        int tPROG_maxbusy;
        /* MAX Busytime for block erase, unit: us */
        int tBERS_maxbusy;

        unsigned short column_cmdaddr_bits;/* read from cache ,the bits of cmd + addr */

};

struct jz_spinand_partition {
        char name[32];         /* identifier string */
        unsigned int size;          /* partition size */
        unsigned int offset;        /* offset within the master MTD space */
        unsigned int mask_flags;       /* master MTD flags to mask out for this partition */
        unsigned int manager_mode;         /* manager_mode mtd or ubi */
};

struct chip_param {
        int magic;
        int version;
        int flash_type;
        int num_spi_flash;
        struct jz_spi_support *jz_spi_support;
        int num_partitions;
        struct jz_spinand_partition *chip_paratition;
};
#endif
class nand_chip_param
{
public:
    explicit nand_chip_param(int,int);
    ~nand_chip_param();
public:
    int get_param_from_file();
    const char *get_param_buffer();
    int get_page_size();
private:
    int nand_magic;
    int page_size;
    int pages_offset;
    int param_offset;
    string sysfilename;
//    struct chip_param param;
    char *buffer;
    int fd;
};
#endif

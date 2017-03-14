#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <i2c.h>
#include <i2c-dev.h>
#include <I2cAdapter.h>
#include <iostream>
#include <sstream>

using namespace std;


int eeprom_read(I2cAdapter * mEEpromDevice ,int BUFF_SIZE)
{
	unsigned char buff[BUFF_SIZE];

	signed int ret = mEEpromDevice->I2cMasterRecv(buff,BUFF_SIZE);
	cout << "ret =" << ret <<endl;
	int i;

	for (i = 0; i < BUFF_SIZE; i++)
	{
		if(i % 16 == 0)
			printf ("\n");
		printf("%02x ",buff[i]);
	}
	printf ("\n");
}

int eeprom_write_page(int addr_p,int page_addr)

{
	int num, err, i, j;
	int fd;
	char *buff;
	const char test_data[] ={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

	num = 17;

	buff =(char*) malloc(num * sizeof(char));
	if(buff < 0){
		printf("alloc failed\n");
		return -1;
	}

	/*buff[0] is word addr*/
	buff[0] = addr_p;

	printf("write data:\n");
	for(i = 1; i < num; i++)
	{
		buff[i] = test_data[i];
		if(((i-1) % 16 ) == 0)
			printf("\n");
		printf("%02x ",buff[i]);
	}
	printf("from word addr:%d\n",buff[0]);

	fd = open("/dev/i2c-1",O_RDWR);
	if(fd < 0){
		printf("device open failed\n");
		return -1;
	}

	err = ioctl(fd, I2C_SLAVE_FORCE, page_addr);
	if(err < 0){
		printf("ioctl failed:%d\n",err);
		return -1;
	}

	write(fd, buff, num);
	close(fd);
	return 0;
}

int main(int argc,char **argv)
{
	int page_begin_addr= 0x50;
	I2cAdapter * mEEpromDevice;
	mEEpromDevice = new I2cAdapter(1); /* /dev/i2c-x, eeprom x=1 */
	mEEpromDevice->I2cSetSlave(page_begin_addr);

	/*eeprom_write*/
	printf("================eeprom write ==================");
	int i ,p;
	for(p=0 ;p<8 ;p++) /*8 page_addr*/
	{
		printf("------------page addr: 0x%02x\n",page_begin_addr);
		for(i=0; i< 256; i++)
		{
			if(i % 16 == 0)
			{
				eeprom_write_page(i,page_begin_addr);
				sleep(1);
			}
		}
		page_begin_addr++;
	}

	/*read eeprom*/
	printf("================ eeprom write ==================");
	int buff_size = 2048;
	eeprom_read(mEEpromDevice ,buff_size);
}

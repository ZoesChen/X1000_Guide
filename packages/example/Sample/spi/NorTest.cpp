#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "spidev.h"
#include "spi.h"


class NorDev{
	public:
		int InitSpi(unsigned short mode, unsigned short bits, unsigned int speed);
};

SpiDev spinorflash(0,0);


int NorDev::InitSpi(unsigned short mode, unsigned short bits, unsigned int speed)
{
	spinorflash.SpiSetMode(mode);
	spinorflash.SpiSetBitsPerWord(bits);
	spinorflash.SpiSetMaxSpeed(speed);

}


int main()
{
	unsigned short mode = 0;
	unsigned short bits = 8;
	unsigned int speed = 1000000;
	unsigned int delay = 500;

	unsigned char send_buf[1] = {0x9f,};
	unsigned char recv_buf[3] = {0};

	NorDev nor;

	nor.InitSpi(mode, bits, speed);

	spinorflash.SpiMessageTransfer(send_buf, recv_buf, sizeof(send_buf) + sizeof(recv_buf));
	unsigned int id = (recv_buf[1] << 16) | (recv_buf[2] << 8) | recv_buf[3];
	printf("recv_buf[0]=%x  recv_buf[1]=%x recv_buf[2]=%x recv_buf[3]=%x\n",recv_buf[0], recv_buf[1], recv_buf[2], recv_buf[3]);
	printf("id=%06x\n", id);

}

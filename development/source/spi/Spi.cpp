#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "spidev.h"
#include "spi.h"


#define SPI_READ_MODE 	1
#define SPI_WRITE_MODE  0

#define MAX_BUF 100

/**
 * @brief open a spi device
 *
 * @param bus_id num of spi bus id
 * @param chip_id num of spi chip id
 */
SpiDev::SpiDev(unsigned int bus_id, unsigned int chip_id)
{
	this->fd = SpiOpenDev(bus_id, chip_id);
}

SpiDev::~SpiDev()
{
	close(this->fd);
}

/**
 * @brief open a spi device
 *
 * @param bus_id num of spi bus id
 * @param chip_id num of spi chip id
 */
int SpiDev::SpiOpenDev(unsigned int bus_id, unsigned int chip_id)
{

	char buf[MAX_BUF];
	unsigned int fd;
	snprintf(buf, sizeof(buf), "/dev/spidev%d.%d", bus_id, chip_id);
	fd = open(buf, O_RDWR);
	if (fd < 0)
		printf("open device error,%s\n", buf);
	return fd;
}

/**
 * @brief set spi transfer mode
 *
 * @param mode from 0~3
 *
 * @return success:0, error:-1
 */
int SpiDev::SpiSetMode(unsigned short mode)
{
	int ret = 0;

	ret = ioctl(this->fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		printf("can't set spi mode\n");

	ret = ioctl(this->fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		printf("can't get spi mode\n");

	printf("spi mode: %d\n", mode);
}

/**
 * @brief set transfer bits per word
 *
 * @param bits bits to transfer for one times
 *
 * @return success:0, error:-1
 */
int SpiDev::SpiSetBitsPerWord(unsigned short bits)
{
	int ret;
	unsigned short bits_per_word = bits;
	ret = ioctl(this->fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word);
	if (ret == -1)
		printf("can't set bits per word\n");

	ret = ioctl(this->fd, SPI_IOC_RD_BITS_PER_WORD, &bits_per_word);
	if (ret == -1)
		printf("can't get bits per word\n");

	printf("bits per word: %d\n", bits);
}

/**
 * @brief set max transfer speed
 *
 * @param speed  max transfer speed
 *
 * @return success:0, error:-1
 */
int SpiDev::SpiSetMaxSpeed(unsigned int speed)
{
	int ret = 0;
	ret = ioctl(this->fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't set max speed hz\n");

	ret = ioctl(this->fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't get max speed hz\n");

	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
}


/**
 * @brief spi transfer
 *
 * @param send_buf buf master device want to send
 * @param recv_buf buf master device want to receive
 * @param len total of send_buf and recv_buf
 *
 * @return success:0, error:-1
 */
int SpiDev::SpiMessageTransfer(unsigned char *send_buf, unsigned char *recv_buf, int len)
{

	struct spi_ioc_transfer tr;
	memset(&tr, 0, sizeof(tr));
	tr.tx_buf = (unsigned long)send_buf;
	tr.rx_buf = (unsigned long)recv_buf;
	tr.len = len;
	if(ioctl(this->fd, SPI_IOC_MESSAGE(1), &tr) < 0){
		printf("unable to transfer data\n");
		return -1;
	}
	return 0;
}





#define SPI_READ_MODE 	1
#define SPI_WRITE_MODE  0

#define MAX_BUF 100



class SpiDev{
	int fd;

public:
	SpiDev(unsigned int bus_id, unsigned int chip_id);

	~SpiDev();

	int SpiOpenDev(unsigned int bus_id, unsigned int chip_id);

	int SpiSetMode(unsigned short mode);

	int SpiSetBitsPerWord(unsigned short bits);

	int SpiSetMaxSpeed(unsigned int speed);

	int SpiMessageTransfer(unsigned char *send_buf, unsigned char *recv_buf, int len);

};

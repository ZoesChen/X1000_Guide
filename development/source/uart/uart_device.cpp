#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "uart_device.h"

UART_device::UART_device(const int uart) : uart(uart)
{
	std::cout << "Initialize the uart:" << uart << std::endl;

	memset(&ntm, 0,sizeof(ntm));

	pthread_mutex_init(&mt,NULL);
	sprintf(name,"/dev/ttyS%d",uart);
	fd = open(name, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd <0)
	{
		printf("open %s error %s\n",name,strerror(errno));
	}
	tcgetattr(fd,&otm);
}

UART_device::~UART_device()
{
	if(fd>0)
	{
		tcsetattr(fd,TCSANOW,&otm);
		close(fd);
		fd = -1;
	}
}

int UART_device::setUARTSpeed(int speed,int hardware)
{
	int i;

	tcgetattr(fd,&ntm);
	ntm.c_cflag = CLOCAL | CREAD;
	if(hardware)
		ntm.c_cflag |= CRTSCTS;

	switch(speed)
	{
	case 300:
		ntm.c_cflag |= B300;
		break;
	case 1200:
		ntm.c_cflag |= B1200;
		break;
	case 2400:
		ntm.c_cflag |= B2400;
		break;
	case 4800:
		ntm.c_cflag |= B4800;
		break;
	case 9600:
		ntm.c_cflag |= B9600;
		break;
	case 19200:
		ntm.c_cflag |= B19200;
		break;
	case 38400:
		ntm.c_cflag |= B38400;
		break;
	case 57600:
		ntm.c_cflag |= B57600;
		break;
	case 115200:
		ntm.c_cflag |= B115200;
		break;
	default:
		printf("Baudrate fail!current = %d\n",speed);
	}
	ntm.c_iflag = IGNPAR;
	ntm.c_oflag = 0;
	tcsetattr(fd,TCSANOW,&ntm);
	tcflush(fd, TCIFLUSH);

	return 0;
}

int UART_device::setUARTParity(int databits,int parity,int stopbits)
{

	if( tcgetattr(fd,&ntm) != 0)
	{
		std::cout << "SetupSerial " << name << std::endl;
		return 1;
	}

	ntm.c_cflag &= ~CSIZE;
	switch (databits)
	{

	case 7:
		ntm.c_cflag |= CS7;
		break;
	case 8:
		ntm.c_cflag |= CS8;
		break;
	default:
		std::cout << "Unsupported data size" << std::endl;
		return 5;
	}

	switch (parity)
	{

	case 'n':
	case 'N':
		ntm.c_cflag &= ~PARENB; /* Clear parity enable */
		ntm.c_iflag &= ~INPCK; /* Enable parity checking */
		break;
	case 'o':
	case 'O':
		ntm.c_cflag |= (PARODD|PARENB);
		ntm.c_iflag |= INPCK; /* Disnable parity checking */
		break;
	case 'e':
	case 'E':
		ntm.c_cflag |= PARENB; /* Enable parity */
		ntm.c_cflag &= ~PARODD;
		ntm.c_iflag |= INPCK; /* Disnable parity checking */
		break;
	case 'S':
	case 's': /*as no parity*/
		ntm.c_cflag &= ~PARENB;
		ntm.c_cflag &= ~CSTOPB;
		break;
	default:
		std::cout << "Unsupported parity" << std::endl;
		return 2;
	}

	switch (stopbits)
	{
	case 1:
		ntm.c_cflag &= ~CSTOPB;
		break;
	case 2:
		ntm.c_cflag |= CSTOPB;
		break;
	default:
		std::cout << "Unsupported stop bits" << std::endl;
		return 3;
	}

	ntm.c_lflag = 0;
	ntm.c_cc[VTIME] = 0; // inter-character timer unused

	ntm.c_cc[VMIN] = 0; // blocking read until 1 chars received

	if (tcsetattr(fd,TCSANOW,&ntm) != 0)
	{
		std::cout << "SetupSerial" << std::endl;
		return 4;
	}
	tcflush(fd, TCIFLUSH);

	return 0;
}

int UART_device::recvnUART(unsigned char *pbuf,int size)
{
	int ret,left,bytes;

	left = size;

	while(left>0)
	{
		ret = 0;
		bytes = 0;

		pthread_mutex_lock(&mt);
		ioctl(fd, FIONREAD, &bytes);
		if(bytes>0)
		{
			ret = read(fd,pbuf,left);
		}
		pthread_mutex_unlock(&mt);
		if(ret >0)
		{
			left -= ret;
			pbuf += ret;
		}
//		usleep(100);
	}

	return size - left;
}

int UART_device::sendnUART(unsigned char *pbuf,int size)
{
	int ret,nleft;
	char *ptmp;

	ret = 0;
	nleft = size;
	ptmp = (char *)pbuf;

	while(nleft>0)
	{
		pthread_mutex_lock(&mt);
		ret = write(fd,ptmp,nleft);
		pthread_mutex_unlock(&mt);

		if(ret >0)
		{
			nleft -= ret;
			ptmp += ret;
		}
		//usleep(100);

	}

	return size - nleft;
}

int UART_device::lockUART()
{
	if(fd < 0)
	{
		return 1;
	}

	return flock(fd,LOCK_EX);
}
int UART_device::unlockUART()
{
	if(fd < 0)
	{
		return 1;
	}

	return flock(fd,LOCK_UN);
}

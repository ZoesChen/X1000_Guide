#ifndef __UART_H__
#define __UART_H__

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <pthread.h>
#include <sys/file.h>

class UART_device{

public:
	UART_device(const int uart);
	~UART_device();
	/**
	 * @brief  Set uart baud rate
	 *
	 * @returns
	 *		success ---> 0
	 *		fail  ---> -1
	 */
	int setUARTSpeed(int speed,int hardware);
	/**
	 * @brief  Set uart databits, parity and stopbits
	 *
	 * @returns
	 *		success ---> 0
	 *		fail  ---> 1-5
	 */
	int setUARTParity(int databits,int parity,int stopbits);
	/**
	 * @brief  Send data
	 *
	 * @returns
	 *		size - left
	 */
	int sendnUART(unsigned char *pbuf,int size);
	/**
	 * @brief  Receive data
	 *
	 * @returns
	 *		size - nleft
	 */
	int recvnUART(unsigned char *pbuf,int size);
	int lockUART();
	int unlockUART();

private:
	const int uart;
	int fd;
	pthread_mutex_t mt;
	char name[24];
	struct termios ntm;
	struct termios otm;
};
#endif /* __UART_H__ */

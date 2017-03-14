#include <iostream>
#include <uart_device.h>

int main(int argc,char **argv)
{
	UART_device uart_1(1);
	UART_device uart_2(2);
	int nbyte,idx,nbyte1;
	unsigned char receive;
	unsigned char send;
	pid_t fpid;
	int count=0;
	if(uart_1.setUARTSpeed(115200,0)>0)
	{
		std::cout << "setUARTSpeed() error" << std::endl;
		return -1;
	}
	if(uart_1.setUARTParity(8,'N',1)>0)
	{
		std::cout << "setUARTParity() error" << std::endl;
		return -1;
	}

	if(uart_2.setUARTSpeed(115200,0)>0)
	{
		std::cout << "setUARTSpeed() error" << std::endl;
		return -1;
	}
	if(uart_2.setUARTParity(8,'N',1)>0)
	{
		std::cout << "setUARTParity() error" << std::endl;
		return -1;
	}

	fpid=fork();
	if (fpid < 0)
		std::cout << "error in fork!" << std::endl;
	else if (fpid == 0) {
		while(1)
		{
			nbyte1 = uart_1.recvnUART(&send,1);
			if(nbyte1) {
				uart_2.sendnUART(&send,1);
				std::cout << send << std::endl;
			}
		}
	}
	else {
		while(1)
		{
			nbyte = uart_2.recvnUART(&receive,1);
			if(nbyte) {
				uart_1.sendnUART(&receive,1);
				std::cout << receive << std::endl;
			}
		}
	}
}

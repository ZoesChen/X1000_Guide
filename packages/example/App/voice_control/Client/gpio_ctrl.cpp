#include <stdbool.h>
#include "gpio_device.h"

GPIO_device gpio12(12);

void gpio_int(void)
{
	gpio12.export_gpio();
}

int command_handle(const char *command)
{
	if(strcmp("TurnOff",command)==0
		|| strcmp("关灯",command)==0)
	{
		printf( "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!TurnOff" );
		gpio12.write_gpio("direction","out");
		gpio12.write_gpio("value","0");
		printf("close light ok!\n");
	}
	else if(strcmp("TurnON",command)==0
			|| strcmp("开灯",command)==0	)
	{
		printf( "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!TurnOn" );
		gpio12.write_gpio("direction","out");
		gpio12.write_gpio("value","1");
		printf("open light ok!\n");
	}

}

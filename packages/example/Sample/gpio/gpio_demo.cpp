#include <iostream>
#include <gpio_device.h>

int main(){
	GPIO_device phoenix(12);
	phoenix.export_gpio();

	std::cout << "Set pin 12 with high\n";
	phoenix.write_gpio("direction", "high");//"in"; or "low"for output 0; or "high" for output 1

	std::cout << "Set pin 12 edge to both\n";//"both" or "none", "rising", "falling"
	phoenix.write_gpio("edge", "both");

//	std::cout << "Set pin 12 active_low to 0\n";
//	phoenix.write_gpio("active_low", "0");// "0" means the value=1=high value=0=low; "1" means the value=1=low value=0=high default active_low=0

	if(phoenix.poll_gpio()){
		std::cout << "Pin 12 pushd\n";
	}

	return 0;
}



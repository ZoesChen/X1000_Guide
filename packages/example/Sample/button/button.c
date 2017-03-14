#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

#define BUTTON_EVENT "/dev/input/event1"
int main(int argc,char *argv[])
{
	int fd;
	struct input_event event;
	int rd;

	fd = open(BUTTON_EVENT,O_RDONLY);

	while(1){
		rd = read(fd,&event,sizeof(struct input_event));
		if ( rd < sizeof(struct input_event) ) return 0;
		printf("===================== type = %d code = %d value = %d\n",event.type,event.code,event.value);
	}
	close(fd);

	return 0;
}

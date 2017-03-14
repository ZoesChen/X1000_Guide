#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include "key.h"

static int keyFd = -1;
static struct input_event *event = NULL;

int OpenKeyDev()
{
	printf("Enter into OpenKeyDev\n");
	keyFd = open(KEY_DEVICE, O_RDONLY);
	if(keyFd < 0) {
		printf("Open %s fail\n", KEY_DEVICE);
		return -1;
	}

	event = (struct input_event *)malloc(sizeof(struct input_event));
	if (event == NULL) {
		printf("Have no memory\n");
		close(keyFd);
		return -1;
	}
	
	return 0;
}

int ReadKey(int *keyCode)
{
	int res;
	if (event) {
		res = read(keyFd, event, sizeof(struct input_event));
		if(res < sizeof(struct input_event)) {
			printf("Read KeyDevice Fail\n");
			return -1;
		}
		if (event->type == EV_KEY) {
			*keyCode = event->code;
		}
	} else {
		printf("No Event\n");
		return -1;
	}
	return 0;		
}

void CloseKeyDev()
{
	if (keyFd)
		close(keyFd);
	if (event)
		free(event);
}
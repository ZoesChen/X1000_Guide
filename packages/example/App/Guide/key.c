#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include "key.h"

static int keyFd = -1;
static struct input_event *event = NULL;
static int old_sec = -1;
static int new_sec = -1;
static int old_key = -1;
static int new_key = -1;

int OpenKeyDev()
{
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
		if (event->type == EV_KEY) {
			old_key = new_key;
			old_sec = new_sec;
		}
		res = read(keyFd, event, sizeof(struct input_event));

		if(res < sizeof(struct input_event)) {
			printf("Read KeyDevice Fail\n");
			return -1;
		}
		
		if (event->type == EV_KEY) {
			new_sec = event->time.tv_sec;
			new_key = event->code;
			printf("\n********************new_key = %d, new sec=%d********************\n", new_key, new_sec);
			if (new_key == old_key && (new_sec - old_sec) < 1) {
				//printf("[%s] the same key %d report too frequence %d!\n", __FUNCTION__, new_key, (new_sec - old_sec));
				return -1;
			}
			*keyCode = event->code;
		} else {
			return -1;
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
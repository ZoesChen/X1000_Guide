#ifndef _GUIDE_KEY_H_
#define _GUIDE_KEY_H_

#define KEY_DEVICE "/dev/input/event1"
#define CHIGOO_KEY1	1
#define CHIGOO_KEY2 2
#define CHIGOO_KEY3 3
#define CHIGOO_KEY4 4
#define CHIGOO_KEY5 5
#define CHIGOO_KEY6 6
#define CHIGOO_KEY7 7
#define CHIGOO_KEY8 8
#define CHIGOO_KEY9 9
#define CHIGOO_KEY0 10
#define CHIGOO_KEYCE 11
#define CHIGOO_KEYOK 12
#define CHIGOO_LANGUAGE	13
#define CHIGOO_VOLUME_PLUSE 14
#define CHIGOO_VOLUME_REDUSE 15
#define CHIGOO_BACK 16


typedef enum cmdType {
	MUSIC_CMD,
	OPTION_CMD,
	NUMBER_CMD,
	LOCATION_MUSIC_CMD,
	INVAILD_CMD
}CMDTYPE;

typedef struct cmdMsg {
	CMDTYPE	cmdType;
	int			keyNum;
	unsigned long int		locationNum;
	char			cmdValue[4];
}CMDMSG;

int OpenKeyDev();
int ReadKey(int *keyCode);
void CloseKeyDev();
#endif

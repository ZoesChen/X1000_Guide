#ifndef _GUIDE_KEY_H_
#define _GUIDE_KEY_H_

#define KEY_DEVICE "/dev/input/event0"
#define A90_GUIDE_KEY0 		10
#define A90_GUIDE_KEY1 		1
#define A90_GUIDE_KEY2 		2
#define A90_GUIDE_KEY3 		3
#define A90_GUIDE_KEY4 		4
#define A90_GUIDE_KEY5 		5
#define A90_GUIDE_KEY6 		6
#define A90_GUIDE_KEY7 		7
#define A90_GUIDE_KEY8 		8
#define A90_GUIDE_KEY9 		9
#define A90_GUIDE_KEYCE		11
#define A90_GUIDE_KEYOK		12
#define A90_GUIDE_KEYP		13 // Play & Pause
#define A90_GUIDE_KEY_VALUEUP 14
#define A90_GUIDE_KEY_VALUEDOWN 15
#define A90_GUIDE_KEY_BACK	16

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

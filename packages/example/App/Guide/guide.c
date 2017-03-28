#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include "key.h"
#include "location.h"
#include "queue.h"
#include "play.h"

/*
 * Define
 * */

//#define DEBUG_X86
#define DEBUG_ON_DEVBOARD

#define	MAX_MUSIC_INDEX	4	 // 4bit num -> 1 music num
#define	PLAYCMD_INIT_STAT 	(-1)

#define	SUCCESS		0
#define	FAIL			(-1)

#define LED_R_LIGHT		0x01
#define LED_G_LIGHT		0x10
#define LED_B_LIGHT		0x11
#define ALL_OFF			0x00

#define OVERTIMER		(10 *60 * 1000) //10minutes

#define CHARGING 		1
#define UNCHARGING		0
#define LOWPOWER		0
#define NOMALPOWER	1


/*
 * Parameter
 * */
static pthread_t keyThread;
static pthread_t locationThread;
static pthread_t playThread;
static pthread_t batteryThread;
static pthread_t idleThread;
static int readKeyFlag = 0;
static int readLocationFlag = 0;
static unsigned long int old_location = 0;
static int needBeganCount = 0;

static int music_num[4] = {0};
static int musicNum = -1;

static int batteryDevFd = -1;

int isKeyMusicPlaying = 0;
static unsigned long int overTimeCount =  OVERTIMER;

//static int location_music_num[5] = {0};
//static int locationMusicNum = -1;

queue keyQueue;
queue locationQueue;
pthread_cond_t playThreadCond;
pthread_mutex_t playThreadLock;

/*
 * Struct & Enum
 * */
enum QueueCondition
{
	KEYQUEUE_FREE,
	LOCATIONQUEUE_FREE,
	BOTH_FREE,
	BOTH_BUSY,
	INVALID
};

struct ChargeInfo {
	int isCharging;
	int isLowPower;
}chargeInfo;


/************************************  Function ************************************/

static void *KeyThreadHandle(void *arg);
static void *PlayThreadHandle(void *arg);
static void *LocationThreadHandle(void *arg);
static void *BatteryThreadHandle(void *arg);
static void *IdleThreadHandle(void *arg);


static enum QueueCondition whichQueueFree()
{
	enum QueueCondition qc = INVALID;
	if (IsQueueFree(&keyQueue) && IsQueueFree(&locationQueue)) {
		qc = BOTH_FREE;
	} else if (IsQueueFree(&keyQueue)) {
		qc = KEYQUEUE_FREE;
	} else if (IsQueueFree(&locationQueue)) {
		qc = LOCATIONQUEUE_FREE;
	} else {
		qc = BOTH_BUSY;
	}
	return qc;
}

static void InitQueue()
{
	keyQueue.front = NULL;
	keyQueue.end = NULL;
	locationQueue.front = NULL;
	locationQueue.end = NULL;
}

#ifdef DEBUG_ON_DEVBOARD
static void insteadedOkKey(int sig)
{
	signal(sig, SIG_IGN);
	readKeyFlag = 0;
	signal(SIGINT, insteadedOkKey);
}
#endif

static int OpenBatteryDev()
{
	batteryDevFd = open("/dev/char_dev", O_RDWR);
	if (batteryDevFd <= 0) {
		return FAIL;
	} else {
		return SUCCESS;
	}
}

static int Init()
{
	if (OpenKeyDev() < 0) {
		return FAIL;
	}

	if (OpenMcuDev() < 0) {
		return FAIL;
	}

	if (OpenBatteryDev() < 0) {
		return FAIL;
	}
	InitPlay();
	InitQueue();

	pthread_create(&keyThread, NULL, KeyThreadHandle, NULL);
   	pthread_create(&locationThread, NULL, LocationThreadHandle, NULL);
	pthread_create(&playThread, NULL, PlayThreadHandle, NULL);
	pthread_create(&batteryThread, NULL, BatteryThreadHandle, NULL);
	pthread_create(&idleThread, NULL, IdleThreadHandle, NULL);

#ifdef DEBUG_ON_DEVBOARD
//Just for debug, CTRL+C to fill up array music_num, then push into keyqueue
//if not debug on devboard, will be insteaded by ok key
	signal(SIGINT, insteadedOkKey);

#endif

	readKeyFlag = 1;
	readLocationFlag = 1;
	return 0;
}

void *BatteryThreadHandle(void *arg)
{
	int chargeLed = 0;
	while(readKeyFlag) {
		read(batteryDevFd, &chargeInfo, sizeof(chargeInfo));

		#ifdef DEBUG_A90
		printf("%s, %s\n", (chargeInfo.isCharging == CHARGING) ? "Charging" : "UnCharged", 
			(chargeInfo.isLowPower == LOWPOWER) ? "LOW" : "NORMAL");
		#endif

		/*
		*	charging: red light flash	frequnce = 1 s
		*	uncharging && normal power: blue light
		*	uncharging && low power: red light
		*/

		if (chargeInfo.isCharging == CHARGING) {
			if (chargeLed == 0) {
				chargeLed = 1;
				ioctl(batteryDevFd, LED_R_LIGHT, NULL);
			} else {
				chargeLed = 0;
				ioctl(batteryDevFd, ALL_OFF, NULL);
			}
			sleep(1);
		}  else if (chargeInfo.isLowPower == LOWPOWER) {
			ioctl(batteryDevFd, LED_R_LIGHT, NULL);
			sleep(5);//read battery stat erery 5 seconds
		} else {
			ioctl(batteryDevFd, LED_B_LIGHT, NULL);
			sleep(5);//read battery stat erery 5 seconds
		}
	}
	return NULL;
}

void *PlayThreadHandle(void *arg)
{
	CMDMSG *cmdMsg;
	//welcom music
	
	playInterface(MUSIC_CMD, 9999);
	
	while(readKeyFlag) {
		pthread_mutex_lock(&playThreadLock);
		switch(whichQueueFree()){
			case BOTH_FREE:
				//printf("BOTH_FREE\n");
				//when every queue are free, start needBeganCount
				printf("both free, start needBeganCount\n");
				needBeganCount = 1;

				pthread_cond_wait(&playThreadCond, &playThreadLock);

				//when any queue have action, stop needBeganCount
				printf("both free, stop needBeganCount\n");
				needBeganCount = 0;
				usleep(1500);
				printf("%s: Wake up by cond\n", __FUNCTION__);
				pthread_mutex_unlock(&playThreadLock);
				continue;
			break;
			case BOTH_BUSY:
				//printf("BOTH_BUSY\n");
				cmdMsg = Pop(&keyQueue);
			break;
			case KEYQUEUE_FREE:
			//keyqueue NULL, locationQueue not NULL
				//printf("KEYQUEUE_FREE\n");
				cmdMsg = Pop(&locationQueue);
			break;
			case LOCATIONQUEUE_FREE:
				printf("LOCATIONQUEUE_FREE\n");
				cmdMsg = Pop(&keyQueue);
			break;
			default:
			break;
		}
		pthread_mutex_unlock(&playThreadLock);

		switch (cmdMsg->cmdType) {
			case MUSIC_CMD:
				//printf("%s: MUSIC CMD\n", __FUNCTION__);
				musicNum = cmdMsg->cmdValue[3] * 1000 + cmdMsg->cmdValue[2] * 100 + cmdMsg->cmdValue[1] * 10 + cmdMsg->cmdValue[0];
				printf("%s: musicNum is %d\n", __FUNCTION__, musicNum);
				playInterface(cmdMsg->cmdType, musicNum);
				memset(cmdMsg->cmdValue, 0, 4 * sizeof(char));
			break;
			case OPTION_CMD:
				playInterface(cmdMsg->cmdType, cmdMsg->keyNum);
			break;
			case NUMBER_CMD:
				//printf("%s: NUMBER_CMD\n", __FUNCTION__);
				playInterface(cmdMsg->cmdType,  cmdMsg->keyNum);
			break;
			case LOCATION_MUSIC_CMD:
				//printf("%s: LOCATION_MUSIC_CMD\n", __FUNCTION__);
				//printf("%s: locationInfo is %ld\n", __FUNCTION__, cmdMsg->locationNum);
				playInterface(cmdMsg->cmdType, cmdMsg->locationNum);
			break;
			default:
				break;
		}
		
		//free(cmdMsg);
	}
	return NULL;
}

CMDTYPE JudgeKeyCmd(int *value)
{
	int key = *value;
	CMDTYPE cmdType = INVAILD_CMD;
	if (key >= CHIGOO_KEY1 && key <= CHIGOO_KEY0) {
		cmdType = NUMBER_CMD;
		if (key == CHIGOO_KEY0) {
			*value = 0;
		}
	} else if (key >= CHIGOO_KEYCE && key <= CHIGOO_BACK) {
		cmdType = OPTION_CMD;
	}
	return cmdType;
}

void *IdleThreadHandle(void *arg)
{
	while(readKeyFlag) {
		while (needBeganCount) {
			usleep(1000);
			if (overTimeCount-- <= 0) {
				overTimeCount = OVERTIMER;
				system("echo mem >/sys/power/state");
			}
		} 
		printf("needBeganCount be stoped! \n");
		overTimeCount = OVERTIMER;

		//every 30 seconds to check needBeganCount
		sleep(30);
	}
}

void *KeyThreadHandle(void *arg)
{
	int keyCode;
	int musicNumIndex = 0;

	struct cmdMsg *cmdMsg = (struct cmdMsg *)malloc(sizeof(struct cmdMsg));
	if (cmdMsg == NULL) {
		printf("%s: Requre memory fail\n", __FUNCTION__);
		return NULL;
	} else {
		memset(cmdMsg, 0, sizeof(CMDMSG));
	}

	while(readKeyFlag) {
		if (ReadKey(&keyCode) < 0) {
			usleep(10); // avoid readKey always return error cause cpu loading too high
			continue;
		}
		cmdMsg->cmdType = JudgeKeyCmd(&keyCode);

		if (NUMBER_CMD == cmdMsg->cmdType) {
			cmdMsg->keyNum = keyCode;
			printf("[%s] receive %d key\n", __FUNCTION__, keyCode);
			//push number cmd msg into keyqueue 
			pthread_mutex_lock(&playThreadLock);
			//printf("[%s] After lock\n", __FUNCTION__);
			Push(cmdMsg, &keyQueue);
			pthread_cond_signal(&playThreadCond);
			pthread_mutex_unlock(&playThreadLock);
			music_num[musicNumIndex++] = keyCode;
		} else if (OPTION_CMD == cmdMsg->cmdType) {
			cmdMsg->keyNum = keyCode;
			//ToDo: implement option cmd;
			pthread_mutex_lock(&playThreadLock);
			//printf("[%s] After lock\n", __FUNCTION__);
			Push(cmdMsg, &keyQueue);
			pthread_cond_signal(&playThreadCond);
			pthread_mutex_unlock(&playThreadLock);

		}

		//wait something finish
		sleep(1);

		if (musicNumIndex >= MAX_MUSIC_INDEX || keyCode == CHIGOO_KEYOK) {
			int i;
			cmdMsg->cmdType = MUSIC_CMD;
			//when key music playing, location music can not play
			isKeyMusicPlaying = 1;

			//reset timer when this key action is a right action
			exit_time();
			for (i = 0; i < musicNumIndex; i++) {
				cmdMsg->cmdValue[musicNumIndex - i -1] = music_num[i];
			}

			printf("%s: In MUSIC CMD, music number is %d\n", __FUNCTION__, 
				cmdMsg->cmdValue[3] * 1000 + 	cmdMsg->cmdValue[2] * 100 +
				cmdMsg->cmdValue[1] * 10 + cmdMsg->cmdValue[0]);
			
			pthread_mutex_lock(&playThreadLock);
			Push(cmdMsg, &keyQueue);
			pthread_cond_signal(&playThreadCond);
			pthread_mutex_unlock(&playThreadLock);
			//memset(cmdMsg->cmdValue, 0, 4 * sizeof(char));
			musicNumIndex = 0;
		}
	}

	printf("Will return from keyThread\n");
	return NULL;
}

void *LocationThreadHandle(void *arg)
{
	unsigned long int location;
	while(readLocationFlag) {
		ReadLocationInfo(&location);

		//cancel key music first when user go to a new area, just judge IR num do not care geomagnetic inifo
		if (old_location != location / 10000) {
			old_location = location / 10000;
			if (isKeyMusicPlaying == 1)
				isKeyMusicPlaying = 0;
		}

		if (isKeyMusicPlaying == 1) {
			sleep(1);
			printf("%s: key music is playing, wait it finish...\n", __FUNCTION__);
			continue;
		}

		//locationMusicNum = 1111;
		struct cmdMsg *cmdMsg = (struct cmdMsg *)malloc(sizeof(struct cmdMsg));
		cmdMsg->cmdType = LOCATION_MUSIC_CMD;
		cmdMsg->locationNum = location;
		pthread_mutex_lock(&playThreadLock);
		Push(cmdMsg, &locationQueue);
		pthread_cond_signal(&playThreadCond);
		pthread_mutex_unlock(&playThreadLock);
	}
	printf("Will return from locationThread\n");
	return NULL;
}

int main(int argv, char **argc)
{
	printf("Begin......\n");
	if (Init() < 0) {
		return -1;
	}

	pthread_join(keyThread, NULL);
        pthread_join(locationThread, NULL);
	pthread_join(playThread, NULL);
	return 0;
}




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

/*
 * Parameter
 * */
static pthread_t keyThread;
static pthread_t locationThread;
static pthread_t playThread;
static int readKeyFlag = 0;
static int readLocationFlag = 0;

static int music_num[4] = {0};
static int musicNum = -1;

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


/************************************  Function ************************************/

static void *KeyThreadHandle(void *arg);
static void *PlayThreadHandle(void *arg);
static void *LocationThreadHandle(void *arg);

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

static int Init()
{
	if (OpenKeyDev() < 0) {
		return FAIL;
	}

	if (OpenMcuDev() < 0) {
		return FAIL;
	}
	
	InitPlay();
	InitQueue();

	pthread_create(&keyThread, NULL, KeyThreadHandle, NULL);
   	pthread_create(&locationThread, NULL, LocationThreadHandle, NULL);
	pthread_create(&playThread, NULL, PlayThreadHandle, NULL);

#ifdef DEBUG_ON_DEVBOARD
//Just for debug, CTRL+C to fill up array music_num, then push into keyqueue
//if not debug on devboard, will be insteaded by ok key
	signal(SIGINT, insteadedOkKey);

#endif

	readKeyFlag = 1;
	readLocationFlag = 1;
	return 0;
}

void *PlayThreadHandle(void *arg)
{
	CMDMSG *cmdMsg;
	
	while(readKeyFlag) {
		pthread_mutex_lock(&playThreadLock);
		switch(whichQueueFree()){
			case BOTH_FREE:
				printf("BOTH_FREE\n");
				pthread_cond_wait(&playThreadCond, &playThreadLock);
				printf("%s: Wake up by cond\n", __FUNCTION__);
				pthread_mutex_unlock(&playThreadLock);
				continue;
			break;
			case BOTH_BUSY:
				printf("BOTH_BUSY\n");
				cmdMsg = Pop(&keyQueue);
			break;
			case KEYQUEUE_FREE:
			//keyqueue NULL, locationQueue not NULL
				printf("KEYQUEUE_FREE\n");
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
				printf("%s: MUSIC CMD\n", __FUNCTION__);
				musicNum = cmdMsg->cmdValue[3] * 1000 + cmdMsg->cmdValue[2] * 100 + cmdMsg->cmdValue[1] * 10 + cmdMsg->cmdValue[0];
				printf("%s: musicNum is %d\n", __FUNCTION__, musicNum);
				playInterface(cmdMsg->cmdType, musicNum);
				memset(cmdMsg->cmdValue, 0, 4 * sizeof(char));
			break;
			case OPTION_CMD:
				printf("%s: OPTION_CMD\n", __FUNCTION__);
			break;
			case NUMBER_CMD:
				printf("%s: NUMBER_CMD\n", __FUNCTION__);
				playInterface(cmdMsg->cmdType,  cmdMsg->keyNum);
			break;
			case LOCATION_MUSIC_CMD:
				printf("%s: LOCATION_MUSIC_CMD\n", __FUNCTION__);
				printf("%s: locationInfo is %ld\n", __FUNCTION__, cmdMsg->locationNum);
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
	if (key >= A90_GUIDE_KEY1 && key <= A90_GUIDE_KEY0) {
		cmdType = NUMBER_CMD;
		if (key == A90_GUIDE_KEY0) {
			*value = 0;
		}
	} else if (key >= A90_GUIDE_KEYCE && key <= A90_GUIDE_KEY_BACK) {
		cmdType = OPTION_CMD;
	}
	return cmdType;
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
			//......
		}

		//wait something finish
		sleep(1);

		if (musicNumIndex >= MAX_MUSIC_INDEX || keyCode == A90_GUIDE_KEYOK) {
			int i;
			cmdMsg->cmdType = MUSIC_CMD;
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




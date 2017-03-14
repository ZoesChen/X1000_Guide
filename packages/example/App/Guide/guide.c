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
#define MAX_MUSIC_INDEX 4 // 4bit num -> 1 music num
#define PLAYCMD_INIT_STAT -1

/*
 * Parameter
 * */
static pthread_t keyThread;
static pthread_t locationThread;
static pthread_t playThread;
static int readKeyFlag = 0;
static int readLocationFlag = 0;

#define DEBUG_X86
//X86_ESC: stop read
//X86_ZERO: put music num into queue

static int music_num[4] = {0};
static int musicNum = -1;

static int location_music_num[5] = {0};
static int locationMusicNum = -1;
queue keyQueue;
queue locationQueue;
pthread_cond_t playThreadCond;
pthread_mutex_t playThreadLock;

#ifdef DEBUG_X86
static int oldKey       = -1;
static int newKey       = -1;
#endif

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

static void StopThread(int sig)
{
    /* allow the stream to be closed gracefully */
    signal(sig, SIG_IGN);
    readKeyFlag = 0;
}

static int Init()
{
	int res,ret;
	res = OpenKeyDev();
	if(res) {
		return -1;
	}
	ret = OpenMcuDev();
    if(ret < 0) {
        return -1;
    }
	InitPlay();
	InitQueue();
	
	signal(SIGINT, StopThread);
	
	readKeyFlag = 1;
    readLocationFlag = 1;
	return 0;
}

void *PlayThreadHandle(void *arg)
{
	int playCmd = PLAYCMD_INIT_STAT;
	while(readKeyFlag) {
		pthread_mutex_lock(&playThreadLock);
		switch(whichQueueFree()){
			case BOTH_FREE:
				printf("BOTH_FREE\n");
				pthread_cond_wait(&playThreadCond, &playThreadLock);
				printf("Wake up by cond\n");
			break;
			case BOTH_BUSY:
				printf("BOTH_BUSY\n");
				playCmd = Pop(&keyQueue);
			break;
			case KEYQUEUE_FREE:
			//keyqueue NULL, locationQueue not NULL
				printf("KEYQUEUE_FREE\n");
				playCmd = Pop(&locationQueue);
			break;
			case LOCATIONQUEUE_FREE:
				printf("LOCATIONQUEUE_FREE\n");
				playCmd = Pop(&keyQueue);
			break;
			default:
			break;
		}
		pthread_mutex_unlock(&playThreadLock);
		if (playCmd != PLAYCMD_INIT_STAT) {
			//Handle playCmd && reset playCmd
			printf("PlayThreadHandle: playCmd = %d\n", playCmd);
			//StartPlay(playCmd);
			playInterface(playCmd);
			playCmd = PLAYCMD_INIT_STAT;
		}
	}
	return NULL;
}

void *KeyThreadHandle(void *arg)
{
	int keyCode;
	int musicNumIndex = 0;
#ifdef DEBUG_X86
	int x86StartFlag = 0;
#endif
	while(readKeyFlag) {
		ReadKey(&keyCode);
#ifdef DEBUG_X86
		if (keyCode == X86_KEY_A) {
			x86StartFlag = 1;
			continue;
		}
		if (x86StartFlag != 1) {
			continue;
		}

		oldKey = newKey;
		newKey = keyCode;
		if (oldKey == newKey)
			continue;

		if (keyCode == X86_ESC) {
			printf("Press esc\n");
			readKeyFlag = 0;
			continue;
		}
#endif
		if (keyCode >= X86_KEY1 && keyCode <= X86_ZERO) {
			if (musicNumIndex < MAX_MUSIC_INDEX) {
				if (keyCode == X86_ZERO)
					music_num[musicNumIndex] = 0;
				else
					music_num[musicNumIndex] =keyCode -1;

				musicNumIndex++;
			}

			if (musicNumIndex == MAX_MUSIC_INDEX) {
				int i = 0;
				
				musicNum = music_num[0]*1000 + music_num[1]*100 + music_num[2]*10 + music_num[3];
				musicNumIndex = 0;
				printf("MusicNum = %d\n", musicNum);
				for(i = 0; i < MAX_MUSIC_INDEX; i++) {
					music_num[i] = 0;
				}
				//Enqueue music num into keyQueue
				pthread_mutex_lock(&playThreadLock);
				Push(musicNum, &keyQueue);
				pthread_cond_signal(&playThreadCond);
				pthread_mutex_unlock(&playThreadLock);
			}
		}
	}
#ifdef DEBUG_X86
	x86StartFlag = 0;
#endif
	printf("Will return from keyThread\n");
	return NULL;
}

void *LocationThreadHandle(void *arg)
{
	int location;
	int musicNumIndex = 0;
	while(readLocationFlag) {
		ReadLocationInfo(&location);
		if (location >= X86_KEY1 && location <= X86_ZERO) {
			if (musicNumIndex < MAX_MUSIC_INDEX) {
				if (location == X86_ZERO)
					location_music_num[musicNumIndex] = 0;
				else
					location_music_num[musicNumIndex] =location -1;

				musicNumIndex++;
			}

			if (musicNumIndex == MAX_MUSIC_INDEX) {
				int i = 0;
				
				locationMusicNum = location_music_num[0]*1000 + location_music_num[1]*100 + location_music_num[2]*10 + location_music_num[3];
				musicNumIndex = 0;
				printf("MusicNum = %d\n", locationMusicNum);
				for(i = 0; i < MAX_MUSIC_INDEX; i++) {
					music_num[i] = 0;
				}
				//Enqueue music num into locationQueue
				pthread_mutex_lock(&playThreadLock);
				Push(locationMusicNum, &locationQueue);
				pthread_cond_signal(&playThreadCond);
				pthread_mutex_unlock(&playThreadLock);
			}
		}
	}
	printf("Will return from keyThread\n");
	return NULL;
}

int main(int argv, char **argc)
{
	printf("Begin......\n");
	if (Init() < 0) {
		return -1;
	}

	pthread_create(&keyThread, NULL, KeyThreadHandle, NULL);
   	pthread_create(&locationThread, NULL, LocationThreadHandle, NULL);
	pthread_create(&playThread, NULL, PlayThreadHandle, NULL);
	pthread_join(keyThread, NULL);
    pthread_join(locationThread, NULL);
	return 0;
}




#ifndef __SOUND_DEV_H__
#define __SOUND_DEV_H__
#include "ai_main.h"
enum AEC_STATUS
{
    AEC_SLEEP = 0,
    AEC_WAKEUP,
    AEC_WAKEUP_TID2_EXIT,
    AEC_WAKEUP_TID1_EXIT,
    AEC_END,
};

enum snd_dev_type {
	SND_DEV_DMIC = 1,
	SND_DEV_AMIC,
};

/* sound device parameters */
#define AEC_BUFFER_SIZE     (1024)
#define MS                  (32)
#define VOLUME              (45)
#define BIT  	            (16)
#define CHANEL_1            (1)
#define CHANEL_4            (4)
#define RATE   	            (16000)
#define PERIOD_COUNT	    (4)
#define TRUE 	            (1)
#define FALSE 	            (0)
#define AEC_PRINT           AIENGINE_PRINT

/* ioctl command */
#define SNDCTL_DSP_AEC_START _SIOW ('J', 16, int)
#define SNDCTL_DSP_AEC_ABLE  _SIOW ('J', 17, int)

/* function declaration */
int pipe_init(void);
void pipe_close(void);
int sound_device_init_near(int val, int chans);
int sound_device_init_far(void);
int sound_device_init(int vol, int chans);
void sound_device_release(void);
int sound_aec_enable(void);
int sound_aec_disable(void);

/* thread function */
void * dmic_read(void *ptr);
void * loopback_read(void *ptr);
void * aec_handle(void *ptr);


int snd_read(enum snd_dev_type type, void *bufp, unsigned int len);

#endif


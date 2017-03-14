#ifndef __SAMPLE_ON_TIME_H__
#define __SAMPLE_ON_TIME_H__

#include "circ_buf.h"


#define DMIC_DEVSTR	"hw:0,2"
#define AMIC_DEVSTR	"hw:0,0"
#define MAX_BUFFER_TIME 500000  /*  50 us */
#define SOFT_RESAMPLE   0       /*  whether enable soft resample or not */

#define RECORD_BUFSZ 1024
#define MAX_RING_BUFFER_SIZE 1024*8




#define MAX_SUPPORT_DEVICES 10



typedef struct alsa_device {
	snd_pcm_t *handle;
	char devstr[32];
	unsigned int period_size;	/* number of frames per period */
	unsigned int bits_per_sample;
	unsigned int bits_per_frame;
	unsigned int period_bytes;	/* number of bytes per period */

	unsigned int channels;
	unsigned int rate;
	unsigned int fmt;
} alsa_device_t;






typedef struct{
	alsa_device_t *alsa_device;
	short buf[RECORD_BUFSZ];
	struct circbuf *circbuf;
	pthread_t pid;
	int exit_flag;

}record_device_t;

typedef struct {
	record_device_t *devices[MAX_SUPPORT_DEVICES];
	int nr_devs;
	pthread_mutex_t lock;
} SampleOnTime_t;



#endif


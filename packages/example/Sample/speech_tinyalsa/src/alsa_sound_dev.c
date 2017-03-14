#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include "echo_wakeup.h"
#include "sound_dev.h"

#include <tinyalsa/asoundlib.h>

#define DMIC_DEVSTR	"hw:0,2"
#define AMIC_DEVSTR	"hw:0,0"
#define MAX_BUFFER_TIME 500000  /*  50 us */
#define SOFT_RESAMPLE   0       /*  whether enable soft resample or not */

//#define USE_AMIC_ECHO	 not ok!

typedef struct alsa_device {
	char devstr[32];
	unsigned int period_size;	/* number of frames per period */
	unsigned int bits_per_sample;
	unsigned int bits_per_frame;
	unsigned int period_bytes;	/* number of bytes per period */

	unsigned int channels;
	unsigned int rate;
	unsigned int fmt;
} alsa_device_t;


alsa_device_t *dmic_device = NULL;
struct pcm * dmic_pcm = NULL;

/* Extern Variables!! */
extern int aec_wakeup_flag;
extern int error_flag;
static int alsa_init_dev(alsa_device_t *alsa_device, enum pcm_format format,
		unsigned int access, unsigned int channels,
		unsigned int rate, int soft_resample, int latency)
{
	struct pcm *pcm = NULL;
	struct pcm_config config;
	char *devstr = alsa_device->devstr;
	int card,device;
	unsigned int buffer_time = latency; //buffer time max
	unsigned int period_time =buffer_time / PERIOD_COUNT; //four periods
	unsigned int period_size = (rate / 1000)*(period_time / 1000)/channels;   // frames count
	unsigned int period_count = PERIOD_COUNT;
	config.channels = channels;
	config.rate = rate;
	config.period_size = period_size;
	config.period_count = period_count;
	config.format = format;
	config.start_threshold = 0;
	config.stop_threshold = 0;
	config.silence_threshold = 0;

	sscanf(alsa_device->devstr,"hw:%d,%d",&card,&device);

	pcm=pcm_open(card,device,access,&config);
	if(!pcm || !pcm_is_ready(pcm))
	{
		fprintf(stderr,"cannot open audio device %s (%s)\n",devstr,pcm_get_error(pcm));
		if(pcm != NULL)
		{
			free(pcm);
			pcm=NULL;
		}
		return -1;
	}
	dmic_pcm=pcm;

	alsa_device->period_size = period_size;
	alsa_device->bits_per_sample = pcm_format_to_bits(format) ;
	alsa_device->bits_per_frame = alsa_device->bits_per_sample * channels;
	alsa_device->period_bytes = alsa_device->period_size * alsa_device->bits_per_frame / 8;

	alsa_device->channels = channels;
	alsa_device->fmt = format;
	alsa_device->rate = rate;

	return 0;
}

int sound_device_init_near(int val, int chans)
{
	/* main dmic */
	int ret = -1;
	enum pcm_format format = PCM_FORMAT_S16_LE;
	unsigned int access=PCM_IN;
	unsigned int channels = chans;
	unsigned int rate = RATE;
	int soft_resample = SOFT_RESAMPLE;      /*  disable soft resample */

	unsigned int latency = MAX_BUFFER_TIME; /*  latency in us */
	dmic_device = malloc(sizeof(alsa_device_t));
	printf("dmic_device: %p, size: %d\n", dmic_device, sizeof(alsa_device_t));
	if(!dmic_device) {
		printf("alloc dmic_device error!\n");
		return -1;
	}

	memset(dmic_device->devstr, 0, sizeof(dmic_device->devstr));
	memcpy(dmic_device->devstr, DMIC_DEVSTR, strlen(DMIC_DEVSTR));

	ret = alsa_init_dev(dmic_device, format, access, channels, rate, soft_resample, latency);
	if(ret < 0) {
		printf("init alsa device faild!\n");
		free(dmic_device);
		dmic_device = NULL;
		return ret;
	}

	return 0;
}


int sound_device_init(int vol, int chans)
{
    int ret = -1;

    ret = sound_device_init_near(vol, chans);
    if(ret == -1)
    {
        AEC_PRINT("sound_device_init_near failed \n");
        return -1;
    }

    return 0;
}

void sound_device_release(void)
{
    if(dmic_device) {
	pcm_close(dmic_pcm);
	dmic_pcm=NULL;
	free(dmic_device);
	dmic_device = NULL;
    }

}

int sound_aec_enable(void)
{
	return  0;
}

int sound_aec_disable(void)
{
	return 0;
}



char *RECORD_PATH = "record";
FILE *fp = NULL;
char RECORD_PATH_NAME[100] = {0};
char *pRecord = RECORD_PATH_NAME;



char dmic_record_buf[AEC_BUFFER_SIZE];
char amic_record_buf[AEC_BUFFER_SIZE];
int dmic_read_handle(echo_wakeup_t *ew)
{
	int ret = 0;

	while(aec_wakeup_flag != AEC_WAKEUP) {
		ret=pcm_read(dmic_pcm,dmic_record_buf,AEC_BUFFER_SIZE);
		if(ret<0)
		    printf("failed to pcm_read ! %s\n",pcm_get_error(dmic_pcm));

		memset(amic_record_buf, 0, AEC_BUFFER_SIZE);
		echo_wakeup_process(ew, dmic_record_buf, amic_record_buf, AEC_BUFFER_SIZE);

	}

	return 0;
EXIT:
	return -1;

}


/**
* @brief read dmic data to bufp with len bytes.
*
* @param bufp
* @param len
*
* @return bytes readed.
*/
int snd_read(enum snd_dev_type type, void *bufp, unsigned int len)
{
	int fd = -1;
	int ret;
	int count = len;
	size_t f, rest;
	int bits_per_frame;
	ret=pcm_read(dmic_pcm,bufp,count);
	if(ret < 0) {
	    printf("failed to pcm_read ! %s\n",pcm_get_error(dmic_pcm));
	    return -1;
	}
	count -= count;
	return len - count;
}

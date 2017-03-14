#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include "echo_wakeup.h"
#include "sound_dev.h"

#include <alsa/asoundlib.h>


#define DMIC_DEVSTR	"hw:0,2"
#define AMIC_DEVSTR	"hw:0,0"
#define MAX_BUFFER_TIME 500000  /*  50 us */
#define SOFT_RESAMPLE   0       /*  whether enable soft resample or not */

//#define USE_AMIC_ECHO	 not ok!

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


alsa_device_t *dmic_device = NULL;
alsa_device_t *amic_device = NULL;



/* Extern Variables!! */
extern int aec_wakeup_flag;
extern int error_flag;


static int alsa_init_dev(alsa_device_t *alsa_device, snd_pcm_format_t format,
		snd_pcm_access_t access, unsigned int channels,
		unsigned int rate, int soft_resample, int latency)
{
	int err;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *swparams;

	snd_pcm_uframes_t buffer_size;
	snd_pcm_uframes_t period_size;

	char *devstr = alsa_device->devstr;

	printf("alsa init device: %s, channels: %d, rate: %d, latency: %d\n", alsa_device->devstr, channels, rate, latency);

	/*  open pcm */
	if ((err = snd_pcm_open(&alsa_device->handle, devstr, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n",
				devstr,
				snd_strerror (err));
		return err;
	}

	/* set params */
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
				snd_strerror (err));
	}

	if ((err = snd_pcm_hw_params_any(alsa_device->handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
				snd_strerror (err));
	}

	if ((err = snd_pcm_hw_params_set_access(alsa_device->handle, hw_params, access)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
				snd_strerror (err));
	}

	if ((err = snd_pcm_hw_params_set_rate_near(alsa_device->handle, hw_params, &rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
				snd_strerror (err));
	}

	if ((err = snd_pcm_hw_params_set_channels(alsa_device->handle, hw_params, channels)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
				snd_strerror (err));
	}

	unsigned int buffer_time = 0;
	if((err = snd_pcm_hw_params_get_buffer_time_max(hw_params, &buffer_time, 0)) < 0) {
		fprintf(stderr, "cannot get buffer time max (%s)\n",
				snd_strerror(err));
	}
	if(buffer_time > latency) {
		buffer_time = latency;
	}

	unsigned int period_time = 0;
	period_time = buffer_time / 4;

	err = snd_pcm_hw_params_set_period_time_near(alsa_device->handle, hw_params, &period_time, 0);
	if(err < 0) {
		fprintf(stderr, "cannot set period time near (%s)\n",
				snd_strerror(err));
	}

	err = snd_pcm_hw_params_set_buffer_time_near(alsa_device->handle, hw_params, &buffer_time, 0);
	if(err < 0) {
		fprintf(stderr, "cannot set buffer time near (%s)\n",
				snd_strerror(err));
	}


	if ((err = snd_pcm_hw_params(alsa_device->handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
				snd_strerror (err));
	}

	if((err = snd_pcm_hw_params_get_period_size(hw_params, &period_size, 0)) < 0) {
		fprintf(stderr, "cannot get period size (%s)\n",
				snd_strerror(err));
	}

	snd_pcm_hw_params_free (hw_params);

	if ((err = snd_pcm_prepare(alsa_device->handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
				snd_strerror (err));
	}


	alsa_device->period_size = period_size;
	alsa_device->bits_per_sample = snd_pcm_format_physical_width(format);
	alsa_device->bits_per_frame = alsa_device->bits_per_sample * channels;
	alsa_device->period_bytes = alsa_device->period_size * alsa_device->bits_per_frame / 8;

	alsa_device->channels = channels;
	alsa_device->fmt = format;
	alsa_device->rate = rate;

	return err;
}



int sound_device_init_near(int val, int chans)
{
	/* main dmic */
	int ret = -1;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
	snd_pcm_access_t access = SND_PCM_ACCESS_RW_INTERLEAVED;
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

#ifdef USE_AMIC_ECHO
int sound_device_init_far(void)
{
	/* amic */
	int ret = -1;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
	snd_pcm_access_t access = SND_PCM_ACCESS_RW_INTERLEAVED;
	unsigned int channels = 2;
	unsigned int rate = RATE;

	int soft_resample = SOFT_RESAMPLE;      /*  disable soft resample */
	unsigned int latency = MAX_BUFFER_TIME; /*  latency in us */

	amic_device = malloc(sizeof(alsa_device_t));
	if(!amic_device) {
		printf("alloc amic_device error!\n");
		return -1;
	}

	memset(amic_device->devstr, 0, sizeof(amic_device->devstr));
	memcpy(amic_device->devstr, AMIC_DEVSTR, strlen(AMIC_DEVSTR));

	memset(amic_device->devstr, 0, sizeof(amic_device->devstr));
	sprintf(amic_device->devstr, "hw:0,0");
	ret = alsa_init_dev(amic_device, format, access, channels, rate, soft_resample, latency);
	if(ret < 0) {
		printf("init alsa device failed!\n");
		return ret;
	}
	return  0;
}
#endif

int sound_device_init(int vol, int chans)
{
    int ret = -1;

    ret = sound_device_init_near(vol, chans);
    if(ret == -1)
    {
        AEC_PRINT("sound_device_init_near failed \n");
        return -1;
    }

#ifdef USE_AMIC_ECHO
    ret = sound_device_init_far();
    if(ret == -1)
    {
        AEC_PRINT("sound_device_init_far failed \n");
        return -1;
    }
#endif

    return 0;
}

void sound_device_release(void)
{
    if(dmic_device) {
	snd_pcm_close(dmic_device->handle);
	free(dmic_device);
	dmic_device = NULL;
    }
#ifdef USE_AMIC_ECHO
    if(amic_device) {
	snd_pcm_close(amic_device->handle);
	free(amic_device);
	amic_device = NULL;
    }
#endif
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
		size_t f = AEC_BUFFER_SIZE * 8 / dmic_device->bits_per_frame;

		ret = snd_pcm_readi(dmic_device->handle, dmic_record_buf, f);

		if(ret < 0) {
			if((ret = snd_pcm_prepare(dmic_device->handle)) < 0) {
				error_flag = TRUE;
				printf("failed to prepare dmic handle! %s\n", snd_strerror(ret));
				goto EXIT;
			}
			printf("pcm read error!\n");
		}

#ifdef USE_AMIC_ECHO
		ret = snd_pcm_readi(amic_device->handle, amic_record_buf, f);
		if(ret < 0) {
			if((ret = snd_pcm_prepare(amic_device->handle)) < 0) {
				error_flag = TRUE;
				printf("failed to prepare dmic handle! %s\n", snd_strerror(ret));
				goto EXIT;
			}
			printf("pcm read error!\n");
		}
#endif

#if 0
		if(fp == NULL) {
			fp = fopen("/tmp/xbb.wav", "w+");
			if(fp == NULL) {
				perror("open record file!\n");
				error_flag = TRUE;
			}
		}

		if(fwrite(x_tmp_buf, ret * dmic_device->bits_per_frame / 8, 1, fp) != 1){
			printf("error: fwrite !\n");
			error_flag = TRUE;
			goto EXIT;
		}
#endif
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
	snd_pcm_t *capture_handle;
	int fd = -1;
	int ret;
	int count = len;
	size_t f, rest;
	int bits_per_frame;

	switch(type) {
		case SND_DEV_DMIC:
			capture_handle = dmic_device->handle;
			f = len * 8 / dmic_device->bits_per_frame;
			bits_per_frame = dmic_device->bits_per_frame;
			break;
		case SND_DEV_AMIC:
			capture_handle = amic_device->handle;
			f = len * 8 / amic_device->bits_per_frame;
			bits_per_frame = amic_device->bits_per_frame;
			break;
		default:
			printf("unsupported device type!\n");
			break;
	}

	rest = f;
	while(count > 0) {
		ret = snd_pcm_readi(capture_handle, bufp, rest);
		if(ret < 0) {
			if(snd_pcm_prepare(capture_handle) < 0) {
				printf("failed to prepare capture handle!\n");
				return -1;
			}
			continue;
		}

		count -= ret * bits_per_frame / 8;
		bufp += ret * bits_per_frame / 8;
		rest -= ret;

	}
	return len - count;
}

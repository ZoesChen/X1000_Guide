#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/soundcard.h>
#include <sys/stat.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>

#include <hardware/SampleOnTime.h>
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include "circ_buf.h"
#include "SampleOnTime.h"




static void enable_sample_on_time()
{
	system("echo 1 > /proc/jz/clock/aec_enable/enable");
}

static void disable_sample_on_time()
{
	system("echo 0 > /proc/jz/clock/aec_enable/enable");
}

static int alsa_init_dev(alsa_device_t *alsa_device, snd_pcm_format_t format,
		snd_pcm_access_t access, unsigned int channels,
		unsigned int rate, int soft_resample, int latency)
{
	int err;
	snd_pcm_hw_params_t *hw_params;

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

#ifdef DEBUG_RECORD_DATA
	printf("period_size=%d\n", alsa_device->period_size);
	printf("bits_per_sample=%d\n", alsa_device->bits_per_sample);
	printf("bits_per_frame=%d\n", alsa_device->bits_per_frame);
	printf("period_bytes=%d\n", alsa_device->period_bytes);
	printf("channels=%d\n", alsa_device->channels);
	printf("fmt=%d\n", alsa_device->fmt);
	printf("rate=%d\n", alsa_device->rate);
#endif

	return err;
}

static int alsa_release_dev(alsa_device_t *alsa_device)
{
	snd_pcm_close(alsa_device->handle);
	alsa_device->handle = NULL;
	return 0;
}

static int record_init(record_device_t *record_device)
{
	int ret = -1;
	snd_pcm_access_t access = SND_PCM_ACCESS_RW_INTERLEAVED;
	int soft_resample = SOFT_RESAMPLE;      /*  disable soft resample */

	unsigned int latency = MAX_BUFFER_TIME; /*  latency in us */

	ret = alsa_init_dev(record_device->alsa_device, record_device->alsa_device->fmt, access, record_device->alsa_device->channels, record_device->alsa_device->rate, soft_resample, latency);
	if(ret < 0) {
		printf("init alsa device faild!\n");
		free(record_device->alsa_device);
		record_device->alsa_device = NULL;
		return ret;
	}

	return 0;
}


static int record_deinit(record_device_t *record_device)
{
	alsa_release_dev(record_device->alsa_device);
	return 0;
}


static int alsa_record(alsa_device_t *snd_device, char *snd_buf)
{
	int ret = 0;
	size_t f = RECORD_BUFSZ * 8 / snd_device->bits_per_frame;

	ret = snd_pcm_readi(snd_device->handle, snd_buf, f);

	if(ret < 0) {
		if((ret = snd_pcm_prepare(snd_device->handle)) < 0) {
			printf("failed to prepare snd handle! %s\n", snd_strerror(ret));
		}
		printf("record read error!\n");
	}
	return ret*4;
}


void *recording(void *arg)
{
	int len = 0;
	int ret = 0;

	record_device_t *record_device;
	record_device = (record_device_t *)arg;

	while(1){
		if(record_device->exit_flag)
			pthread_exit(NULL);

		len = alsa_record(record_device->alsa_device, (char *)record_device->buf);

		ret = circbuf_block_write(record_device->circbuf, (char *)record_device->buf, len);
		if(ret != len) {
			printf("write %d bytes to circbuf error!\n", len);
		}

	}

}


static int sample_on_time_read(record_device_t *record_device, char *buf, int len)
{
	int ret = 0;


		ret = circbuf_block_read(record_device->circbuf, buf, len);
		if(ret != len) {
			printf("error circbuf_block_read amic_circbuf\n");
		}

	return ret;
}



static int device_init(struct sample_on_time_device_t *dev)
{
	int i = -1;
	SampleOnTime_t *mSampleOnTime;
	mSampleOnTime = malloc(sizeof(SampleOnTime_t));
	if(mSampleOnTime == NULL){
		printf("malloc mSampleOnTime failed\n");
		return -1;
	}

	mSampleOnTime->nr_devs = MAX_SUPPORT_DEVICES;
	for(i=0; i<mSampleOnTime->nr_devs; i++){
		*mSampleOnTime->devices = NULL;
	}

	dev->priv = mSampleOnTime;

	pthread_mutex_init(&mSampleOnTime->lock, NULL);

	enable_sample_on_time();

	return 0;

}

static int new_device(struct sample_on_time_device_t* dev, char *devstr, unsigned int rate, snd_pcm_format_t fmt, unsigned int chan)
{

	SampleOnTime_t *mSampleOnTime;
	record_device_t *rdevice;
	alsa_device_t *alsa_device;
	int id = -1;
	int i = -1;

	mSampleOnTime = dev->priv;

	pthread_mutex_lock(&mSampleOnTime->lock);

	for(i=0; i<mSampleOnTime->nr_devs; i++) {
		if(mSampleOnTime->devices[i] == NULL) {
			id = i;
			break;
		}
	}
	if(id < 0){
		printf("no space for new device\n");
		goto err;
	}

	rdevice = malloc(sizeof(record_device_t));
	if(rdevice == NULL) {
		printf("alloc record device error!\n");
		goto err;
	}

	alsa_device = malloc(sizeof(alsa_device_t));
	if(alsa_device == NULL) {
		printf("alloc alsa_device error!\n");
		goto err;
	}
	memset(alsa_device->devstr, 0, sizeof(alsa_device->devstr));
	memcpy(alsa_device->devstr, devstr, strlen(devstr));
	alsa_device->rate = rate;
	alsa_device->channels = chan;
	alsa_device->fmt = fmt;

	rdevice->alsa_device = alsa_device;

	rdevice->circbuf = malloc(sizeof(struct circbuf));
	rdevice->circbuf->buffer = malloc(MAX_RING_BUFFER_SIZE);
	if(rdevice->circbuf->buffer == NULL) {
		printf(" failed to malloc\n");
		goto err;
	}
	circbuf_init(rdevice->circbuf, rdevice->circbuf->buffer, MAX_RING_BUFFER_SIZE, devstr);

	mSampleOnTime->devices[id] = rdevice;



	pthread_mutex_unlock(&mSampleOnTime->lock);
	return id;

err:

	pthread_mutex_unlock(&mSampleOnTime->lock);
	return -1;
}

static int device_start(struct sample_on_time_device_t* dev, int id)
{
	SampleOnTime_t *mSampleOnTime;
	record_device_t *rdevice;
	int ret = 0;

	mSampleOnTime = dev->priv;

	pthread_mutex_lock(&mSampleOnTime->lock);

	rdevice = mSampleOnTime->devices[id];
	rdevice->exit_flag = 0;
	record_init(rdevice);
	pthread_create(&rdevice->pid, NULL, recording, (void *)rdevice);

	pthread_mutex_unlock(&mSampleOnTime->lock);
	return ret;
}

static int device_block_read(struct sample_on_time_device_t* dev, int id, char *buf, int len)
{
	SampleOnTime_t *mSampleOnTime;
	record_device_t *rdevice;
	int ret = 0;

	mSampleOnTime = dev->priv;

	pthread_mutex_lock(&mSampleOnTime->lock);

	rdevice = mSampleOnTime->devices[id];
	ret = sample_on_time_read(rdevice, buf, len);

	pthread_mutex_unlock(&mSampleOnTime->lock);
	return ret;
}

static int device_get_buffer(struct sample_on_time_device_t* dev, int id, char **buf, int *len)
{
	return -1;
}

static int device_put_buffer(struct sample_on_time_device_t* dev, int id, char **buf, int *len)
{
	return -1;
}

static int device_stop(struct sample_on_time_device_t* dev, int id)
{

	SampleOnTime_t *mSampleOnTime;
	record_device_t *rdevice;

	mSampleOnTime = dev->priv;

	pthread_mutex_lock(&mSampleOnTime->lock);

	rdevice = mSampleOnTime->devices[id];
	rdevice->exit_flag = 1;
	pthread_join(rdevice->pid, NULL);
	record_deinit(rdevice);

	pthread_mutex_unlock(&mSampleOnTime->lock);

	return 0;
}

static int del_device(struct sample_on_time_device_t* dev, int id)
{
	SampleOnTime_t *mSampleOnTime;
	record_device_t *rdevice;
	alsa_device_t *alsa_device;

	mSampleOnTime = dev->priv;
	pthread_mutex_lock(&mSampleOnTime->lock);
	rdevice = mSampleOnTime->devices[id];
	alsa_device = rdevice->alsa_device;

	free(alsa_device);
	free(rdevice->circbuf);
	free(rdevice);
	mSampleOnTime->devices[id] = NULL;
	pthread_mutex_unlock(&mSampleOnTime->lock);
	return 0;
}

static int device_deinit(struct sample_on_time_device_t *dev)
{
	SampleOnTime_t *mSampleOnTime;
	mSampleOnTime = dev->priv;

	pthread_mutex_destroy(&mSampleOnTime->lock);
	free(mSampleOnTime);
	disable_sample_on_time();
	return 0;
}

static int device_close(struct sample_on_time_device_t *dev)
{
	device_deinit(dev);
	free(dev);
	return 0;
}
/** Open a new instance of a lights device using name */
static int open_device(const struct hw_module_t* module, char const* name,
		        struct hw_device_t** device)
{

	struct sample_on_time_device_t *dev = malloc(sizeof(struct sample_on_time_device_t));
	memset(dev, 0, sizeof(*dev));

	dev->common.tag = HARDWARE_DEVICE_TAG;
	dev->common.version = 0;
	dev->common.module = (struct hw_module_t*)module;
	dev->common.close = (int (*)(struct hw_device_t*))device_close;
	dev->new_device = new_device;
	dev->device_start = device_start;
	dev->device_block_read = device_block_read;
	dev->device_get_buffer = device_get_buffer;
	dev->device_put_buffer = device_put_buffer;
	dev->device_stop = device_stop;
	dev->del_device = del_device;

	device_init(dev);
	*device = (struct hw_device_t*)dev;
	return 0;

}
static struct hw_module_methods_t sample_on_time_module_methods = {
	.open =  open_device,
};

/*
 *   * The sample_on_time Module
 *     */
struct hw_module_t HAL_MODULE_INFO_SYM = {
	.tag = HARDWARE_MODULE_TAG,
	.version_major = 1,
	.version_minor = 0,
	.id = SAMPLE_ON_TIME_HARDWARE_MODULE_ID,
	.name = "Ingenic Apus Sample On Time Module",
	.author = "Ingenic.",
	.methods = &sample_on_time_module_methods,
};







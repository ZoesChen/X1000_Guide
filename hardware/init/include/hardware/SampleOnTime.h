#ifndef MANHATTAN_SAMPLE_ON_TIMEINTERFACE_H
#define MANHATTAN_SAMPLE_ON_TIMEINTERFACE_H


#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <hardware/hardware.h>


__BEGIN_DECLS


/**
 * The id of this module
 */
#define SAMPLE_ON_TIME_HARDWARE_MODULE_ID "SampleOnTime"


/**
 * @brief sample_on_time_device_t is used for audio sample at the same time,
 *		 can be used for some modules, for example aec.
 *		 You can use as @link hardware/tests/SampleOnTime/SampleOnTimeTest.c
 */
struct sample_on_time_device_t {
	struct hw_device_t common;

	void *priv;


	/**
	 * @brief create a new device
	 *
	 * @param
	 * @param devstr alsa device name
	 * @param rate alse device rate
	 * @param fmt alsa device format
	 * @param chan alsa device channel
	 * @return device id for succes, negative for err
	 */
	int (*new_device)(struct sample_on_time_device_t* dev, char *devstr, unsigned int rate, snd_pcm_format_t fmt, unsigned int chan);

	/**
	 * @brief start record device
	 *
	 * @param
	 * @param id	device id created by new_device
	 * @return 0 for succes, negative for error
	 */
	int (*device_start)(struct sample_on_time_device_t* dev, int id);

	/**
	 * @brief block read from buffer
	 *
	 * @param
	 * @param id device id created by new_device
	 * @param buf buf to save data
	 * @param len read data length
	 * @return length
	 */
	int (*device_block_read)(struct sample_on_time_device_t* dev, int id, char *buf, int len);

	/**
	 * @brief get buffer pointer
	 *
	 * @param
	 * @param id device id
	 * @param buf buffer point
	 * @param len read data length
	 */
	int (*device_get_buffer)(struct sample_on_time_device_t* dev, int id, char **buf, int *len);

	/**
	 * @brief put buffer pointer
	 *
	 * @param
	 * @param id device id
	 * @param buf buffer point
	 * @param len read data length
	 */
	int (*device_put_buffer)(struct sample_on_time_device_t* dev, int id, char **buf, int *len);

	/**
	 * @brief stop record device
	 *
	 * @param
	 * @param id device id created by new_device
	 * @return 0 for succes, negative for error
	 */
	int (*device_stop)(struct sample_on_time_device_t* dev, int id);

	/**
	 * @brief delete a device
	 *
	 * @param
	 * @param id device id created by new_device
	 * @return 0 for succes, negative for error
	 */
	int (*del_device)(struct sample_on_time_device_t* dev, int id);


};


__END_DECLS


#endif

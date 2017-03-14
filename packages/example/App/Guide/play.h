#ifndef _GUIDE_PLAY_H_
#define _GUIDE_PLAY_H_
#include <stdint.h>
#include <pthread.h>

/*
 * Define
 * */
#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define MUSIC_NUM1 1111
#define MUSIC_NUM2 2222
 
/*
 * Struct
 * */
struct riff_wave_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_id;
};

struct chunk_header {
    uint32_t id;
    uint32_t sz;
};

struct chunk_fmt {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

enum playStatus {
	ENABLEPLAY,
	DISABLEPLAY
};

/*
 * Parameter
 * */
pthread_mutex_t musicLock;
pthread_cond_t musicCond;

enum playStatus playFlag;

/*
 * Function
 * */
int StartPlay(int musicNum);
void InitPlay();
void StopPlay();
#endif

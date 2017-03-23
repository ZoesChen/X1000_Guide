#include <tinyalsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#include "play.h"
#define DEBUG
/*
 * Parameter
 * */
static FILE *file;
struct riff_wave_header riffWaveHeader;
struct chunk_header chunkHeader;
struct chunk_fmt chunkFmt;
static unsigned int device;
static unsigned int card;
static unsigned int period_size;
static unsigned int period_count;
static unsigned int channels;
static unsigned int rate;
static unsigned int bits;
static unsigned int is_raw; /* Default wav file */
static int more_chunks;
static int closeFlag;
static unsigned long int musicNumber = 0;
static CMDTYPE cmdType = INVAILD_CMD;
static pthread_t musicThread;

static char base_path[] = {"/mnt/sd/CHIGOORES/"};
static enum LANGUAGE language = CHINESE;

extern int isKeyMusicPlaying;

//recoder resource file name list
struct LocationInfoArr {
	int id;
	int angle;
} locationInfoArray[256];

static char musicFile[256][64] = {0};
//recoder music file number
static unsigned int fileNumber = 0;
/*
 * Function
 * */
void InitPlay();
void StopPlay();
char *matchMusic(unsigned long int musicNum);

void play_sample(FILE *file, unsigned int card, unsigned int device, unsigned int channels,
                 unsigned int rate, unsigned int bits, unsigned int period_size,
                 unsigned int period_count);
                 
int sample_is_playable(unsigned int card, unsigned int device, unsigned int channels,
                        unsigned int rate, unsigned int bits, unsigned int period_size,
                        unsigned int period_count);
                        
static int check_param(struct pcm_params *params, unsigned int param, unsigned int value,
                 char *param_name, char *param_unit);

void playInterface(CMDTYPE cmd, unsigned long int mNum)
{
	if (playFlag == ENABLEPLAY) {
		printf("%s: Now is playing\n", __FUNCTION__);
		pthread_mutex_lock(&musicLock);
		printf("%s: get music lock\n", __FUNCTION__);
		playFlag = DISABLEPLAY;
		pthread_cond_wait(&musicCond, &musicLock);
		printf("%s: Be wake up\n", __FUNCTION__);
		pthread_mutex_unlock(&musicLock);
	}
	/*Add by Zoe: sleep 10ms to make sure music thread wait music Cond*/
	usleep(10000);
	if (playFlag == DISABLEPLAY) {
		printf("%s: Now is stoping\n", __FUNCTION__);
		pthread_mutex_lock(&musicLock);
		musicNumber = mNum;
		cmdType = cmd;
		playFlag = ENABLEPLAY;
		pthread_cond_signal(&musicCond);
		pthread_mutex_unlock(&musicLock);
	}
}


void *MusicThreadHandle(void *arg) 
{
	while(1) {
		pthread_mutex_lock(&musicLock);
		pthread_cond_wait(&musicCond, &musicLock);
		pthread_mutex_unlock(&musicLock);
		
		while(playFlag == ENABLEPLAY) {
			char *musicName = NULL;
			musicName = matchMusic(musicNumber);
			if (musicName == NULL) {
				printf("Can not match music!\n");
				if (isKeyMusicPlaying == 1)
					isKeyMusicPlaying = 0;
				return NULL;
			}

			file = fopen(musicName, "rb");
			if (file == NULL) {
				printf("Open %s fail\n", musicName);
				playFlag = DISABLEPLAY;
				free(musicName);
				continue;
			} else {
				free(musicName);
			}

			if (!is_raw) {
				fread(&riffWaveHeader, sizeof(struct riff_wave_header), 1, file);
				if ((riffWaveHeader.riff_id != ID_RIFF) || (riffWaveHeader.wave_id != ID_WAVE)) {
					fprintf(stderr, "Error: '%s' is not a riff/wave file\n", musicName);
					fclose(file);
					playFlag = DISABLEPLAY;
					continue;

				}
				do {
					fread(&chunkHeader, sizeof(struct chunk_header), 1, file);
					switch (chunkHeader.id) {
				            case ID_FMT:
				                fread(&chunkFmt, sizeof(struct chunk_fmt), 1, file);
				                printf("%s: chunkHeader.sz = %d\n", __FUNCTION__, chunkHeader.sz);
				                /* If the format header is larger, skip the rest */
				                if (chunkHeader.sz > sizeof(struct chunk_fmt))
				                    fseek(file, chunkHeader.sz - sizeof(struct chunk_fmt), SEEK_CUR);
				                break;
				            case ID_DATA:
				                /* Stop looking for chunks */
				                more_chunks = 0;
				                break;
				            default:
				            /* Unknown chunk, skip bytes */
				            fseek(file, chunkHeader.sz, SEEK_CUR);
					}
				} while (more_chunks);
				
				channels = chunkFmt.num_channels;
				rate = chunkFmt.sample_rate;
				bits = chunkFmt.bits_per_sample;
			}

			play_sample(file, card, device, channels, rate, bits, period_size, period_count);
			fclose(file);
			playFlag = DISABLEPLAY;
			if (isKeyMusicPlaying == 1) {
				printf("%s: key music is play finish\n", __FUNCTION__);
				isKeyMusicPlaying = 0;
			}
		}
		pthread_mutex_lock(&musicLock);
		pthread_cond_signal(&musicCond);
		pthread_mutex_unlock(&musicLock);
	}
	return NULL;
}

static void scanResource()
{
	unsigned int i;
	for(i = 0; i < fileNumber; i++) {
		printf("%s\n", musicFile[i]);
		printf("%d,%d\n", locationInfoArray[i].id, locationInfoArray[i].angle);
	}
}

static void readResource()
{
	DIR *dir;
	struct dirent *ptr;
	char basePath[256] = {0};
	int index = 0;
	sprintf(basePath, "%sCN", base_path);
	printf("open %s\n", basePath);
	
	if ((dir = opendir(basePath)) == NULL) {
		printf("Open dir error...\n");
		return;
	}

	while((ptr = readdir(dir)) != NULL) {
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name,"..") == 0)
			continue;
		if (ptr->d_type == 8) { // file
			//printf("%s\n", ptr->d_name);
			int nameIndex = 3; //the first 3 bit is ''I' 'd' '_'
			int i = 0;
			int num = 0; //get the num of number
			char buf[64] = {0};
			int isDigitFlag = 0;
			while(ptr->d_name[nameIndex] != 0) {
				if (isdigit(ptr->d_name[nameIndex])) {
					buf[i] = ptr->d_name[nameIndex];
					isDigitFlag = 1;
				} else {
					buf[i] = ' ';
					if (isDigitFlag == 1) {
						num++;
						isDigitFlag = 0;
					}
				}
				++i;
				++nameIndex;
			}
			strcpy(musicFile[index], ptr->d_name);
			switch(num) {
				case 1:
					sscanf(buf, "%d ", &(locationInfoArray[index].id));
				break;
				case 2:
					sscanf(buf, "%d %d ", &(locationInfoArray[index].id), &(locationInfoArray[index].angle));
				break;
				default:
				break;
			}
			index++;
		}
	}
	fileNumber = index;
	scanResource();
	closedir(dir);
}

void InitPlay()
{
	device = 0;
	card = 0;
	period_size = 1024;
	period_count = 4;
	channels = 2;
	rate = 48000;
	bits = 16;
	is_raw = 0;
	more_chunks = 1;
	closeFlag = 0;
	playFlag = DISABLEPLAY;
	readResource();
	pthread_create(&musicThread, NULL, MusicThreadHandle, NULL);
}

static int findMinDiffAngle(int *angleDiffArray, int index)
{
	int min = angleDiffArray[0];
	int minIndex = 0;
	int i;
	for (i = 0; i < index; i++) {
		if(angleDiffArray[i] < min) {
			min = angleDiffArray[i];
			minIndex = i;
		}
	}
	return minIndex;
}

static int findLocationFile(int id, int usrAngle)
{
	int angle = 0;
	int index = 0;
	int existNum = 0;
	struct LocationInfoArr info[4] = {0};
	int angleAbsDiff[4] = {0};
	//Find out in this eara, how many audio resource exist,  
	//and save it info into info 
	for (index = 0; index < fileNumber; index++) {
		if (id == locationInfoArray[index].id) {
			info[existNum].id = locationInfoArray[index].id;
			info[existNum].angle = locationInfoArray[index].angle;
			angleAbsDiff[existNum] = abs(usrAngle - locationInfoArray[index].angle);
			existNum++;
		}
	}

	printf("%s: In this eara, have %d audio resource\n", __FUNCTION__, existNum);

	if (existNum == 1) {
		angle = info[0].angle;
	} else if (existNum > 1) {
		angle = info[findMinDiffAngle(angleAbsDiff, existNum)].angle;
	} else {
		angle = -1;
	}
	printf("%s: the real angle is %d\n", __FUNCTION__, angle);
	return angle;
}

char *matchMusic(unsigned long int musicNum)
{
	char *name = (char *)malloc(sizeof(char) * 128);
	char languagePath[10] = {0};
	int id = 0;
	int usrAngle = 0;
	int locationAngle = 0;
	
	if (language == CHINESE) {
		strcpy(languagePath,"CN/");
	} else if (language == ENGLISH) {
		strcpy(languagePath, "EN/");
	}

	if (cmdType == MUSIC_CMD) {

		sprintf(name,"%s%sId_%ld.wav", base_path, languagePath, musicNum);
	} else if (cmdType == NUMBER_CMD) {
		sprintf(name, "%sSYSTEM/%ld.wav", base_path, musicNum);
	} else if (cmdType == LOCATION_MUSIC_CMD) {
		printf("%s: musicNum is %ld\n", __FUNCTION__, musicNum);
		id = musicNum / 10000;
		usrAngle = musicNum - id * 10000;
		if (usrAngle != 0) {
			locationAngle = findLocationFile(id, usrAngle);
		} else {
			locationAngle = usrAngle;
		}
		if (locationAngle == 0) {
			//this point have no resouce with angle info
			sprintf(name, "%s%sId_%d.wav", base_path, languagePath,id);
		} else if (locationAngle > 0){
			sprintf(name, "%s%sId_%d_%d.wav", base_path, languagePath, id, locationAngle);
		} else {
			return NULL;
		}
	}
	printf("%s: the music name is %s\n", __FUNCTION__, name);

	return name;
}

void play_sample(FILE *file, unsigned int card, unsigned int device, unsigned int channels,
                 unsigned int rate, unsigned int bits, unsigned int period_size,
                 unsigned int period_count)
{
    struct pcm_config config;
    struct pcm *pcm;
    char *buffer;
    int size;
    int num_read;

    memset(&config, 0, sizeof(config));
    config.channels = channels;
    config.rate = rate;
    config.period_size = period_size;
    config.period_count = period_count;
    if (bits == 32)
        config.format = PCM_FORMAT_S32_LE;
    else if (bits == 16)
        config.format = PCM_FORMAT_S16_LE;
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    if (!sample_is_playable(card, device, channels, rate, bits, period_size, period_count)) {
        return;
    }

    pcm = pcm_open(card, device, PCM_OUT, &config);
    
    if (!pcm || !pcm_is_ready(pcm)) {
        fprintf(stderr, "Unable to open PCM device %u (%s)\n",
                device, pcm_get_error(pcm));
        return;
    }

    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Unable to allocate %d bytes\n", size);
        free(buffer);
        pcm_close(pcm);
        return;
    }

    printf("Playing sample: %u ch, %u hz, %u bit\n", channels, rate, bits);

    do {
        num_read = fread(buffer, 1, size, file);
        if (num_read > 0) {
	    if (pcm_write(pcm, buffer, num_read)) {
                fprintf(stderr, "Error playing sample\n");
                break;
            }
        }else if(num_read == 0) {
	    memset(buffer, 0, size);
	    if (pcm_write(pcm, buffer, size)) {
                fprintf(stderr, "Error playing sample\n");
                break;
            }
	}
    } while ((playFlag == ENABLEPLAY) && num_read > 0);
    playFlag = DISABLEPLAY;
    free(buffer);
    pcm_close(pcm);
}

int sample_is_playable(unsigned int card, unsigned int device, unsigned int channels,
                        unsigned int rate, unsigned int bits, unsigned int period_size,
                        unsigned int period_count)
{
    struct pcm_params *params;
    int can_play;

    params = pcm_params_get(card, device, PCM_OUT);
    if (params == NULL) {
        fprintf(stderr, "Unable to open PCM device %u.\n", device);
        return 0;
    }

    can_play = check_param(params, PCM_PARAM_RATE, rate, "Sample rate", "Hz");
    can_play &= check_param(params, PCM_PARAM_CHANNELS, channels, "Sample", " channels");
    can_play &= check_param(params, PCM_PARAM_SAMPLE_BITS, bits, "Bitrate", " bits");
    can_play &= check_param(params, PCM_PARAM_PERIOD_SIZE, period_size, "Period size", "Hz");
    can_play &= check_param(params, PCM_PARAM_PERIODS, period_count, "Period count", "Hz");

    pcm_params_free(params);

    return can_play;
}

static int check_param(struct pcm_params *params, unsigned int param, unsigned int value,
                 char *param_name, char *param_unit)
{
    unsigned int min;
    unsigned int max;
    int is_within_bounds = 1;

    min = pcm_params_get_min(params, param);
    if (value < min) {
        fprintf(stderr, "%s is %u%s, device only supports >= %u%s\n", param_name, value,
                param_unit, min, param_unit);
        is_within_bounds = 0;
    }

    max = pcm_params_get_max(params, param);
    if (value > max) {
        fprintf(stderr, "%s is %u%s, device only supports <= %u%s\n", param_name, value,
                param_unit, max, param_unit);
        is_within_bounds = 0;
    }

    return is_within_bounds;
}

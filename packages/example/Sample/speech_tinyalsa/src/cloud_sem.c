#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "aiengine.h"
#include "ai_main.h"
#include "sound_dev.h"
#include "cJSON.h"

#ifdef USE_ULA
#include "ula_doa.h"
#endif


#define ULA_DBG_SAVE_WAV

enum ASR_STATUS
{
    ASR_SLEEP = 0,
    ASR_SUCCESS,
    ASR_FAIL
};

static int g_speak_end_flag = 0;
static int recog_result_flag = ASR_SLEEP;
extern char recog_buf[STR_BUFFER_SZ];
extern int fd_dsp_rd;

char *_semantic_param =
"\
{\
    \"coreProvideType\": \"cloud\",\
    \"vadEnable\": 1,\
    \"audio\": {\
        \"audioType\": \"wav\",\
        \"channel\": 1,\
        \"sampleBytes\": 2,\
        \"sampleRate\": 16000\
    },\
    \"app\": {\
        \"userId\": \"aispeech\"\
    },\
    \"request\": {\
        \"coreType\": \"cn.dlg.ita\",\
        \"res\": \"aihome\"\
    }\
}";
char semantic_param[4096];

#ifdef USE_ULA
int phisDisplay(DOUBLE phis)
{
    printf("\nTheta[%.4f] Degree[%.4f]\n", (float)phis, (float)(phis*180/PI));
}
#endif

int _semantic_callback(const void *usrdata, const char *id, int type,
                                        const void *message, int size)
{
    cJSON *out = cJSON_Parse((char*) message);
    if (!out)
    {
        return -1;
    }
    //vad status
    cJSON *vad_status = cJSON_GetObjectItem(out, "vad_status");
    if (vad_status)
    {
        if(vad_status->valueint == 2)
        {
            // vad_status  0 - start , 1 -speaking, 2 - end
            g_speak_end_flag = 1;
            printf("*****************vad end**********************\n");
        }
        goto OUT;
    }

    cJSON *result = cJSON_GetObjectItem(out, "result");
    if (result)
    {
        cJSON *input = cJSON_GetObjectItem(result, "input");
        if(input)
        {
            memset(recog_buf, 0, STR_BUFFER_SZ);
            sprintf(recog_buf, input->valuestring);
            recog_result_flag = ASR_SUCCESS;
        }
    }
    else
    {
        recog_result_flag = ASR_FAIL;
        memset(recog_buf, 0, STR_BUFFER_SZ);
    }

OUT:
    AIENGINE_PRINT("resp data: %.*s\n", size, (char *) message);
    if (out)
    {
        cJSON_Delete(out);
    }
    return 0;
}


void *timeout_detect(void *ptr)
{
    int loop = 0;
    while(!g_speak_end_flag)
    {
        usleep(10000);
        loop++;
        if(loop > 2000)
        {
            AIENGINE_PRINT("No record found, Time out\n");
            g_speak_end_flag = 1;
            recog_result_flag = ASR_FAIL;
            break;
        }
    }
    return NULL;
}


int AI_cloudSem(struct aiengine *agn)
{
    char uuid[64] = {0};
#ifdef USE_ULA
	char *pcInData = NULL;
	char *pcOutData = NULL;
    ula_doa_t *pstDoa = NULL;
    ula_beamform_t *pstBeamform = NULL;
    ula_beamform_private_t stPriData;
#else
    char buf[RECORD_BUFSZ] = {0};
#endif
    int loop = 0;
    int ret = -1;
    pthread_t timeout_detect_thread = 0;
#ifdef USE_ULA
#ifdef ULA_DBG_SAVE_WAV
    char name[128] = {0};
    memset(name, 0, 128);
    sprintf(name, "%d.pcm", time());
    FILE *fpRaw = fopen(name, "ab+");
    memset(name, 0, 128);
    sprintf(name, "%d_bf.pcm", time());
    FILE *fpBF = fopen(name, "ab+");
#endif
#endif
	memset(semantic_param, 0, sizeof(semantic_param));
	memcpy(semantic_param, _semantic_param, strlen(_semantic_param));

    recog_result_flag = ASR_SLEEP;
    g_speak_end_flag = 0;
#ifdef USE_ULA
    /* private structure init */
    memset(&stPriData, 0, sizeof(stPriData));
    stPriData.chans = CHANEL_4;    /* actual channels */
    stPriData.fs = RATE;
    stPriData.ulMax = 8192*4*2; /* Max input data length */
    stPriData.ulTotalOutLen = stPriData.ulMax/2;    /* Max output data length */
    stPriData.lEndFlag = 0;
    memset(stPriData.acConfig, '\0', sizeof(stPriData.acConfig));
    strcpy(stPriData.acConfig, "./bin/x1000-4mic/config.ini");

    /* Doa and beamform init */
    ret = ULA_init(&stPriData, &pstDoa, &pstBeamform);
	if(ULA_RTN_FAIL == ret)
	{
		printf("Doa_beamformer module init fail.");
		goto OUT;
	}
    pstDoa->pGetTheta_callback = phisDisplay;
#endif
    /* play welcome to indicate */
    if(aiengine_start(agn, semantic_param, uuid, _semantic_callback, NULL) != 0)
    {
        AIENGINE_PRINT("aiengine start sem failed!\n");
        goto ULA_RELEASE;
    }
#ifdef USE_ULA
    /* alloc input data buffer */
    pcInData = calloc(1, RECORD_BUFSZ*4);
    if(NULL == pcInData)
    {
        goto ULA_RELEASE;
    }

    /* alloc output data buffer */
	pcOutData = calloc(1, stPriData.ulTotalOutLen);
    if(NULL == pcOutData)
    {
        goto ULA_RELEASE;
    }
    ULA_reset(pstDoa, pstBeamform);
#endif
    if(pthread_create(&timeout_detect_thread, NULL, timeout_detect, NULL) != 0)
    {
        AIENGINE_PRINT("Can't create timeout_detect_thread\n");
    }
    pthread_detach(timeout_detect_thread);

#ifdef USE_ULA
    /* sound device init */
    ret = sound_device_init(VOLUME, CHANEL_4);
#else
	ret = sound_device_init(VOLUME, CHANEL_1);
#endif
    if(ret == -1)
    {
        AIENGINE_PRINT("\"Sound device init failed!\"\n");
        goto ULA_RELEASE;
    }

    ret = sound_aec_enable();
    if(ret != 0)
    {
        AIENGINE_PRINT("\"Sound device enable failed!\"\n");
        goto SOUND_DEV_DEINIT;
    }

    printf("Please Speak(比如：播放刘德华的忘情水) ...\n");

    while(!g_speak_end_flag)
    {
#ifdef USE_ULA
	ret = snd_read(SND_DEV_DMIC, pcInData, RECORD_BUFSZ * 4);
        if(ret < 0)
        {
            AIENGINE_PRINT("mozart_record failed\n");
            break;
        }
#ifdef ULA_DBG_SAVE_WAV
        fwrite(pcInData, ret, 1, fpRaw);
#endif

        ret = ULA_multi2monoPhisBF(pstDoa, pstBeamform, pcInData, ret, &stPriData, pcOutData);

#ifdef ULA_DBG_SAVE_WAV
        if(ret)
        {
            fwrite(pcOutData, ret, 1, fpBF);
        }
#endif
        ret = aiengine_feed(agn, pcOutData, ret);
        if (ret < 0)
        {
            AIENGINE_PRINT("engine feed failed.\n");
            break;
        }
#else
	ret = snd_read(SND_DEV_DMIC, buf, RECORD_BUFSZ);
        if(ret < 0)
        {
            AIENGINE_PRINT("mozart_record failed\n");
            break;
        }

        ret = aiengine_feed(agn, buf, ret);
        if (ret < 0)
        {
            AIENGINE_PRINT("engine feed failed.\n");
            break;
        }
#endif
    }

    if(aiengine_stop(agn) != 0)
    {
        AIENGINE_PRINT("aiengine stop failed!\n");
        goto SOUND_DEV_DISABLE;
    }
    printf("Waiting for the reuslt ...\n");

    loop = 0;
    while(recog_result_flag == ASR_SLEEP)
    {
        usleep(100000);
        if(loop>300)
        {
            AIENGINE_PRINT("No result found, Time out\n");
            recog_result_flag = ASR_FAIL;
            break;
        }
        loop++;
    }

    if(recog_result_flag != ASR_FAIL)
    {
        ret = 0;
    }

SOUND_DEV_DISABLE:
    sound_aec_disable();

SOUND_DEV_DEINIT:
    sound_device_release();

ULA_RELEASE:
#ifdef USE_ULA
    if(pcOutData)
    {
        free(pcOutData);
        pcOutData = NULL;
    }
    if(pcInData)
    {
        free(pcInData);
        pcInData = NULL;
    }

    ULA_beamformRelease(pstBeamform);
	ULA_doaRelease(pstDoa);
#endif
OUT:
#ifdef USE_ULA
#ifdef ULA_DBG_SAVE_WAV
    fclose(fpBF);
    fclose(fpRaw);
#endif
#endif
    return ret;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "aiengine.h"
#include "ai_main.h"
#include "echo_wakeup.h"

char recog_buf[STR_BUFFER_SZ] = {0};

static const char *version =
"X1000 Demo Code\n"\
"Ver : 1.0.2\n"\
"Date: 2015-11-27";

static const char *cfg =
"\
{\
    \"luaPath\": \"./bin/\",\
    \"appKey\": \"14327742440003c5\",\
    \"secretKey\": \"59db7351b3790ec75c776f6881b35d7e\",\
    \"provision\": \"./bin/prov-jz-2.7.1-mac.file-20201031\",\
    \"serialNumber\": \"/usr/data/serialNumber\",\
    \"prof\": {\
        \"enable\": 0,\
        \"output\": \"./a.log\"\
    },\
    \"vad\":{\
        \"enable\": 1,\
        \"res\": \"./bin/vad.aihome.0.3.1027.bin\",\
        \"speechLowSeek\": 60,\
        \"sampleRate\": 16000,\
        \"pauseTime\":800,\
        \"strip\": 1\
    },\
    \"cloud\": {\
        \"server\": \"ws://s-test.api.aispeech.com:10000\"\
    },\
    \"native\": {\
        \"cn.dnn\": {\
            \"resBinPath\": \"./bin/aihome.1030.bin\"\
        },\
        \"cn.echo\": {\
            \"frameLen\": 512,\
            \"filterLen\": 2048,\
            \"rate\": 16000\
        }\
    }\
}";

extern int AI_aec(echo_wakeup_t *ew);
extern int AI_cloudSem(struct aiengine *agn);
extern void AI_cloudSyn(struct aiengine *agn, char *SynTxt);



char buf[TMP_BUFFER_SZ] = {0};
int main(int argc, char **argv)
{
    int ret = -1;
    enum AIENGINE_STAT stat = AI_AEC_ST;
    char *pcBuf = NULL;
    cJSON *cjs = NULL;
    cJSON *result = NULL;
    echo_wakeup_t *ew = NULL;
    struct aiengine *agn = NULL;
//    char music[128] = {0};

#if 0
		system("killall -9 mplayer");
		system("usleep 10000");
    sprintf(music, "mplayer %s ","welcome.mp3");
#endif

    /* print version info */
    printf("%s\n", version);

    /* create echo wakeup engine */
    ew = echo_wakeup_new(cfg);
    if(NULL == ew)
    {
        printf("[%s : %s : %d] Create AEC error. \n", __FILE__, __func__, __LINE__);
        goto OUT;
    }

    /* create AIEngine */
    agn = (struct aiengine *)aiengine_new(cfg);
    if(NULL == agn)
    {
        printf("[%s : %s : %d] create aiengine error. \n", __FILE__, __func__, __LINE__);
        goto AEC_DEL;
    }

    /* auth process */
    aiengine_opt(agn, 10, buf, TMP_BUFFER_SZ);

    AIENGINE_PRINT("Get opt: %s\n", buf);
    cjs = cJSON_Parse(buf);
    if (cjs)
    {
        result = cJSON_GetObjectItem(cjs, "success");
        if (result)
        {
            ret = 0;
        }
        cJSON_Delete(cjs);
    }

    if(ret)
    {
        aiengine_opt(agn, 11, buf, TMP_BUFFER_SZ);
        AIENGINE_PRINT("%s\n", buf);
    }

    /* main loop */
    while(1)
    {
        switch(stat)
        {
            /* echo cancellation */
            case AI_AEC_ST:
            {
                if(AI_aec(ew) == 0);
                {
                    stat = AI_SEM_ST;
                }
                break;
            }
            /* cloud speech recognition */
            case AI_SEM_ST:
            {
                memset(recog_buf, 0, sizeof(recog_buf));

                if(AI_cloudSem(agn) == 0)
                {
                    printf("Result: %s\n", recog_buf);
                    stat = AI_SYNC_ST;
                }

                break;
            }
            /* speech synthesis */
            case AI_SYNC_ST:
            {
                AI_cloudSyn(agn, recog_buf);

                stat = AI_AEC_ST;
                break;
            }
        }
    }

    ret = 0;
AGN_DEL:
    aiengine_delete(agn);
AEC_DEL:
    echo_wakeup_delete(ew);
OUT:
    return ret;
}


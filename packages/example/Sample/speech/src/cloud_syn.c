#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "cJSON.h"
#include "aiengine.h"
#include "ai_main.h"

int g_play_end_flag = 0;

static const char *cloud_syn_param =
"\
{\
    \"coreProvideType\": \"cloud\",\
    \"audio\": {\
        \"audioType\": \"mp3\",\
        \"channel\": 1,\
        \"sampleBytes\": 2,\
        \"sampleRate\": 16000,\
        \"compress\":\"raw\" \
    },\
    \"request\": {\
        \"coreType\": \"cn.sent.syn\",\
        \"res\": \"syn_chnsnt_zhilingf\",\
        \"realBack\":1\
    }\
}";

#define cloud_sync_record "/usr/data/cloud_sync_record.mp3"
char *fn = cloud_sync_record;

int _cloud_syn_callback(const void *usrdata, const char *id, int type,
        const void *message, int size)
{
    if(type == 1)
    {
        printf(" Type is %d\n", type);
        g_play_end_flag = 1;
        return 0;
    }

    /* SYNC END*/
    if (size == 0)
    {
    		
        char sys_cmd[256]={0};
        g_play_end_flag = 1;
#if 0
				system("killall -9 mplayer");
				system("usleep 10000");
        sprintf(sys_cmd, "mplayer %s", fn); //need fix
        system(sys_cmd);
#endif
    }
    else 
    {
    		FILE *fout = fopen(fn, "a+");
        if(fout)
        {
            fwrite(message, size, 1, fout);
        		fclose(fout);
        }
    }

    return 0;
}

void AI_cloudSyn(struct aiengine *agn, char *SynTxt)
{
    char uuid[64] = {0};
    char *_param = NULL;
    cJSON *param_js = NULL;
    cJSON *request_js = NULL;
    cJSON *syn_js = NULL;
    
    g_play_end_flag = 0;
    if(!strcmp(SynTxt, ""))
    {
        return ;
    }
    
    param_js = cJSON_Parse(cloud_syn_param);
    if(param_js)
    {
        syn_js = cJSON_CreateString(SynTxt);
        request_js = cJSON_GetObjectItem(param_js, "request");
        cJSON_AddItemToObject(request_js, "refText", syn_js);
    }
    else
    {
        return ; 
    }
    

    _param = cJSON_Print(param_js);

    AIENGINE_PRINT("=======tts syn=========\n");
    AIENGINE_PRINT("Syn: %s \n", SynTxt);
    FILE *fout = fopen(fn, "w+");
    if (fout)
   	 	fclose(fout);
    
    aiengine_start(agn, _param, uuid, _cloud_syn_callback, NULL);
    aiengine_stop(agn);

    /* waiting for the end of sync */
    while(g_play_end_flag == 0)
    {
        usleep(1000);
    }
    
    if(_param)
    {
        free(_param);
    }

    if(param_js)
    {
        cJSON_Delete(param_js);
    }
}


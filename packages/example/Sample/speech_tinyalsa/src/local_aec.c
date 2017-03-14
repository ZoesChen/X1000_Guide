#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "cJSON.h"
#include "echo_wakeup.h"
#include "sound_dev.h"

int error_flag = FALSE;
int aec_wakeup_flag = AEC_SLEEP;

static const char *param = "{"
"   \"request\": {"
"       \"env\": \"words=你好小乐;\""
"   }"
"}";


int _wakeup_aec_callback(const void *usrdata, const char *id,
                            int type,const void *message, int size)
{
    //printf("resp data: %.*s\n", size, (char *) message);
    cJSON *out = cJSON_Parse((char*) message);
    if (!out)
    {
        return -1;
    }

    cJSON *result = cJSON_GetObjectItem(out, "result");
    if (result)
    {
        cJSON *wakeupWrd = cJSON_GetObjectItem(result, "wakeupWord");
        if (wakeupWrd && !strcmp(wakeupWrd->valuestring, "你 好 小 乐"))
        {
            printf("=======>唤醒成功<=======\n");
            aec_wakeup_flag = AEC_WAKEUP;
        }
    }


    AIENGINE_PRINT("resp data: %.*s\n", size, (char *) message);
    if (out)
    {
        cJSON_Delete(out);
    }
    return 0;
}


int AI_aec(echo_wakeup_t *ew)
{
	int ret = 0;
	error_flag = FALSE;
	aec_wakeup_flag = AEC_SLEEP;
	echo_wakeup_reset(ew);

	echo_wakeup_register_handler(ew, NULL, _wakeup_aec_callback);
	if(echo_wakeup_start(ew, param) != 0)
	{
		AEC_PRINT("\"Aiengine start failed!\"\n");
		error_flag = TRUE;
		goto ECHO_STOP;
	}

	ret = sound_device_init(VOLUME, CHANEL_1);
	if(ret < 0) {
		printf("sound device init error!\n");
		return -1;
	}

	ret = dmic_read_handle(ew);
	if(ret < 0) {
		printf("echo handle mic data error!\n");
	}


	sound_device_release();
ECHO_STOP:
	echo_wakeup_end(ew);

	return 0;
}


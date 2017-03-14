#ifndef __AI_MAIN_H__
#define __AI_MAIN_H__

enum AIENGINE_STAT
{
    AI_AEC_ST = 0,
    AI_SEM_ST,
    AI_SYNC_ST,
    AI_ST_MAX
};

#define RECORD_BUFSZ                (1024)
#define TMP_BUFFER_SZ               (1024)
#define STR_BUFFER_SZ               (2048)

#ifdef AI_DBG
#define AIENGINE_PRINT(format, ...) {printf("[%s : %s : %d] ", __FILE__, __func__, __LINE__); printf(format, ##__VA_ARGS__);}
#else
#define AIENGINE_PRINT(format, ...)
#endif

#endif


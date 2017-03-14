#ifndef __AISPEECH__ECHO_WAKEUP_ECHO_WAKEUP_H_INCLUDED__
#define __AISPEECH__ECHO_WAKEUP_ECHO_WAKEUP_H_INCLUDED__

typedef int (*echo_wakeup_handler_f)(const void *usrdata, const char *id, int type, const void *message,
                                     int bytes);

typedef struct echo_wakeup {
    void *echo;
    void *wakeup;

    void *wakeup_cfg;
    void *buffer_echo;
    void *buffer_ref;

    void *usrdata;
    echo_wakeup_handler_f h;
} echo_wakeup_t;

#ifdef __cplusplus
extern "C" {
#endif

echo_wakeup_t *echo_wakeup_new(const char *cfg);
void echo_wakeup_delete(echo_wakeup_t *ew);
void echo_wakeup_reset(echo_wakeup_t *ew);

void echo_wakeup_register_handler(echo_wakeup_t *ew, void *usrdata, echo_wakeup_handler_f h);
int echo_wakeup_start(echo_wakeup_t *ew, const char *req);
int echo_wakeup_process(echo_wakeup_t *ew, const char *echo, const char *ref, int bytes);
int echo_wakeup_end(echo_wakeup_t *ew);

#ifdef __cplusplus
}
#endif
#endif //__AISPEECH__ECHO_WAKEUP_ECHO_WAKEUP_H_INCLUDED__

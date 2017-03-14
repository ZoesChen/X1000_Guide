#ifndef WEBRTC_AEC_H_
#define WEBRTC_AEC_H_

#ifdef __cplusplus
extern "C" {
#endif
int ingenic_apm_init();
int ingenic_apm_set_far_frame(short *buf);
int ingenic_apm_set_near_frame(short *input,short *output, int time);
void ingenic_apm_destroy();
#ifdef __cplusplus
}
#endif
#endif


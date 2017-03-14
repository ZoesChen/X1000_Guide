#ifndef __ULA_DOA_H__
#define __ULA_DOA_H__

#include <malloc.h>
#include "ula_type.h"
#include "ula_beamform.h"
typedef struct
{
	U8 chan1;
	U8 chan2;
	S32 fs;
    S32 ov;
	S32 vs;
    S32 nf;		// 
    S32 nfh;
	S32 nd;
	S32 nfo;
	S32 ndo;
	S32 L;
	S32 delay_old;
	S32 frame_len;
	S32 frame_inc;
    S32 doa_output;
	S32 win_len;
	U32 fft_len;
    U32 ulReset;
	FLOAT alpha;
	FLOAT alpha1;
	FLOAT doa_thres;
    DOUBLE *fft1;
    DOUBLE *fft2;
    DOUBLE *sxy;
    DOUBLE *cxy;
	DOUBLE *tmp_buf;
    DOUBLE *han_win;
	DOUBLE delta;
	DOUBLE mics[8];
    S16 theta_thresh_l;
    S16 theta_thresh_h;
    U32 store_len;
    S16 *store_buf;
    S32 (*pGetTheta_callback)(DOUBLE theta);
}ula_doa_t;

#ifdef ULA_DBG
#define ULA_DOA_DBG(...)        do	\
								{	\
									printf("\n\n**** ULA Debug in %s() %d: ****\n\n", __FUNCTION__, __LINE__);	\
									printf(__VA_ARGS__);										\
									printf("\n\n**** ULA Debug End ****\n\n");		\
								}while(0)

#else
#define ULA_DOA_DBG(...)
#endif

#ifdef ULA_PRETEST
#define ULA_PRETEST_DBG(...)        do	\
								{	\
									printf(__VA_ARGS__);    \
								}while(0)

#else
#define ULA_PRETEST_DBG(...)
#endif

#define FUNCTION(a, b)  a##b

S32 ULA_init(VOID *private, ula_doa_t **pDoa, ula_beamform_t **pBeamform);
VOID ULA_reset(ula_doa_t *pstDoa, ula_beamform_t *pstBeamform);
S32 ULA_confParse(S8 *cfg, ula_doa_t *pstDoa, ula_beamform_t *pstBeamform);
VOID ULA_doaRelease(ula_doa_t *pstDoa);

DOUBLE ULA_multi2monoPhis(ula_doa_t *pstDoa, ula_beamform_t *pstBeamform, VOID *pInData, U32 inLen, ula_beamform_private_t *pstPriData, VOID *pOutData);
S32 ULA_multi2monoPhisBF(ula_doa_t *pstDoa, ula_beamform_t *pstBeamform, VOID *pInData, U32 inLen, ula_beamform_private_t *pstPriData, VOID *pOutData);
S32 ULA_multi2monoBF(ula_doa_t *pstDoa, ula_beamform_t *pstBeamform, VOID *pInData, U32 inLen, ula_beamform_private_t *pstPriData, VOID *pOutData);
#endif


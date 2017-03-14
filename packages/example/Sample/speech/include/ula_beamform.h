#ifndef _ULA_BEAMFORM_H__
#define _ULA_BEAMFORM_H__

typedef struct
{
	S32 wav_len;	//wav data len(in short) per mic, with out wav header
    S16 *data;
}ula_wav_data_t;

/* macros */
#define TWO_PI 					(6.2831853071795864769252867665590057683943L)
#define PI 						(TWO_PI/2)

/***************************************/
typedef struct
{
	S8 useSuperDirec;	/* 
								 * 0 - don't use super directive 
								 * 1 - using super directive
								 */
	U32 c;
	U32 fs;			/* sample per second */
	U32 frm_len;		/* frame len */
	U32 frm_inc;		/* frame increase */
	U32 L;				/* align */
	U32 chansOfWav;     /* chans of wav data per time */
    U32 chansOfAct;     /* actual used chans */
    U32 ulReset;
	FLOAT sigma;
	DOUBLE theta;
	DOUBLE *d;
	DOUBLE *weight;
	DOUBLE *han_win;			/* points at hanning window	*/
	ula_wav_data_t *pstInWav;	/* points at in wave */
	ula_wav_data_t *pstOutWav;	/* points at out wave */
    DOUBLE *pdFFTBUF;
    DOUBLE *pdESTBUF;
}ula_beamform_t;

typedef struct
{
	U32 chans;			/* actual chans */
	U32 fs;			    /* Sample rate */
    U32 ulMax;          /* max input data length */
    U32 ulTotalOutLen;  /* max ouput length */
    S32 lEndFlag;       /* 0 - data is not end, more data to be handled
                         * 1 - data is end, no more data to be handled
                         */
    S8 acConfig[1024];   /* config file */
}ula_beamform_private_t;

enum ULA_RTN_STATE
{
	ULA_RTN_FAIL = -1,
	ULA_RTN_SUCCESS
};

/* prototype declear */
DOUBLE * ULA_hanningWindow(U32 N);
VOID ULA_hanningWindowWithSqrt(U32 N, DOUBLE **pdBuf);
VOID ULA_beamformWeightInit(ula_beamform_t *pstBeamform);
S32 ULA_beamformInit(ula_beamform_t *pstBeamform, U32 chans, U32 fs, S32 useCfgFile);
VOID ULA_beamformCal(ula_beamform_t *pstBeamform, U32 handle_len, U32 *offset, U32 pointsPerChan);
VOID ULA_beamformRelease(ula_beamform_t *pstBeamform);
#endif

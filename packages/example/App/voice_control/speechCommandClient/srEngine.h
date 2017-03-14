#ifndef __SR_ENGINE_h__
#define __SR_ENGINE_h__

//void * (*speech_call_back)(const char*,int );

int start_speech_engine( void * (*speech_call_back)(const char*,int ) );


#endif	/* __SR_ENGINE_h__ */

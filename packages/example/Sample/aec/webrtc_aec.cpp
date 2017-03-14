#include <stdio.h>
#include <time.h>
#include <sys/time.h>
//#include <audio_processing.h>
//#include <audio_buffer.h>
//#include <module_common_types.h>
#include "webrtc_aec.h"
#include <webrtc_audio_processing/audio_processing.h>
//#include <webrtc_audio_processing/audio_buffer.h>
#include <webrtc_audio_processing/module_common_types.h>

using webrtc::AudioFrame;
//using webrtc::AudioBuffer;
using webrtc::AudioProcessing;
using webrtc::EchoCancellation;
using webrtc::EchoControlMobile;
using webrtc::GainControl;
using webrtc::HighPassFilter;
using webrtc::LevelEstimator;
using webrtc::NoiseSuppression;
using webrtc::VoiceDetection;

#define AEC		/* acoustic echo cancellation */
#define AECM		/* acoustic echo control for mobile ;it is not recommended to use*/
#define AGC		/* automatic gain control */
#define HP		/* High Pass Filter */
#define LE		/* Level Estimator */
#define NS		/* noise suppression */
#define VAD		/* voice activity detection */

#define SAMPLE_RATE	8000
#define SAMPLE_TIME	10
#define CHANNEL_NUM	1

AudioProcessing *apm = NULL;
AudioFrame *far_frame = NULL;
AudioFrame *near_frame = NULL;

int ingenic_apm_init() {
	int32_t sample_rate_hz = SAMPLE_RATE;
	int num_capture_input_channels = CHANNEL_NUM;
	int num_capture_output_channels = CHANNEL_NUM;
	int num_render_channels = CHANNEL_NUM;

        apm = AudioProcessing::Create(0);
	if(apm == NULL) {
		printf("AudioProcessing::Create() error !\n");
		return -1;
	}

	apm->set_sample_rate_hz(sample_rate_hz);

	apm->set_num_channels(num_capture_input_channels, num_capture_output_channels);

	apm->set_num_reverse_channels(num_render_channels);

#ifdef AEC
	apm->echo_cancellation()->Enable(true);
	apm->echo_cancellation()->set_device_sample_rate_hz(sample_rate_hz);
	apm->echo_cancellation()->set_stream_drift_samples(0);
	apm->echo_cancellation()->enable_drift_compensation(false);
	apm->echo_cancellation()->set_suppression_level(EchoCancellation::kLowSuppression);	//0
//	apm->echo_cancellation()->set_suppression_level(EchoCancellation::kModerateSuppression);//1
//	apm->echo_cancellation()->set_suppression_level(EchoCancellation::kHighSuppression);	//2
	apm->echo_cancellation()->enable_metrics(true);
	apm->echo_cancellation()->enable_delay_logging(true);
#else	//AECM
	apm->echo_control_mobile()->Enable(true);
//	apm->echo_control_mobile()->set_routing_mode(EchoControlMobile::kQuietEarpieceOrHeadset);
//	apm->echo_control_mobile()->set_routing_mode(EchoControlMobile::kEarpiece);
//	apm->echo_control_mobile()->set_routing_mode(EchoControlMobile::kLoudEarpiece);
//	apm->echo_control_mobile()->set_routing_mode(EchoControlMobile::kSpeakerphone);
	apm->echo_control_mobile()->set_routing_mode(EchoControlMobile::kLoudSpeakerphone);
	apm->echo_control_mobile()->enable_comfort_noise(true);
//???   apm->echo_control_mobile()->SetEchoPath();
#endif

#ifdef AGC
	apm->gain_control()->Enable(true);
	apm->gain_control()->set_mode(GainControl::kFixedDigital);	//2
	apm->gain_control()->set_target_level_dbfs(15);			//Limited to [0, 31];3, 20
	apm->gain_control()->set_compression_gain_db(0);		//Limited to [0, 90];9, 30
	apm->gain_control()->enable_limiter(true);
#endif

#ifdef HP
	apm->high_pass_filter()->Enable(true);
#endif

#ifdef LE
	apm->level_estimator()->Enable(true);
#endif

#ifdef NS
	apm->noise_suppression()->Enable(true);

//      apm->noise_suppression()->set_level(NoiseSuppression::kLow);		//0
 //     apm->noise_suppression()->set_level(NoiseSuppression::kModerate);	//1
	apm->noise_suppression()->set_level(NoiseSuppression::kHigh);		//2
//	apm->noise_suppression()->set_level(NoiseSuppression::kVeryHigh);	//3
#endif

#ifdef VAD
	apm->voice_detection()->Enable(true);
//	apm->voice_detection()->set_likelihood(VoiceDetection::kVeryLowLikelihood);	//0
	apm->voice_detection()->set_likelihood(VoiceDetection::kLowLikelihood);		//1
//	apm->voice_detection()->set_likelihood(VoiceDetection::kModerateLikelihood);	//2
//	apm->voice_detection()->set_likelihood(VoiceDetection::kHighLikelihood);	//3

	/* aec_test: webrtc_x1000/src/modules/audio_processing/voice_detection_impl.cc:131: virtual int webrtc::VoiceDetectionImpl::set_frame_size_ms(int): Assertion `size == 10' failed.
	 */
//	apm->voice_detection()->set_frame_size_ms(30);
#endif

	apm->Initialize();

	return 0;
}

void dump()
{
	int ret;
	int sample_rate_hz_dump = apm->sample_rate_hz();
	int num_input_channels_dump = apm->num_input_channels();
	int num_output_channels_dump = apm->num_output_channels();
	int num_reverse_channels_dump = apm->num_reverse_channels();
	int stream_delay_ms_dump = apm->stream_delay_ms();
	printf("sample rate : %d\n", sample_rate_hz_dump);
	printf("num_input_channels : %d\n", num_input_channels_dump);
	printf("num_output_channels : %d\n", num_output_channels_dump);
	printf("num_reverse_channels : %d\n", num_reverse_channels_dump);
	printf("stream_delay_ms : %d\n", stream_delay_ms_dump);

/* AEC */
	ret = apm->echo_cancellation()->is_enabled();
	if(ret) {
		printf("AEC enable !\n");
		ret = apm->echo_cancellation()->is_drift_compensation_enabled();
		if(ret) {
			printf("\t\tenable_drift_compensation");

			ret = apm->echo_cancellation()->device_sample_rate_hz();
			printf("\t\t\tdevice_sample_rate_hz : %d\n", ret);
			ret = apm->echo_cancellation()->stream_drift_samples();
			printf("\t\t\tstream_drift_samples : %d\n", ret);
		}

		ret = apm->echo_cancellation()->suppression_level();
		printf("\t\tsuppression_level : %d\n", ret);

		ret = apm->echo_cancellation()->are_metrics_enabled();
		if(ret) {
			printf("\t\tenable_metrics\n");

		//???	apm->echo_cancellation()->GetMetrics();//dai yong
		}

		ret = apm->echo_cancellation()->is_delay_logging_enabled();
		if(ret) {
			printf("\t\tenable_delay_logging\n");

		//???	apm->echo_cancellation()->GetDelayMetrics();//dai yong
		}
	}

/* AECM */
	ret = apm->echo_control_mobile()->is_enabled();
	if(ret) {
		printf("AECM enable !\n");

		ret = apm->echo_control_mobile()->routing_mode();
		printf("\t\trouting_mode : %d\n", ret);

		ret = apm->echo_control_mobile()->is_comfort_noise_enabled();
		if(ret)
			printf("\t\tenable_comfort_noise\n");

	//???	apm->echo_control_mobile()->GetEchoPath();
	//???	apm->echo_control_mobile()->echo_path_size_bytes();
	}

/* AGC */
	ret = apm->gain_control()->is_enabled();
	if(ret == 1) {
		printf("AGC enabled !\n");

		ret = apm->gain_control()->mode();
		printf("\t\tmode : %d\n", ret);

		ret = apm->gain_control()->target_level_dbfs();
		printf("\t\ttarget_level_dbfs : %d\n", ret);

		ret = apm->gain_control()->compression_gain_db();
		printf("\t\tcompression_gain_db : %d\n", ret);

		ret = apm->gain_control()->is_limiter_enabled();
		if(ret)
			printf("\t\tlimiter_enabled\n");
	}

/* HP */
	ret = apm->high_pass_filter()->is_enabled();
	if(ret)
		printf("HighPassFilter is enabled\n");

/* LE */
	ret = apm->level_estimator()->is_enabled();
	if(ret) {
		printf("LevelEstimator is enable\n");
		/* not support */
		/*
		Metrics metrics, reverse_metrics;
		apm->level_estimator()->GetMetrics((Metrics*) &metrics, (Metrics*) &reverse_metrics);
		*/
	}

/* NS */
	ret = apm->noise_suppression()->is_enabled();
	if(ret) {
		printf("NoiseSuppression is enabled !\n");
		ret = apm->noise_suppression()->level();
		printf("\t\tNoiseSuppression : %d\n", ret);
	}

/* VAD */
	ret = apm->voice_detection()->is_enabled();
	if(ret) {
		printf("voice activity detection is enable !\n");

		ret = apm->voice_detection()->likelihood();
		printf("\t\tlikelihood : %d\n", ret);

		ret = apm->voice_detection()->frame_size_ms();
		printf("\t\tframe size per ms : %d\n", ret);
	}
}

int ingenic_apm_set_far_frame(short *buf) {
	int i, ret;
	far_frame = new AudioFrame();
	far_frame->_audioChannel = 1;
	far_frame->_frequencyInHz = 8000;
	far_frame->_payloadDataLengthInSamples = far_frame->_frequencyInHz/100;

	for(i=0;i<far_frame->_payloadDataLengthInSamples;i++)
	      //far_frame->_payloadData[i]=buf[i] * 20;
	      far_frame->_payloadData[i]=buf[i] * 2;

	ret = apm->AnalyzeReverseStream(far_frame);
	if(ret < 0) {
		printf("far AnalyzeReverseStream() error : %d\n", ret);
	}

    delete far_frame ;
	return 0;
}

int ingenic_apm_set_near_frame(short *input, short *output, int time){
	int i, ret;
	near_frame = new AudioFrame();
	near_frame->_audioChannel = 1;
	near_frame->_frequencyInHz = 8000;
	near_frame->_payloadDataLengthInSamples = near_frame->_frequencyInHz/100;

	for(i=0;i<near_frame->_payloadDataLengthInSamples;i++){
	      near_frame->_payloadData[i]=input[i];
	}
	apm->set_stream_delay_ms(time);
#ifdef VAD
//	apm->voice_detection()->set_stream_has_voice(false);
#endif

#ifdef AEC_BAK//dai yong
	ret = apm->echo_cancellation()->stream_has_echo();
	if(ret == 0) {
		printf("AEC : the current frame almost certainly contains no echo\n");
		return 0;
	}
	printf("the current frame contains echo\n");
#endif

	ret = apm->ProcessStream(near_frame);
	if(ret < 0) {
		printf(" near AnalyzeReverseStream() error : %d\n", ret);
	}

#ifdef VAD
	ret = apm->voice_detection()->stream_has_voice();
	if(ret == 0) {
		/* there is not human voice */
		for(i = 0; i < near_frame->_payloadDataLengthInSamples; i++)
			near_frame->_payloadData[i] = near_frame->_payloadData[i] / 5;
//			near_frame->_payloadData[i] = 0;
	}
#endif

	memcpy(output, near_frame->_payloadData, near_frame->_payloadDataLengthInSamples*sizeof(short));
    delete near_frame;
	return 0;
}

void ingenic_apm_destroy() {
	dump();

	AudioProcessing::Destroy(apm);//remove this method
	apm = NULL;
/*
	if (far_frame != NULL) {
	      delete far_frame;
	      far_frame = NULL;
	}

	if (near_frame != NULL) {
	      delete near_frame;
	      near_frame = NULL;
	}
	*/
}


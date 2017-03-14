#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <srEngine.h>
//#include <ClientConfig.h>
#include <ActionSendSpeechCommand.h>


#define SPEECH_TEXT_MAX (128)

char speech_text[SPEECH_TEXT_MAX];
int speech_len;

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pthread_cond;

extern int start_speech_engine( void * (*speech_call_back)(const char*,int ) );

/* move speech text to reserved buffer */
void * speech_callback(const char* buf, int len)
{

	printf("1111 speech_call_back:(%d) %s\n", len, buf);

	pthread_mutex_lock(&buffer_mutex);
	memset((void*)speech_text, '\0', SPEECH_TEXT_MAX);
	//strncpy(speech_text, buf, SPEECH_TEXT_MAX);

	if (len > SPEECH_TEXT_MAX-1)
		len = SPEECH_TEXT_MAX-1;

	memcpy((void*)speech_text, (void*)buf, len);
	printf("2222 speech_call_back:(%d) %s\n", len, speech_text);

	speech_len = len;
	pthread_cond_signal(&pthread_cond);

	pthread_mutex_unlock(&buffer_mutex);

	return NULL;
}

void *speech_thread_routine(void* data)
{

	while (1) {
		printf("speech_thread_routine() loop...\n");
		start_speech_engine(speech_callback);
		sleep(2);
	}
	return 0;
}


int init_speech_thread(int c)
{
	pthread_t speech_thread_id;
	pthread_mutex_init(&buffer_mutex, NULL);
	pthread_cond_init(&pthread_cond, NULL);
	pthread_create(&speech_thread_id, NULL, speech_thread_routine, NULL);
}

int do_speech_loop(int client_sockfd, ClientConfig * gConfig)
{
	/*  */
	while (1) {
		printf("main thread loop...\n");

		pthread_mutex_lock(&buffer_mutex);

		/* wait speech commands */
		pthread_cond_wait(&pthread_cond, &buffer_mutex);
		printf("main thread wakeuped by srEngine: (%d) %s\n", speech_len, speech_text);
		{
			ActionSendSpeechCommand * act;
			DEBUG_LINE();
			act = new ActionSendSpeechCommand(client_sockfd, gConfig);
			DEBUG_LINE();
			//act->setSpeechCommand("TurnON");
			act->setSpeechCommand(speech_text);
			DEBUG_LINE();
			act->commitToServer();

			DEBUG_LINE();
			delete act; // ???
		}
		pthread_mutex_unlock(&buffer_mutex);

	}

	return 0;
}


#if 0
int main(int argc, char *argv[])
{
	pthread_t speech_thread_id;

	pthread_mutex_init(&buffer_mutex, NULL);

	pthread_cond_init(&pthread_cond, NULL);

	pthread_create(&speech_thread_id, NULL, speech_thread_routine, NULL);

	/*  */
	while (1) {
		printf("main thread loop...\n");

		pthread_mutex_lock(&buffer_mutex);

		/* wait speech commands */
		pthread_cond_wait(&pthread_cond, &buffer_mutex);
		printf("main thread wakeuped by srEngine: (%d) %s\n", speech_len, speech_text);

		pthread_mutex_unlock(&buffer_mutex);

	}

#if 0
	pthread_join(speech_thread_id, NULL);
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&count_mutex);
	pthread_cond_destroy(&count_threshold_cv);
	pthread_exit(NULL);
#endif
	return 0;
}
#endif

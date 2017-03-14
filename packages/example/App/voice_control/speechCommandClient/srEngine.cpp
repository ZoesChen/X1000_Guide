

#include <stdio.h>
#include <string.h>

#define SPEECH_TEXT_MAX (128)

#define CMD_FILE "sr_cmd.conf"

//export LD_LIBRARY_PATH=/data1/home/lgwang/pocketsphinx_install/lib:$LD_LIBRARY_PATH

/* host 192.168.4.15: */

#define CMD_BUFFER_SIZE (2048)
char sr_cmd_buffer[CMD_BUFFER_SIZE];

/*
const char* sr_cmd_1 ="/data1/home/lgwang/pocketsphinx_install/bin/pocketsphinx_continuous \
-hmm /data1/home/lgwang/pocketsphinx_install/share/pocketsphinx/model/zh-cn/zh_broadcastnews_ptm256_8000/ \
-lm /data1/home/lgwang/pocketsphinx_install/share/pocketsphinx/model/Command/2347.lm \
-dict /data1/home/lgwang/pocketsphinx_install/share/pocketsphinx/model/Command/2347.dic \
-infile /data1/home/lgwang/pocketsphinx_install/share/pocketsphinx/model/speech_wav/one_two_three_loudly.wav \
2>/tmp/sr.log";
*/
// for Manhattan
const char* sr_cmd_1 = "\
						/usr/sbin/pocketsphinx_continuous \
						-hmm /pocketsphinx/tdt_sc_8k \
						-lm /pocketsphinx/7010.lm \
						-dict /pocketsphinx/7010.dic \
						-inmic yes \
						2>/tmp/sr.log";

/*
   -time true \

-infile /data1/home/lgwang/pocketsphinx_install/share/pocketsphinx/model/speech_wav/one_two_three_loudly.wav \
-infile /data1/home/lgwang/rec_wav/rec_LingXiLingXi_60s.wav \
*/



char *parse_speech_engine_command(char* fname, char *cmd_buf, int buf_size)
{
	FILE   *f;
	char * buf;
	int max_len, len;

	f = fopen(fname, "r");

	if (!f) {
		printf("%s() fopen(\"%s\") failed.\n", __FUNCTION__, fname);
		return NULL;
	}

	buf = cmd_buf;

	max_len = buf_size;
	memset(buf, 0, max_len);

	len = 0;
	while (!feof(f)) {
		len += fread(buf, sizeof(char), max_len-len, f);
		buf += len;
		if (len >= max_len)
			break;
	}

	fclose(f);

	return cmd_buf;
}





/*
the popen out:(128) ~~~~~~~~~~关闭 窗口 关闭~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
filter_speech_command():
1111 speech_call_back:(20) 关闭 窗口 关闭~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

2222 speech_call_back:(20) 关闭 窗口 关闭

 */

int filter_speech_command(const char *speech, int len, const char ** text, int * text_len)
{
	const char *s;
	int i, pos_s;

	if (! speech || len <3 || !text)
		return -1;

	s = speech;

	*text = NULL;

	for (i=0; i<len; i++) {
		/* text start */
		if ( *text == NULL && *s != '~') {
			*text = s;
			pos_s = i;
		}

		/* text end */
		if ( *text != NULL && *s == '~') {
			*text_len = i - pos_s;
			break;
		}
		s++;
	}


	if (i==len) {
		printf("failed filter_speech_command()...\n");
	}

	//printf("speech= %p, *text= %p, *text_len= %d\n", speech, *text, *text_len);


	return 0;
}


int start_speech_engine( void * (*speech_call_back)(const char*,int ) )
{
	FILE   *stream;
	char   buf[SPEECH_TEXT_MAX+1];
	int len;
	char * cmd;

	cmd = parse_speech_engine_command(CMD_FILE, sr_cmd_buffer, CMD_BUFFER_SIZE);

	if (!cmd) {
		printf("start_speech_engine() parse_speech_engine_command failed\n");
		return -1;
	}

	//printf("popen(cmd): \n\t%s\n", cmd);
   	stream = popen( cmd, "r" );

	//fread( buf, sizeof(char), sizeof(buf), stream);
	//printf("the popen out:%s\n", buf);

	if (stream != NULL)
	{
		while (!feof(stream))
		{
			const char * text;
			int text_len;
			memset( buf, '\0', sizeof(buf) );

			//len = fread(buf, sizeof(char), sizeof(buf), stream);
			len = fread(buf, sizeof(char), SPEECH_TEXT_MAX, stream);
			printf("the popen out:(%d) %s\n", len, buf);

			/* test filter_speech_command() */
			//sleep(1);strcpy(buf, "abcd"); len = strlen(buf);
#if 1
			text = NULL;
			text_len = 0;
			filter_speech_command(buf, len, &text, &text_len);
			if (text && text_len>0)
				speech_call_back(text, text_len);
#endif
		}

		pclose(stream);
	}

	//pclose( stream );

	return 0;
}

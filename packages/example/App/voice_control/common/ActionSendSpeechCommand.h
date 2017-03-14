#ifndef __ACTION_SEND_SPEECH_COMMAND_H
#define __ACTION_SEND_SPEECH_COMMAND_H

#include <ActionBase.h>


class ClientConfig;

class ActionSendSpeechCommand: public ActionBase
{
public:
	ActionSendSpeechCommand(int sock, ClientConfig *config);
	~ActionSendSpeechCommand();

	int setSpeechCommand(const char * cmd);

private:
	ClientConfig *mConfig;
};


struct ClientHeader;
class ActionSendSpeechCommandParser: public ActionBaseParser
{
public:
	ActionSendSpeechCommandParser(cJSON *jsonRoot);
	~ActionSendSpeechCommandParser();

	int getSpeechCommand(const char ** cmd);

private:

};

#endif	/* __ACTION_SEND_SPEECH_COMMAND_H */

#ifndef __ACTION_OK_H
#define __ACTION_OK_H

#include <ActionBase.h>

class ActionOK: public ActionBase
{
public:
	ActionOK(int sock);
	~ActionOK();

private:


};


class ActionOKParser: public ActionBaseParser
{
public:
	ActionOKParser(cJSON *jsonRoot);
	~ActionOKParser();

private:
//	ClientHeader* mClientHeader;
};

#endif	/* __ACTION_OK_H */

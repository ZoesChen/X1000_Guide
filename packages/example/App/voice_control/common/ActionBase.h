#ifndef __ACTION_BASE_H__
#define __ACTION_BASE_H__

#include <cJSON.h>
#include <debug.h>
#include <stdlib.h>


struct OperateStruct {
	char * name;
	char * value;		/* int ? */
};

// --------------------------- Action ---------------------------------
/* cjson wrapper */
class ActionBase
{
public:
	ActionBase(int sock);
	~ActionBase();		/* free mJsonString */

	int addRootItem(struct OperateStruct * ops);
	int setActionType(const char * action);

	/* Client Header */
	int addClientHeaderItem(struct OperateStruct * ops);
	int addClientHeaderItem(const char * name, const char * value);
	int setClientName(const char * name);
	int setClientID(const char * id);

	/* Actions */
	int getOperateItemNum();
	int setOperateName(const char * name);
	int addOperateItem(struct OperateStruct * ops);
	int addOperateItem(const char * ops, const char * value);
	//int getOperateValue(const char * ops, const char * value);

	/* sent message to server */
	int commitToServer();

	const char *makeString() {return makeJsonString();}
	const char *makeJsonString();

private:
	int mSocket;
	cJSON *mJsonRoot;
	cJSON *mJsonClientHeader;
	cJSON *mJsonOperate;
	int m_ops_num;
	const char * mJsonString;

	const char * mOperatename;
};


// --------------------------- Action Parser ---------------------------------
cJSON * messageStringToCJson(const char *msg);
const char * getActionType(cJSON * jsonMsg);

class ActionBaseParser
{
public:
	ActionBaseParser( char * jsonStr);
	ActionBaseParser(cJSON *jsonObj);
	~ActionBaseParser();

	const char * getActionType() { return mActionType;}

	/* Client Header */
	const char * getClientHeaderItemValue(const char * name) { return NULL;}

	const char * getClientName();
	const char * getClientID();

	/* Actions */
	int getOperateNum();
	const char * getOperateValue(const char * name);
	//int parseOperateItem(int item, struct OperateStruct * ops);

protected:
	int parseState;

private:
	char * mJsonStr;
	cJSON *mJsonRoot;
	cJSON *mJsonClientHeader;
	cJSON *mJsonOperate;
	int m_ops_num;

	const char * mActionType;
	const char * mClientName;
	const char * mClientID;

	/* operate list??? */

};


#endif	/* __ACTION_BASE_H__ */

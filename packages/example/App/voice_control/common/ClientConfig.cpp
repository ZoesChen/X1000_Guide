#include <ClientConfig.h>


ClientConfig::ClientConfig()
{
	return;
}

ClientConfig::~ClientConfig()
{
	return;
}

int ClientConfig::setClientName(const char *name)
{
	int mlen=(strlen(name) > MAX_BUFF)?strlen(name):MAX_BUFF;
	strcpy(mClientName,name);
	return 0;
}

int ClientConfig::setClientID(const char * id)
{
	int mlenid=(strlen(id) > MAX_BUFF)?strlen(id):MAX_BUFF;
	strcpy(mClientID,id);
	return 0;
}

int ClientConfig::setFuncVector(vector<string> cvector)
{
	mClientFuncVector = cvector;
	return 0;
}

const char * ClientConfig::getClientName()
{
	printf("!!!!!!!!!!!!!!!!!!!!!!!!---------i%s-----\n", mClientName );
	return mClientName;
}

const char * ClientConfig::getClientID()
{
	return mClientID;
}

ClientHeader * ClientConfig::getClientHeader()
{
	return &mClientHeader;
}

vector<string> * ClientConfig::getClientFuncVector()
{
	return &mClientFuncVector;
}

char * ClientConfig::getVectorKey(int i)
{
	char *p	=(char*)mClientFuncVector[i].substr(0,mClientFuncVector[i].find("=")).c_str();
	return p;
}

char * ClientConfig::getVectorValue(int i)
{
	char *p	=(char*)mClientFuncVector[i].substr(mClientFuncVector[i].find("=")+1).c_str();
	return p;
}

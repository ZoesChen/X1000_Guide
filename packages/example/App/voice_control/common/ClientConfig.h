#ifndef __CLIENT_CONFIG_H__
#define __CLIENT_CONFIG_H__

#include <ClientHeader.h>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#define MAX_BUFF 50

using namespace std;

class ClientConfig
{
	public:
		ClientConfig();
		~ClientConfig();

		int setClientName(const char *);
		int setClientID(const char *);
		int setFuncVector(vector<string> cvector );

		const char * getClientName();
		const char * getClientID();
		char * getVectorKey(int);
		char * getVectorValue(int);
		ClientHeader * getClientHeader();
		vector<string> *getClientFuncVector();



	private:
		char mClientID[MAX_BUFF];
		char mClientName[MAX_BUFF];
		ClientHeader mClientHeader;
		vector<string>  mClientFuncVector;
};

#endif	/* __CLIENT_CONFIG_H__ */

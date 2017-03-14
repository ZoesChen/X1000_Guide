#ifndef __INI_PARSER_H__
#define __INI_PARSER_H__

#include <ClientConfig.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <fstream>


using namespace std;

#define MAX_FILE 512
#define COMMENT_FLG '#'
#define SETCTION_BEGIN_FLG '['
#define SETCTION_END_FLG ']'

class IniParser
{
	public:
		IniParser(ClientConfig* config, const char* filename);
		~IniParser();

		void LoadIniToConfig();
		void storeConfigToIni(char *ini);
		class BaseData *getConfigObj();
		char * GetVectorValue(int i);
		char * GetVectorKey(int i);

	
	private:
		int ReadIniFile();
		void PushBackToVector(string oneLine);

		ClientConfig * m_pClientConfig;

		char * mIniName;
		char mIniFile[20];
		char mFileBuffer[MAX_FILE];

		vector<string> mStrVect;
		vector<string> mHeaderVect;
		vector<string> mOperatorVect;
};

#endif	/* __INI_PARSER_H__ */

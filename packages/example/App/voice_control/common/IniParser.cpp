#include <IniParser.h>

static string strTrim(string aStr)
{
	string s=aStr;
	unsigned int first,last;
	if(string::npos != (first =s.find_first_not_of(' ')))
	{
		s=s.substr(first,s.length()-first);
	}
	if(string::npos != (last =s.find_last_not_of(' ')))
	{
		s=s.substr(0,last+1);
	}
	return s;
}


IniParser::IniParser(ClientConfig * config, const char * inifile)
{
	m_pClientConfig = config;
	int iLen =(strlen(inifile) > MAX_FILE)?strlen(inifile):MAX_FILE;
	strcpy(mIniFile, inifile);
}

IniParser::~IniParser()
{
}


int IniParser::ReadIniFile()
{
	ifstream fin(mIniFile);
	string strLine;
	unsigned int Last;
	while (std::getline(fin,strLine).good())
	{
		if(string::npos != (Last = strLine.find_last_not_of('\r')))
		{
			strLine =strLine.substr(0,Last+1);
		}
		PushBackToVector(strLine);
	}

	vector<string>::iterator it =mStrVect.begin();
	for(it;it!=mStrVect.end();it++)
	{
		cout <<"----------1---iniparse vector   :" << *it <<endl;
	}
	return 0;
}

void IniParser::PushBackToVector(string oneLine)
{
	unsigned int uPos;
	if(string::npos != (uPos = oneLine.find_first_of(COMMENT_FLG)))
	{
		oneLine = oneLine.substr(0, uPos + 1);
	}
	oneLine=strTrim(oneLine);
	if(oneLine.empty()|| oneLine.length()<2) return;

	unsigned int First, Last;
	First= oneLine.find_first_of(SETCTION_BEGIN_FLG);
	Last= oneLine.find_last_of(SETCTION_END_FLG);

	if(string::npos == First && string::npos == Last)
	{
		mStrVect.push_back(oneLine);
	}

}

void IniParser::LoadIniToConfig()
{
	ReadIniFile();
	char * clientIniClientName = (char*)mStrVect[0].substr(mStrVect[0].find("=")+1).c_str();
	m_pClientConfig ->setClientName(clientIniClientName);
	cout <<"-----2------- getClientName() :" << m_pClientConfig->getClientName() <<endl;
	char * clientIniClientID = (char*)mStrVect[1].substr(mStrVect[1].find("=")+1).c_str();
	m_pClientConfig ->setClientID(clientIniClientID);
	cout <<"-----2------- getClientID() :" << m_pClientConfig->getClientID() <<endl;
	vector<string> cvector;
	cvector.assign(mStrVect.begin()+2,mStrVect.end());
	m_pClientConfig ->setFuncVector(cvector);
	cout <<"-----2------- getClientVector() :" << m_pClientConfig->getClientFuncVector() <<endl;

	vector<string>::iterator it =mStrVect.begin();
	for(it;it!=mStrVect.end();it++)
	{
		cout <<"---2----------iniparse vector   :" << *it <<endl;
	}
}

#ifndef __ZPRECTIME_H
#define __ZPRECTIME_H

#include <iostream>
#include <string>

#ifdef _WINDOWS32
#include <windows.h>
#else
#include <sys/time.h>
#endif

using namespace std;

#ifdef _WINDOWS32
#ifdef PUB_TOOLS_EXPORTS
#define PUB_TOOLS_API __declspec(dllexport)
#else
#define PUB_TOOLS_API __declspec(dllimport)
#endif
#else
#define PUB_TOOLS_API
#endif

class PUB_TOOLS_API ZPrecTime  
{
	string m_strWork;
public:
	ZPrecTime(const string & strWork);
	ZPrecTime(const string & strWork,int flag);
	~ZPrecTime();

public:
	int StartTime();
	
	int FinishTime();
	
private:
    int m_flag;
#ifndef _WINDOWS32		
	struct timeval m_TpStart,m_TpFinish;
    struct timezone m_TzpStart,m_TzpFinish;
#else
    int m_StartMSec;
    int m_EndMSec;
#endif
        
        
};

#endif



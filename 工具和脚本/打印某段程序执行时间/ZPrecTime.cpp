#include "ZPrecTime.h"

//extern float g_syn_every_usedtime;
//extern float g_syn_every_usedtime2;
//extern float g_syn_every_usedtime3;
//extern float g_syn_every_usedtime4;

ZPrecTime::ZPrecTime(const string &strWork)
{	
	m_strWork = strWork;
	m_flag = 0;
	StartTime();
}

ZPrecTime::ZPrecTime(const string &strWork,int flag)
{	
	m_strWork = strWork;
	m_flag = flag;
	StartTime();
}


ZPrecTime::~ZPrecTime()
{
    FinishTime();
}

#ifndef _WINDOWS32
int ZPrecTime::StartTime()
{
	int syn_every_k = 1;
	syn_every_k = gettimeofday(&m_TpStart,&m_TzpStart);
	
	return syn_every_k;
}

int ZPrecTime::FinishTime()
{
	int syn_every_k = 1;
	syn_every_k=gettimeofday(&m_TpFinish,&m_TzpFinish);		
	float syn_every_usedtime =(m_TpFinish.tv_sec-m_TpStart.tv_sec) + (m_TpFinish.tv_usec-m_TpStart.tv_usec)/1000000.0;
//	if (syn_every_usedtime >=1)
//	{
//	cout<<m_strWork<<syn_every_usedtime<<" Ãë"<<endl;
//	}
//	else
//	{
//	cout<<m_strWork<<syn_every_usedtime<<" Ãë"<<endl;
//	}
	
//	if (m_flag == 1)
//	    g_syn_every_usedtime = syn_every_usedtime;
//	else if (m_flag == 2)
//	    g_syn_every_usedtime2 = syn_every_usedtime;
//	else if (m_flag == 3)
//	    g_syn_every_usedtime3 = syn_every_usedtime;
//	else if (m_flag == 4)
//	    g_syn_every_usedtime4 = syn_every_usedtime;
	    
	if (m_flag == 0)
	    cout<<m_strWork<<syn_every_usedtime<<" Ãë"<<endl;
	
	return syn_every_k;
}
#else
int ZPrecTime::StartTime()
{
    m_StartMSec = timeGetTime();
    
    return m_StartMSec;
}

int ZPrecTime::FinishTime()
{
    m_EndMSec = timeGetTime();		
    
    int msec = m_EndMSec - m_StartMSec;
    
    int sec = msec / 1000;
    double usedtime = (msec % 1000)/1000.0;
    usedtime += sec;

    cout<<m_strWork<<" used time is "<<usedtime<<" seconds"<<endl;
    
    return m_EndMSec;
}
#endif

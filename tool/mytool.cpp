#include "mytool.h"



//��intֵת��Ϊstring����
string MyTool::IntToStr(int value)
{
    string ret_str;
    char buff[10];
    sprintf(buff, "%d", value);
    ret_str = buff;
    return ret_str;
}

//��ĸСдת����д
int MyTool::Check_Toupper(char *str)
{
    int i;
    int len=strlen(str);
    for(i=0; i<len; i++)
    {
        str[i]=toupper(str[i]);
    }

    return 0;
}

//��ĸ��дתСд
int MyTool::Check_Tolower(char *str)
{
    int i;
    int len=strlen(str);
    for(i=0; i<len; i++)
    {
        str[i]=tolower(str[i]);
    }

    return 0;
}
//��ȡ��ǰʱ���ַ���
string MyTool::GetTimeString()
{
    char value_string[50] = "";
    string str_time;
    long             seconds;
    struct tm      *tmp_time;

    seconds = time(NULL);
    tmp_time = localtime((const time_t *)&seconds);

    sprintf(value_string, "%04d-%02d-%02d %02d:%02d:%02d",
            tmp_time->tm_year + 1900,
            tmp_time->tm_mon + 1,
            tmp_time->tm_mday,
            tmp_time->tm_hour,
            tmp_time->tm_min,
            tmp_time->tm_sec);
    str_time = value_string;

    return str_time;
}



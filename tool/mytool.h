#ifndef  MYTOOL_H_
#define  MYTOOL_H_

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <vector>
#include <time.h>
#include <iterator>

using namespace std;

class MyTool
{
	private:
		
	public:				
		//��intֵת��Ϊstring����
        string IntToStr(int value);
        //��ĸСдת����д
        int Check_Toupper(char *str);
        //��ĸ��дתСд
       int Check_Tolower(char *str);
       //��ȡ��ǰʱ���ַ���
       string GetTimeString();
};

#endif

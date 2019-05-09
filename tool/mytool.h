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
		//将int值转换为string类型
        string IntToStr(int value);
        //字母小写转换大写
        int Check_Toupper(char *str);
        //字母大写转小写
       int Check_Tolower(char *str);
       //获取当前时间字符串
       string GetTimeString();
};

#endif

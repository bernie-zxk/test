//Similar_str.h
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define STR_GBK2312 0


using namespace std;

class Similar_str
{
public:
    string str_a;//第一个字符串
    string str_b;//第二个字符串
    vector <int> v_size;//存放各匹配部分的长度
	//vector <string> v_map_str;
	short CN_LEN;//汉字占得字节数

    //Similar_str(string stra,string strb);
    Similar_str();
    ~Similar_str();

    //计算相似度
    float calc_similar();

    //比较字符串，统计各匹配部分的长度
    float get_similar(string stra, string strb);

	//每个str_a的子字符串，到str_b中找到能匹配的长度，返回匹配长度，变量返回str_b的位置
	int find_map(string sa, int& b_pos);

    //检查首字符是否是中文
        bool isCN(string str); 

	//全角转半角
	int ToDBC_gb(char *input,int len);

}; 

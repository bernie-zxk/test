#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class H_FILE
{
public:
    string h_fname;     //头文件名
    string trans_fname; //要传输的文件名
    string time_str;   //当前时间时标
	string f_dv;	   //区域名
	string f_tail;     //后缀 如  CIME  analog  discrete  mdfalg
    string send_addr;  //发送地址
    vector<string> recv_addr;   //接受地址（可能多个）
    string trans_type;   //传输类型
    string f_context;     //内容(如“增量遥测点表”)
	

/* 
  入参: 传输的文件名、发送地址、接受地址、内容
*/
    H_FILE(string fname, string sendaddr, vector<string> recvaddr, string fcontext,string mailtype);
/*
  无参数构造
*/
	H_FILE();
    ~H_FILE();
/*
  设置参数，与无参数构造函数配合使用
*/
	int set_info(string fname, string sendaddr, vector<string> recvaddr, string fcontext,string mailtype);
/*
  生成头文件
*/
	int creat_hfile(string &path);
/*
  组织头文件名
*/
    string get_hfilename(string &tfile);
/*
    组织时标
*/
	string get_timestr(string &tfile);
/*
    根据文件名，获得区域名
*/
	string get_dvstr(string &tfile);
/*
    根据文件名，获得文件后缀（如 CIME  analog）
*/
	string get_tailstr(string &tfile);

};

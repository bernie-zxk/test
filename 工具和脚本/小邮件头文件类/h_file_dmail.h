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
    string h_fname;     //ͷ�ļ���
    string trans_fname; //Ҫ������ļ���
    string time_str;   //��ǰʱ��ʱ��
	string f_dv;	   //������
	string f_tail;     //��׺ ��  CIME  analog  discrete  mdfalg
    string send_addr;  //���͵�ַ
    vector<string> recv_addr;   //���ܵ�ַ�����ܶ����
    string trans_type;   //��������
    string f_context;     //����(�硰����ң����)
	

/* 
  ���: ������ļ��������͵�ַ�����ܵ�ַ������
*/
    H_FILE(string fname, string sendaddr, vector<string> recvaddr, string fcontext,string mailtype);
/*
  �޲�������
*/
	H_FILE();
    ~H_FILE();
/*
  ���ò��������޲������캯�����ʹ��
*/
	int set_info(string fname, string sendaddr, vector<string> recvaddr, string fcontext,string mailtype);
/*
  ����ͷ�ļ�
*/
	int creat_hfile(string &path);
/*
  ��֯ͷ�ļ���
*/
    string get_hfilename(string &tfile);
/*
    ��֯ʱ��
*/
	string get_timestr(string &tfile);
/*
    �����ļ��������������
*/
	string get_dvstr(string &tfile);
/*
    �����ļ���������ļ���׺���� CIME  analog��
*/
	string get_tailstr(string &tfile);

};

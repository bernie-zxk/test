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
    string str_a;//��һ���ַ���
    string str_b;//�ڶ����ַ���
    vector <int> v_size;//��Ÿ�ƥ�䲿�ֵĳ���
	//vector <string> v_map_str;
	short CN_LEN;//����ռ���ֽ���

    //Similar_str(string stra,string strb);
    Similar_str();
    ~Similar_str();

    //�������ƶ�
    float calc_similar();

    //�Ƚ��ַ�����ͳ�Ƹ�ƥ�䲿�ֵĳ���
    float get_similar(string stra, string strb);

	//ÿ��str_a�����ַ�������str_b���ҵ���ƥ��ĳ��ȣ�����ƥ�䳤�ȣ���������str_b��λ��
	int find_map(string sa, int& b_pos);

    //������ַ��Ƿ�������
        bool isCN(string str); 

	//ȫ��ת���
	int ToDBC_gb(char *input,int len);

}; 

#ifndef  _MYSQL_API_H_
#define  _MYSQL_API_H_

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <vector>
#include <time.h>
#include <iterator>


/**
*
*mysql�� ʹ��c����APL������ȡ����
*
**/

#include <mysql/mysql.h>

using namespace std;


const short C_MYSQL_TYPE_DEFAULT = 0;
const short C_MYSQL_TYPE_STRING = 1;
const short C_MYSQL_TYPE_UCHAR = 2;
const short C_MYSQL_TYPE_SHORT = 3;
const short C_MYSQL_TYPE_INT = 4;
const short C_MYSQL_TYPE_DATETIME = 5;
const short C_MYSQL_TYPE_FLOAT = 6;
const short C_MYSQL_TYPE_DOUBLE = 7;
const short C_MYSQL_TYPE_KEYID = 8;
const short C_MYSQL_TYPE_BINARY = 9;
const short C_MYSQL_TYPE_TEXT = 10;
const short C_MYSQL_TYPE_IMAGE = 11;
const short C_MYSQL_TYPE_APPKEYID = 12;
const short C_MYSQL_TYPE_APPID = 13;
const short C_MYSQL_TYPE_UINT = 14;
const short C_MYSQL_TYPE_LONG = 15;


const int MYSQL_FAIL =-1;
const int MYSQL_SUCCEED =0;
const int MYSQL_MAX_COLS =200;

typedef struct ColAttr{
	unsigned int		data_type;			//�ṹ���Ա����������
	unsigned int		data_size;			//���ݵĴ�С�����ֽڼ���

}ColInfo_Stru;

typedef vector<vector<string> > ResultInfo;


class CMySqlApi
{
private:

	//mysql���� ���
	MYSQL m_mySql;
	//�����
	MYSQL_RES *m_pRes;
    string m_errMsg;
  
	int m_rowNum;
    int m_colNum;


	//�ṹ�����
	int DoAlign(int* x, int b);
	//��¼����������ͳ���
	int GetMaxDataTypeLength(int data_type,int attr_length,int * MaxLength);
	//ʱ���ַ���ת��Ϊlong ����
	long TransStrTimeToLong(const char *timep,const char *format);


public:
	
	//��ʼ��  mysql���
    CMySqlApi();
	//�ͷŽ����
    ~CMySqlApi();

	//����mysql  �� ������ݿ����� dbname
	int DbConnect(const char *host,const char *user_name,const char *user_passwd,const char *dbname);
	//�Ͽ�
	int DbDisconnect();

    //��mysql��������ݵ������
	int ReadData(const char * sql);
	//�ͷŽ����
	void FreeReadData();
    //���������vector<vector<string> >��
	int GetResult(ResultInfo & result_vec);
	//����������ݶ������ṹ����
	int GetAlignResult(void* align_result_ptr,struct ColAttr * attrs_info, const int struct_length);
//    int &RowNum(){return m_rowNum;}
//    int &ColNum(){return m_colNum;}
//    string &ErrMsg(){return m_errMsg;}
    
    int ExecTransCommit(const vector<string> &cmdbuf);
    int ExecSingleCommit(const string &sql_str);
    int ExecSingleCommit(const char *sql_char);
    
};







#endif


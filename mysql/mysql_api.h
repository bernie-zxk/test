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
*mysql： 使用c语言APL函数读取数据
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
	unsigned int		data_type;			//结构体成员的数据类型
	unsigned int		data_size;			//数据的大小，以字节计算

}ColInfo_Stru;

typedef vector<vector<string> > ResultInfo;


class CMySqlApi
{
private:

	//mysql连接 句柄
	MYSQL m_mySql;
	//结果集
	MYSQL_RES *m_pRes;
    string m_errMsg;
  
	int m_rowNum;
    int m_colNum;


	//结构体对齐
	int DoAlign(int* x, int b);
	//记录最大数据类型长度
	int GetMaxDataTypeLength(int data_type,int attr_length,int * MaxLength);
	//时间字符串转换为long 数据
	long TransStrTimeToLong(const char *timep,const char *format);


public:
	
	//初始化  mysql句柄
    CMySqlApi();
	//释放结果集
    ~CMySqlApi();

	//连接mysql  库 添加数据库名称 dbname
	int DbConnect(const char *host,const char *user_name,const char *user_passwd,const char *dbname);
	//断库
	int DbDisconnect();

    //从mysql库读出数据到结果集
	int ReadData(const char * sql);
	//释放结果集
	void FreeReadData();
    //将结果放入vector<vector<string> >中
	int GetResult(ResultInfo & result_vec);
	//将结果集数据对齐放入结构体中
	int GetAlignResult(void* align_result_ptr,struct ColAttr * attrs_info, const int struct_length);
//    int &RowNum(){return m_rowNum;}
//    int &ColNum(){return m_colNum;}
//    string &ErrMsg(){return m_errMsg;}
    
    int ExecTransCommit(const vector<string> &cmdbuf);
    int ExecSingleCommit(const string &sql_str);
    int ExecSingleCommit(const char *sql_char);
    
};







#endif


#ifndef   __SQL_CHANGE_H__
#define   __SQL_CHANGE_H__
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <cstring>
#include <vector>
#include <map>
#include <stack>
#include <iostream>
#include  "db_com/sql_syntax_anls.h"
#include "db_com/lexical_anls.h"

#include "common.h"


using namespace std;
using namespace SQL_ANALYSER;

#define MAX_TABLE_NAME_ENG_LEN          32
#define MAX_TABLE_NAME_CHN_LEN          64
#define MAX_COLUMN_NAME_ENG_LEN         32
#define MAX_COLUMN_NAME_CHN_LEN         64
const short C_DATATYPE_DEFAULT = 0;


typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;

const int     MAX_DB_PARA_NAME  = 64;
//----------------------------------------

class CMySqlApi   g_db_dci;


typedef struct LoginInfoStru
{
    char  host[MAX_DB_PARA_NAME];
    char  user_name[MAX_DB_PARA_NAME];
    char  user_passwd[MAX_DB_PARA_NAME];
	char  dbname[MAX_DB_PARA_NAME];

} TLoginInfo;
//-------------------------------

typedef vector<string> StrSeq;

struct TPreAnalysInfo
{
    int         sql_type;
    char        table_name[MAX_TABLE_NAME_ENG_LEN];
    string      err_msg;
};


struct  TRequestInfo
{
    bool            err_flag;
    int             sql_type;
    short           col_info_index;
    string          err_msg;
};
typedef vector<TRequestInfo> VRequestInfo;

// ��������޸�ʱ�����Ϣ
struct  TColumnInfo
{
    short           column_id;
    char            column_name[MAX_COLUMN_NAME_ENG_LEN];
    char            column_name_chn[MAX_COLUMN_NAME_CHN_LEN];
    uchar           column_type;
    short           column_length;
    uchar           is_key;
//    uchar           allow_null;
};
typedef vector<TColumnInfo> VColumnInfo;


typedef vector<TAnalysColumnInfo>       VECAnalysColumnInfo;

struct  TTableColumnInfo
{
    int                     table_no;
    string                  table_name;
    VColumnInfo         all_col_vec;
    VECAnalysColumnInfo     anls_col_vec;
};

// ���ȫ����Ϣ.
struct  TTableInfo   
{
        int     table_id;
        char    table_name_eng[MAX_TABLE_NAME_ENG_LEN];
        uchar   key_generate_type;//������������
		int     reserved_4;//�Ƿ�ģ�ͷ���
};


//ÿ��SQL�������
struct OneSqlResult
{
    string sql;
    TSqlAnlsResult sqlAnlsResult;
};
typedef vector<OneSqlResult> VOneSqlResult;

//ÿ��SQL�������
struct ClassifyResult
{
	TTableInfo      table_info;
	TTableColumnInfo table_column_info;
    VOneSqlResult VOnesqlResult;

};
typedef vector<ClassifyResult> VClassifyResult;

//ȫ��SQL�������
struct AnlsResult
{
  VClassifyResult Insert;
  VClassifyResult Update;
  vector<string> Delete_vec;

};



class CSqlTool
{
public:
    CSqlTool();
    ~CSqlTool();
//	void PrintExportResult(const char * sql_str, const TSqlAnlsResult & sql_anls_result);
//	void printfResult(const TSqlAnlsResult & sql_anls_result);
    int SqlProcess(const StrSeq& sql_request);

private:
	map<long,long> id_relation;
   //�õ�ÿ���������Ϣ�ṹ
	bool GetTableColInfo(vector<string>& sql_vec,
						    vector<TRequestInfo>& tableRequestVec,
					   	    vector<TTableColumnInfo>& anlsColInfoVec);

    // SQLԤ�������̣��õ�ÿ��SQL�ı��������͡�
    bool PreAnalysProcess(string& sql_str,TPreAnalysInfo& preAnalysInfo);
    // ���ݱ��������ñ�ı���Լ���Ķ�����Ϣ
//    bool GetTableColInfoByTableName(const char * table_name, TTableColumnInfo& table_column_info);

//	string GetValueByConstInfo(const short data_type, const TConstInfo & const_info);
//	bool AllSqlAnalys(vector<string>& sql_vec,
//								vector<TRequestInfo>& tableRequestVec,
//								vector<TTableColumnInfo>& anlsColInfoVec,AnlsResult& anlsResult);
//	bool AnalysProcess(string& sql_str,
//								 VECAnalysColumnInfo&  anls_col_vec,
//								 TSqlAnlsResult&  sqlAnlsResult);
//	int StructVectorClear(TSqlAnlsResult& sqlAnlsResult);
//	bool GetTableInfo(int table_no, TTableInfo & table_all_info);
//	
//   bool AllIdChange(VClassifyResult& InsertResult);
//   bool TableIdChange(ClassifyResult& tableResult);
//   long GetRegionId(TSqlAnlsResult&  sqlAnlsResult);
//   bool IS_Key(const char* column_name,TTableColumnInfo& table_column_info);
//   long GetMaxKeyID(char * column_name, char * table_name, long table_no,long& last_max_key_id);
//   long GetMaxKeyIDInRegion(const long region_id_num,
//                                   const char* key_column_name,
//                                   const long table_no,
//                                   const char* table_name,
//                                   const TSqlAnlsResult&  sqlAnlsResult,
//                                   long &last_region_id_num,long &last_max_key_id);

//   bool AllMeasChange(AnlsResult& anlsResult);
//   bool InsertTableRecord(ClassifyResult& tableResult);
//   bool UpdateTableRecord(ClassifyResult& tableResult);
//   bool PopFromStack(stack<string> & where_stack, string & str);
//   void PushIntoStack(stack<string> & where_stack, string & str);
//   bool GetSqlFormatByConstInfo(const TConstInfo & const_info, string & const_string);
//   bool GetSqlFormatByOperator(const int operator_code, const string operand_1,
//                                      const string operand_2, string & expression);
//   bool GetWhereExpressionByAnlsResult(string & where_str,
//        TSqlAnlsResult& sqlAnlsResult);

//   








};

#endif


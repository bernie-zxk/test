#ifndef EXPORT_TRIGGER_H_
#define EXPORT_TRIGGER_H_
#include "common.h"



#define  PATH_LEN (256)

#define  EXPORT_FAIL (-1)
#define  EXPORT_SUCCEED (0)



const string  EXPORT_PATH = "/conf/check_trigger_txt/";
const string  CONF_PATH = "/conf/";
const string  EXPORT_NAME_TXT = "export_trigger_txt";
string g_home_path;
string g_export_path;




char  CHECK_TRI_CONF[] = "check_trigger.conf";

class CDci g_CDci;
string g_err_str;//比对触发器、函数、存储过程的过程中遇到的错误信息
string g_strHomePath;//家路径地址

STRU_VECTOR_INFO  g_stru_vec_only_db;  //仅在关系库存在的触发器、函数、存储过程
STRU_VECTOR_INFO  g_stru_vec_only_conf;//仅在标准库中存在的触发器、函数、存储过程
STRU_VECTOR_INFO  g_stru_vec_diff_name;//比对结果不相同的触发器、函数、存储过程
STRU_VECTOR_INFO  g_stru_vec_err_name; //比对过程中出错的触发器、函数、存储过程




//创建目录
int CreateCatalog(string path,string name);
//写导出文本
int WriteExportText(string &catalog_path,string &name,string &buff);
//创建导出目录
int CreateExportCatalog(string &export_path);
//导出
int ExportAllTrigger(int model);
//基于配置文件导出
int ExportByConf();
//将所有触发器、函数、存储过程写入导出文件
int WriteTextByStru(STRU_VECTOR_INFO &stru_vec_name);
//基于数据库导出并自动生成配置文件
int ExportAddConf();
//基于数据库写配置文件
int WriteConf(STRU_VECTOR_INFO &str_vec_name);
int PrintfErrInfo();



#endif



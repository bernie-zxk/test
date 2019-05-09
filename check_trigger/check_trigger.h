#ifndef CHECK_TRIGGER_H_
#define CHECK_TRIGGER_H_
#include "common.h"

char  CHECK_TRI_CONF[] = "check_trigger.conf";


class CDci g_CDci;
string g_err_str;//比对触发器、函数、存储过程的过程中遇到的错误信息
string g_strHomePath;//家路径地址

STRU_VECTOR_INFO  g_stru_vec_only_db;  //仅在关系库存在的触发器、函数、存储过程
STRU_VECTOR_INFO  g_stru_vec_only_conf;//仅在标准库中存在的触发器、函数、存储过程
STRU_VECTOR_INFO  g_stru_vec_diff_name;//比对结果不相同的触发器、函数、存储过程
STRU_VECTOR_INFO  g_stru_vec_err_name; //比对过程中出错的触发器、函数、存储过程





#endif

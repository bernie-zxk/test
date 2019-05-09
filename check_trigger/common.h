#ifndef COMMON_H_
#define COMMON_H_
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "services.h"
#include "serviceglobal.h"

#include "paramanage.h"
#include "dcisg.h"

#define  MAX_PARA_NAME (128)
#define  NAME_LEN (256)

#define  CHECK_FAIL (-1)
#define  CHECK_SUCCEED (0)
#define  CHECK_NOEXIST (-2) //记录不存在
#define  CHECK_MORE  (-3)  //记录过多
#define  CHECK_EQ     (1)  //比对结果相等
#define  CHECK_NOEQ   (2)  //比对结构不相等
//比对分为错误和警告两种提示
//flag 为1表示为触发器错误比对2表示触发器警告比对3 表示函数错误比对4表示函数警告比对
//5 表示存储过程错误比对  6表示存储过程警告比对
#define FLAG_TRI_ERR    (1)
#define FLAG_FUNC_ERR   (2)
#define FLAG_PROC_ERR   (3)
#define FLAG_TRI_WARN   (4)
#define FLAG_FUNC_WARN  (5)
#define FLAG_PROC_WARN  (6)

//打印错误信息时,用来区分是否打印仅存在关系库中的
//触发器、函数、存储过程,如果是,
//则仅存在关系库中的函数和存储过程是放在一起打印的
#define PRINT_OTHER     (0)
#define PRINT_ONLY_DB   (1)

#define     HOME_ENV_NAME	   "D5000_HOME"

const string  PUBLIC_TRI_PATH = "/conf/check_trigger_txt/trigger/";
const string  PUBLIC_FUNC_PATH = "/conf/check_trigger_txt/function/";
const string  PUBLIC_PROC_PATH = "/conf/check_trigger_txt/procedure/";
const string  PUBLIC_TRI_WARN_PATH = "/conf/check_trigger_txt/trigger_warn/";
const string  PUBLIC_FUNC_WARN_PATH = "/conf/check_trigger_txt/function_warn/";
const string  PUBLIC_PROC_WARN_PATH = "/conf/check_trigger_txt/procedure_warn/";


extern char  CHECK_TRI_CONF[];
#define OUT
#define IN
#define ALTER

#define  DATA_FLAG  (2) //
//ReadDataEx查询结果:
//2个普通属性一个大文本:
//属性数据||2字节标识数据是否为空||属性数据||2字节标识||8字节空(返回的属性信息中属性长度)||2字节标识||大文本数据
//1个普通2个大文本:
//属性数据||2字节标识||8字节空(返回的属性信息中属性长度)||2字节标识||8字节空||2字节标识||大文本数据||大本文数据
//一个大文本
//8字节空||2字节标识||大文本数据

//当查询结果中有多个属性只有一个大文本时计算偏移的方法
//当前属性的下一个属性为大文本
//a为当前属性偏移,b为当前属性长度,2字节为标识,
//c为下一大文本属性长度(属性描述信息中的长度一般为8),2字节为标识
//#define  ONE_TXT_OFFSET(a,b,c)  (a+b+2+c+2) 

//查询结果仅仅只有一个大文本时的计算便宜算法
//a为大文本属性长度(属性描述信息中的长度一般为8),2字节为标识
#define  ONE_TXT_OFFSET(a)  ((a)+2) 

//ReadData 查询结果:
//2个普通属性
//第一个属性数据||第二个属性数据
//调用ReadData接口  读取一个触发器名字时一个属性数据占用空间的长度算法
//a为属性的长度 
#define  ONE_ATTR_OFFSET(a)  (a)



typedef struct
{
	vector<string> vec_trigger_name;
    vector<string> vec_func_name;
    vector<string> vec_proc_name;
    vector<string> vec_trigger_name_warn;
    vector<string> vec_func_name_warn;
    vector<string> vec_proc_name_warn;
}STRU_VECTOR_INFO;

static	const	char  *flagdes[7] = {
"一切正常",   		
"触发器",   		//FLAG_TRI_ERR      (1)
"函数", 			//FLAG_FUN_ERR      (2)
"存储过程", 		//FLAG_PROC_ERR   (3)
"警告触发器",   	//FLAG_TRI_WARN   (4)
"警告函数",			//FLAG_FUN_WARN   (5)
"警告存储过程"		//FLAG_PROC_WARN  (6)
};


extern class CDci g_CDci;
extern string g_err_str;//比对触发器、函数、存储过程的过程中遇到的错误信息
extern string g_strHomePath;//家路径地址

extern STRU_VECTOR_INFO  g_stru_vec_only_db;  //仅在关系库存在的触发器、函数、存储过程
extern STRU_VECTOR_INFO  g_stru_vec_only_conf;//仅在标准库中存在的触发器、函数、存储过程
extern STRU_VECTOR_INFO  g_stru_vec_diff_name;//比对结果不相同的触发器、函数、存储过程
extern STRU_VECTOR_INFO  g_stru_vec_err_name; //比对过程中出错的触发器、函数、存储过程



//数值转字符
string IntToStr(int value);
//字母小写转换大写
int Check_Toupper(ALTER char *str);
//字母大写转换小写
int Check_Tolower(ALTER char *str);
//自定义结构体清空
int StructVectorClear(ALTER STRU_VECTOR_INFO &stru);
//定义的vector结构体的拷贝
int StructVectorCpy(OUT STRU_VECTOR_INFO &dest, IN STRU_VECTOR_INFO &src);
//关系库资源释放
int Check_FreeDb(IN ColAttr *attr,IN int attr_num, IN char *data_buf);
//从关系库获取名字
int GetNameFromDb(char *sql,vector<string> &vec_name);
//从关系库获取关系库中触发器、函数、存储过程名字
int GetAllTriggerNameFromDb(STRU_VECTOR_INFO &stru_vec_name_db);
//从当前系统关系库中获取指定的触发器或函数或存储过程等代码
//flag 为1、4表示为触发器  2、5  表示函数  3、6 表示存储过程
int GetTriggerTextFromDb(IN int flag,IN string &name, OUT string &db_tri_string);
//从当前目录下获取指定的触发器或存储过程或函数等代码(标准逻辑代码)
//flag 为1、4表示为触发器  2、5  表示函数  3、6 表示存储过程
int GetTriggerTextFromFile(IN int flag,IN string &name, OUT string &file_tri_string);
//去掉最后一个分号   原因触发器等代码最后加不加;号都可以编译成功
//为了避免最后一个;号不匹配的问题则统一去掉
int RmLastSemicolon(string &dest_str);
//删除string中的指定字符
int RmChar(string &dest_str, char del_c);
//去掉获得的代码中的空格、回车、TAB等等  ' ' '\t'   '\n'   '\r'
int RmSpaceAndTab(string &tri_string);
//去掉 注释行
int RmComment(string &tri_string);
//读取配置文件check_trigger.conf 指定内容
int GetConfigInfo(IN string &section_name,IN string &prefix_rep_name, OUT vector<string> &vec_name);
//读取配置文件check_trigger.conf所有内容
int GetAllConfigInfo(OUT STRU_VECTOR_INFO &stru_vec_name_conf);
//按标识flag存储名称到目的结构体中
int StorageStructMember(STRU_VECTOR_INFO &dest_vector_info,string &name,int flag);
//获取成员数量
int CountStruMemberNum(STRU_VECTOR_INFO &stru_vec_name);





#endif


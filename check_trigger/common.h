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
#define  CHECK_NOEXIST (-2) //��¼������
#define  CHECK_MORE  (-3)  //��¼����
#define  CHECK_EQ     (1)  //�ȶԽ�����
#define  CHECK_NOEQ   (2)  //�ȶԽṹ�����
//�ȶԷ�Ϊ����;���������ʾ
//flag Ϊ1��ʾΪ����������ȶ�2��ʾ����������ȶ�3 ��ʾ��������ȶ�4��ʾ��������ȶ�
//5 ��ʾ�洢���̴���ȶ�  6��ʾ�洢���̾���ȶ�
#define FLAG_TRI_ERR    (1)
#define FLAG_FUNC_ERR   (2)
#define FLAG_PROC_ERR   (3)
#define FLAG_TRI_WARN   (4)
#define FLAG_FUNC_WARN  (5)
#define FLAG_PROC_WARN  (6)

//��ӡ������Ϣʱ,���������Ƿ��ӡ�����ڹ�ϵ���е�
//���������������洢����,�����,
//������ڹ�ϵ���еĺ����ʹ洢�����Ƿ���һ���ӡ��
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
//ReadDataEx��ѯ���:
//2����ͨ����һ�����ı�:
//��������||2�ֽڱ�ʶ�����Ƿ�Ϊ��||��������||2�ֽڱ�ʶ||8�ֽڿ�(���ص�������Ϣ�����Գ���)||2�ֽڱ�ʶ||���ı�����
//1����ͨ2�����ı�:
//��������||2�ֽڱ�ʶ||8�ֽڿ�(���ص�������Ϣ�����Գ���)||2�ֽڱ�ʶ||8�ֽڿ�||2�ֽڱ�ʶ||���ı�����||��������
//һ�����ı�
//8�ֽڿ�||2�ֽڱ�ʶ||���ı�����

//����ѯ������ж������ֻ��һ�����ı�ʱ����ƫ�Ƶķ���
//��ǰ���Ե���һ������Ϊ���ı�
//aΪ��ǰ����ƫ��,bΪ��ǰ���Գ���,2�ֽ�Ϊ��ʶ,
//cΪ��һ���ı����Գ���(����������Ϣ�еĳ���һ��Ϊ8),2�ֽ�Ϊ��ʶ
//#define  ONE_TXT_OFFSET(a,b,c)  (a+b+2+c+2) 

//��ѯ�������ֻ��һ�����ı�ʱ�ļ�������㷨
//aΪ���ı����Գ���(����������Ϣ�еĳ���һ��Ϊ8),2�ֽ�Ϊ��ʶ
#define  ONE_TXT_OFFSET(a)  ((a)+2) 

//ReadData ��ѯ���:
//2����ͨ����
//��һ����������||�ڶ�����������
//����ReadData�ӿ�  ��ȡһ������������ʱһ����������ռ�ÿռ�ĳ����㷨
//aΪ���Եĳ��� 
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
"һ������",   		
"������",   		//FLAG_TRI_ERR      (1)
"����", 			//FLAG_FUN_ERR      (2)
"�洢����", 		//FLAG_PROC_ERR   (3)
"���津����",   	//FLAG_TRI_WARN   (4)
"���溯��",			//FLAG_FUN_WARN   (5)
"����洢����"		//FLAG_PROC_WARN  (6)
};


extern class CDci g_CDci;
extern string g_err_str;//�ȶԴ��������������洢���̵Ĺ����������Ĵ�����Ϣ
extern string g_strHomePath;//��·����ַ

extern STRU_VECTOR_INFO  g_stru_vec_only_db;  //���ڹ�ϵ����ڵĴ��������������洢����
extern STRU_VECTOR_INFO  g_stru_vec_only_conf;//���ڱ�׼���д��ڵĴ��������������洢����
extern STRU_VECTOR_INFO  g_stru_vec_diff_name;//�ȶԽ������ͬ�Ĵ��������������洢����
extern STRU_VECTOR_INFO  g_stru_vec_err_name; //�ȶԹ����г���Ĵ��������������洢����



//��ֵת�ַ�
string IntToStr(int value);
//��ĸСдת����д
int Check_Toupper(ALTER char *str);
//��ĸ��дת��Сд
int Check_Tolower(ALTER char *str);
//�Զ���ṹ�����
int StructVectorClear(ALTER STRU_VECTOR_INFO &stru);
//�����vector�ṹ��Ŀ���
int StructVectorCpy(OUT STRU_VECTOR_INFO &dest, IN STRU_VECTOR_INFO &src);
//��ϵ����Դ�ͷ�
int Check_FreeDb(IN ColAttr *attr,IN int attr_num, IN char *data_buf);
//�ӹ�ϵ���ȡ����
int GetNameFromDb(char *sql,vector<string> &vec_name);
//�ӹ�ϵ���ȡ��ϵ���д��������������洢��������
int GetAllTriggerNameFromDb(STRU_VECTOR_INFO &stru_vec_name_db);
//�ӵ�ǰϵͳ��ϵ���л�ȡָ���Ĵ�����������洢���̵ȴ���
//flag Ϊ1��4��ʾΪ������  2��5  ��ʾ����  3��6 ��ʾ�洢����
int GetTriggerTextFromDb(IN int flag,IN string &name, OUT string &db_tri_string);
//�ӵ�ǰĿ¼�»�ȡָ���Ĵ�������洢���̻����ȴ���(��׼�߼�����)
//flag Ϊ1��4��ʾΪ������  2��5  ��ʾ����  3��6 ��ʾ�洢����
int GetTriggerTextFromFile(IN int flag,IN string &name, OUT string &file_tri_string);
//ȥ�����һ���ֺ�   ԭ�򴥷����ȴ������Ӳ���;�Ŷ����Ա���ɹ�
//Ϊ�˱������һ��;�Ų�ƥ���������ͳһȥ��
int RmLastSemicolon(string &dest_str);
//ɾ��string�е�ָ���ַ�
int RmChar(string &dest_str, char del_c);
//ȥ����õĴ����еĿո񡢻س���TAB�ȵ�  ' ' '\t'   '\n'   '\r'
int RmSpaceAndTab(string &tri_string);
//ȥ�� ע����
int RmComment(string &tri_string);
//��ȡ�����ļ�check_trigger.conf ָ������
int GetConfigInfo(IN string &section_name,IN string &prefix_rep_name, OUT vector<string> &vec_name);
//��ȡ�����ļ�check_trigger.conf��������
int GetAllConfigInfo(OUT STRU_VECTOR_INFO &stru_vec_name_conf);
//����ʶflag�洢���Ƶ�Ŀ�Ľṹ����
int StorageStructMember(STRU_VECTOR_INFO &dest_vector_info,string &name,int flag);
//��ȡ��Ա����
int CountStruMemberNum(STRU_VECTOR_INFO &stru_vec_name);





#endif


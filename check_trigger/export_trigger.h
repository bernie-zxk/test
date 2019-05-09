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
string g_err_str;//�ȶԴ��������������洢���̵Ĺ����������Ĵ�����Ϣ
string g_strHomePath;//��·����ַ

STRU_VECTOR_INFO  g_stru_vec_only_db;  //���ڹ�ϵ����ڵĴ��������������洢����
STRU_VECTOR_INFO  g_stru_vec_only_conf;//���ڱ�׼���д��ڵĴ��������������洢����
STRU_VECTOR_INFO  g_stru_vec_diff_name;//�ȶԽ������ͬ�Ĵ��������������洢����
STRU_VECTOR_INFO  g_stru_vec_err_name; //�ȶԹ����г���Ĵ��������������洢����




//����Ŀ¼
int CreateCatalog(string path,string name);
//д�����ı�
int WriteExportText(string &catalog_path,string &name,string &buff);
//��������Ŀ¼
int CreateExportCatalog(string &export_path);
//����
int ExportAllTrigger(int model);
//���������ļ�����
int ExportByConf();
//�����д��������������洢����д�뵼���ļ�
int WriteTextByStru(STRU_VECTOR_INFO &stru_vec_name);
//�������ݿ⵼�����Զ����������ļ�
int ExportAddConf();
//�������ݿ�д�����ļ�
int WriteConf(STRU_VECTOR_INFO &str_vec_name);
int PrintfErrInfo();



#endif



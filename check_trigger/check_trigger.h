#ifndef CHECK_TRIGGER_H_
#define CHECK_TRIGGER_H_
#include "common.h"

char  CHECK_TRI_CONF[] = "check_trigger.conf";


class CDci g_CDci;
string g_err_str;//�ȶԴ��������������洢���̵Ĺ����������Ĵ�����Ϣ
string g_strHomePath;//��·����ַ

STRU_VECTOR_INFO  g_stru_vec_only_db;  //���ڹ�ϵ����ڵĴ��������������洢����
STRU_VECTOR_INFO  g_stru_vec_only_conf;//���ڱ�׼���д��ڵĴ��������������洢����
STRU_VECTOR_INFO  g_stru_vec_diff_name;//�ȶԽ������ͬ�Ĵ��������������洢����
STRU_VECTOR_INFO  g_stru_vec_err_name; //�ȶԹ����г���Ĵ��������������洢����





#endif

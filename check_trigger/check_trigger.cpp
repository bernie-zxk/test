#include "check_trigger.h"


//�Զ���ṹ�嵥����Ա�ȶ�
int StructMemberCmp(ALTER vector <string> &vec_conf,ALTER vector <string> &vec_db,OUT vector <string> &vec_com,OUT vector <string> &vec_err);
//�Զ���ṹ��ȶ�
int StructVectorCmp(IN STRU_VECTOR_INFO &stu_vec_name_conf,IN STRU_VECTOR_INFO &stu_vec_name_db,OUT STRU_VECTOR_INFO &stu_vec_name_com);
//���ִ���Σ�����������   һ����ͷ   һ���ֳ����߼�   ��BEGINΪ�ָ�
int AnalyseTri(IN string name,IN int flag,OUT string &head_db_string,ALTER string &db_string,OUT string &head_file_string,ALTER string &file_string);
//�ȶԹ�ϵ��ͱ�׼���еĴ�����
int CmpDbAndFileTri(IN string name,IN int flag,IN string &db_tri_string,IN string file_tri_string);
//�ȶ�ָ�������������߼�
//flag Ϊ1��ʾΪ����������ȶ�4��ʾ����������ȶ�2 ��ʾ��������ȶ�5��ʾ��������ȶ�
// 3 ��ʾ�洢���̴���ȶ�  6��ʾ�洢���̾���ȶ�
int CmpOneTrigger(IN string name,IN int flag);
//�ȶԶ��������������洢����
//flag Ϊ1��ʾΪ����������ȶ�4��ʾ����������ȶ�2 ��ʾ��������ȶ�5��ʾ��������ȶ�
// 3 ��ʾ�洢���̴���ȶ�  6��ʾ�洢���̾���ȶ�
int CmpPartTrigger(IN vector<string> &vec_name, IN int flag);
//�ȶ�ȫ��������
int CmpAllTrigger();
//��ӡ�ṹ���Ա
int PrintfStruct(STRU_VECTOR_INFO &stru_vec_name,int flag = PRINT_OTHER);
//��ӡ������Ϣ
int PrintfErrInfo();


//�ṹ���е�����Ա�ıȶ�     vec_com��¼��ϵ��ͱ�׼���ﶼ���ڵĴ�������   
//vec_err��¼�����ڱ�׼��Ĵ�������
int StructMemberCmp(ALTER vector <string> &vec_conf,ALTER vector <string> &vec_db,OUT vector <string> &vec_com,OUT vector <string> &vec_err)
{
    int i,j;
    int  findflag=0;
    for(i=0;i<vec_conf.size();i++)
    {
        findflag=0;
        for(j=0;j<vec_db.size();j++)
        {
            if(vec_conf[i]==vec_db[j])
            {
                vec_com.push_back(vec_conf[i]);
                vec_db.erase(vec_db.begin()+j);
                findflag=1;
                break;
            }
        }
        //���û�ҵ���ô��������������洢���̽��ڱ�׼�����
        if(findflag!=1)
        {
            vec_err.push_back(vec_conf[i]);
        }
    }
    return CHECK_SUCCEED;
}

//�ṹ��ȶ�
int StructVectorCmp(IN STRU_VECTOR_INFO &stu_vec_name_conf,IN STRU_VECTOR_INFO &stu_vec_name_db,OUT STRU_VECTOR_INFO &stu_vec_name_com)
{
    //��ϵ���еĴ�����������vec_name_db��vec_trigger_name��Ա��
    //��ϵ���еĴ洢���̺ͺ���������vec_name_db��vec_func_name��Ա��
    STRU_VECTOR_INFO tmp_stru_vec_name_conf;
    STRU_VECTOR_INFO tmp_stru_vec_name_db;
    StructVectorClear(tmp_stru_vec_name_conf);
    StructVectorClear(tmp_stru_vec_name_db);
    
    StructVectorCpy(tmp_stru_vec_name_conf,stu_vec_name_conf);
    StructVectorCpy(tmp_stru_vec_name_db,stu_vec_name_db);

    //�ȶԴ�����   ��¼��ϵ��ͱ�׼�ﶼ���ڵĴ����������ڱ�׼����ڵĴ�����
    StructMemberCmp(tmp_stru_vec_name_conf.vec_trigger_name,tmp_stru_vec_name_db.vec_trigger_name,
                        stu_vec_name_com.vec_trigger_name,g_stru_vec_only_conf.vec_trigger_name);
    
    //�ȶԺ���
    StructMemberCmp(tmp_stru_vec_name_conf.vec_func_name,tmp_stru_vec_name_db.vec_func_name,
                        stu_vec_name_com.vec_func_name,g_stru_vec_only_conf.vec_func_name);
    
    //�ȶԴ洢����
    StructMemberCmp(tmp_stru_vec_name_conf.vec_proc_name,tmp_stru_vec_name_db.vec_func_name,
                        stu_vec_name_com.vec_proc_name,g_stru_vec_only_conf.vec_proc_name);
    
    //�ȶԾ��津����
    StructMemberCmp(tmp_stru_vec_name_conf.vec_trigger_name_warn,tmp_stru_vec_name_db.vec_trigger_name,
                        stu_vec_name_com.vec_trigger_name_warn,g_stru_vec_only_conf.vec_trigger_name_warn);

    //���溯��
    StructMemberCmp(tmp_stru_vec_name_conf.vec_func_name_warn,tmp_stru_vec_name_db.vec_func_name,
                        stu_vec_name_com.vec_func_name_warn,g_stru_vec_only_conf.vec_func_name_warn);
    
    //����洢����
    StructMemberCmp(tmp_stru_vec_name_conf.vec_proc_name_warn,tmp_stru_vec_name_db.vec_func_name,
                        stu_vec_name_com.vec_proc_name_warn,g_stru_vec_only_conf.vec_proc_name_warn);
    //��ȡ���ڹ�ϵ����ڵĴ��������������洢����
    StructVectorCpy(g_stru_vec_only_db,tmp_stru_vec_name_db);
    
    return CHECK_SUCCEED;
}

//���ִ���Σ�����������   һ����ͷ   һ���ֳ����߼�   ��BEGINΪ�ָ�
int AnalyseTri(IN string name,IN int flag,OUT string &head_db_string,ALTER string &db_string,OUT string &head_file_string,ALTER string &file_string)
{
    char err_char[1024];
    int pos=0;
    int len=0;
    head_db_string = db_string;
    head_file_string = file_string;
    //��ϵ���еĴ��봦��
    //ȥ��ͷ
    //�ҵ��ָ�λ�� begin
    pos = db_string.find("begin",0);
    if(pos == db_string.npos)
    {
        sprintf(err_char,"%s:%s  ����AnalyseTri����ʱ�Թ�ϵ����%s������зָ�ʱ��δ�ҵ�'begin'�ָ�λ��!!!\n",\
                    flagdes[flag],name.c_str(),name.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        return CHECK_FAIL;
    }
    //begin  �ָ�λ��ǰ�ַ����ĳ���
    len = pos;
    db_string.erase(0,len);

    
    //ȥ��β
    pos=0;
    len=0;
    //�ҵ��ָ�λ�� begin
    pos = head_db_string.find("begin",0);
    if(pos == head_db_string.npos)
    {
        sprintf(err_char,"%s:%s  ����AnalyseTri����ʱ�Թ�ϵ����%s������зָ�ʱ��δ�ҵ�'begin'�ָ�λ��!!!\n",\
                    flagdes[flag],name.c_str(),name.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        return CHECK_FAIL;
    }
    //begin  �ָ�λ�ú���ַ���ȫ��ɾ��
    head_db_string.erase(pos,head_db_string.size());

    //��׼���еĴ��봦��
       //ȥ��ͷ
        //�ҵ��ָ�λ�� begin
    pos = file_string.find("begin",0);
    if(pos == file_string.npos)
    {
        sprintf(err_char,"%s:%s  ����AnalyseTri����ʱ�Ա�׼����%s������зָ�ʱ��δ�ҵ�'begin'�ָ�λ��!!!\n",\
                    flagdes[flag],name.c_str(),name.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        return CHECK_FAIL;
    }
        //begin  �ָ�λ��ǰ�ַ����ĳ���
    len = pos;
    file_string.erase(0,len);
           
      //ȥ��β
    pos=0;
    len=0;
      //�ҵ��ָ�λ�� begin
    pos = head_file_string.find("begin",0);
    if(pos == head_file_string.npos)
    {
        sprintf(err_char,"%s:%s  ����AnalyseTri����ʱ�Թ�ϵ����%s������зָ�ʱ��δ�ҵ�'begin'�ָ�λ��!!!\n",\
                    flagdes[flag],name.c_str(),name.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        return CHECK_FAIL;
    }
      //begin  �ָ�λ�ú���ַ���ȫ��ɾ��
    head_file_string.erase(pos,head_file_string.size());
    
    return CHECK_SUCCEED;
}
//�ȶԹ�ϵ��ͱ�׼���еĴ�����
int CmpDbAndFileTri(IN string name,IN int flag,IN string &db_tri_string,IN string file_tri_string)
{
    char err_char[1024];
    string head_db_tri_string;
    string head_file_tri_string;
    string tmp_db_tri_string = db_tri_string;
    string tmp_file_tri_string = file_tri_string;
    int ret;
    
    //ȥ�������ֵ�ע����
    RmComment(tmp_db_tri_string);
    RmComment(tmp_file_tri_string);
    
   // printf("��ϵ��  ȥ��ע�� len=%d, txt=%s\n",tmp_db_tri_string.size(),tmp_db_tri_string.c_str());
    
   // printf("��׼��  ȥ��ע�� len=%d, txt=%s\n",tmp_file_tri_string.size(),tmp_file_tri_string.c_str());

    //���ַ�ת����Сд
    //��ϵ��
    char *data_char=NULL;
    data_char =(char *)malloc(sizeof(char) * tmp_db_tri_string.size()+1);
    if(data_char==NULL)
    {
        sprintf(err_char,"%s:%s  ����CmpDbAndFileTri����ʱ�Թ�ϵ����%s�������Сдת��ʱ,����ռ�ʧ��!!!\n",\
                    flagdes[flag],name.c_str(),name.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        return CHECK_FAIL;
    }
    memset(data_char,0,(sizeof(char) * tmp_db_tri_string.size()+1));
    strcpy(data_char,tmp_db_tri_string.c_str());
    Check_Tolower(data_char);
    tmp_db_tri_string = data_char;

    if(data_char !=NULL)
    {
        free(data_char);
        data_char = NULL;
    }

    //��׼��
    data_char =(char *)malloc(sizeof(char) * tmp_file_tri_string.size()+1);
    if(data_char==NULL)
    {
        sprintf(err_char,"%s:%s  ����CmpDbAndFileTri����ʱ�Ա�׼��%s�������Сдת��ʱ,����ռ�ʧ��!!!\n",\
                    flagdes[flag],name.c_str(),name.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        return CHECK_FAIL;
    }
    memset(data_char,0,(sizeof(char) * tmp_file_tri_string.size()+1));
    strcpy(data_char,tmp_file_tri_string.c_str());
    Check_Tolower(data_char);
    tmp_file_tri_string = data_char;

    if(data_char !=NULL)
    {
        free(data_char);
        data_char = NULL;
    }
   // printf("��ϵ��  ת��Сд len=%d, txt=%s\n",tmp_db_tri_string.size(),tmp_db_tri_string.c_str());
    
   // printf("��׼��  ת��Сд len=%d, txt=%s\n",tmp_file_tri_string.size(),tmp_file_tri_string.c_str());
    
    
    //����������   һ����ͷ   һ���ֳ����߼�   ��BEGINΪ�ָ�
    ret=AnalyseTri(name,flag,head_db_tri_string,tmp_db_tri_string,head_file_tri_string,tmp_file_tri_string);
    if(ret!=CHECK_SUCCEED)
    {
        printf("�ָ��׼�⡢��ϵ���еĴ����ʱ����!!!\n");
        return CHECK_FAIL;
    }
    //ͷ�����ֲ����бȶ�,ֻ�ȶԴ����߼�����,��Ϊ�߼�һ�¼���Ϊ��������ͬ
    //head_db_tri_string  head_file_tri_string��ʱ������
    
   // printf("��ϵ��  ͷ len=%d, txt=%s\n",head_db_tri_string.size(),head_db_tri_string.c_str());
   // printf("��ϵ��  �߼��� len=%d, txt=%s\n",tmp_db_tri_string.size(),tmp_db_tri_string.c_str());
   // printf("��׼��  ͷ len=%d, txt=%s\n",head_file_tri_string.size(),head_file_tri_string.c_str());
   // printf("��׼��  �߼��� len=%d, txt=%s\n",tmp_file_tri_string.size(),tmp_file_tri_string.c_str());

    //��ʼ��������߼��αȶ�
        //ȥ�������ֵĿո� �س�   tab��
    RmSpaceAndTab(tmp_db_tri_string);
    RmSpaceAndTab(tmp_file_tri_string);

    //������������һ���ַ�Ϊ";"����ȥ��,
    //ȥ�����һ���ֺ�   ԭ�򴥷����ȴ������Ӳ���;�Ŷ����Ա���ɹ�
    //Ϊ�˱������һ��;�Ų�ƥ���������ͳһȥ��
    RmLastSemicolon(tmp_db_tri_string);
    RmLastSemicolon(tmp_file_tri_string);
    
    //printf("��ϵ�� %s:%s  �߼��� ȥ���س��Ϳո�  len=%d, txt=%s\n",flagdes[flag],name.c_str(),tmp_db_tri_string.size(),tmp_db_tri_string.c_str());
    //printf("��׼�� %s:%s  �߼��� ȥ���س��Ϳո�  len=%d, txt=%s\n",flagdes[flag],name.c_str(),tmp_file_tri_string.size(),tmp_file_tri_string.c_str());
        //���ַ������бȶ�
    if(tmp_db_tri_string != tmp_file_tri_string)
    {
        StorageStructMember(g_stru_vec_diff_name,name,flag);
    }
    //�齨������Ϣ
    return CHECK_SUCCEED;
}

//�ȶ�ָ�������������߼�
//flag Ϊ1��ʾΪ����������ȶ�4��ʾ����������ȶ�2 ��ʾ��������ȶ�5��ʾ��������ȶ�
// 3 ��ʾ�洢���̴���ȶ�  6��ʾ�洢���̾���ȶ�
int CmpOneTrigger(IN string name,IN int flag)
{
    string db_tri_string;
    string file_tri_string;

    //��ȡ��ϵ���ж�Ӧ�Ĵ����߼�
    int ret = GetTriggerTextFromDb(flag,name, db_tri_string);
    if(ret!=CHECK_SUCCEED)
    {
        printf("��ȡ��ϵ���е�%s:%s����\n",flagdes[flag],name.c_str());
        return CHECK_FAIL;
    }

    //��ȡ��׼�����Ӧ�Ĵ����߼�
    ret = GetTriggerTextFromFile(flag,name, file_tri_string);
    if(ret!=CHECK_SUCCEED)
    {
        printf("��ȡ��׼���е�%s:%s����\n",flagdes[flag],name.c_str());
        return CHECK_FAIL;
    }

    
    //printf("��ϵ��  len=%d, txt=%s\n",db_tri_string.size(),db_tri_string.c_str());
    
    //printf("��׼��  len=%d, txt=%s\n",file_tri_string.size(),file_tri_string.c_str());
    //�ȶ������ִ����Ƿ�һ��
    ret = CmpDbAndFileTri(name,flag,db_tri_string, file_tri_string);
    if(ret!=CHECK_SUCCEED)
    {
        printf("�ȶ�%s:%s����\n",flagdes[flag],name.c_str());
        return CHECK_FAIL;
    }
    //�齨������Ϣ
   return CHECK_SUCCEED; 
}
//�ȶԶ��������������洢����
//flag Ϊ1��ʾΪ����������ȶ�4��ʾ����������ȶ�2 ��ʾ��������ȶ�5��ʾ��������ȶ�
// 3 ��ʾ�洢���̴���ȶ�  6��ʾ�洢���̾���ȶ�
int CmpPartTrigger(IN vector<string> &vec_name, IN int flag)
{
    int i;
    string tmp_name;
    //ѭ�����ô������ıȶ�
    for(i=0;i<vec_name.size();i++)
    {
        tmp_name=vec_name[i];
        CmpOneTrigger(tmp_name,flag);
    }
	return CHECK_SUCCEED;
}

//�ȶ�ȫ��������
int CmpAllTrigger()
{
	int ret;

    STRU_VECTOR_INFO stru_vec_name_conf;
    STRU_VECTOR_INFO stru_vec_name_db;//�õ��ýṹ���һ����Աvec_trigger_name��¼��ϵ���еĴ�������
                            //�ڶ�����Աvec_func_name��¼�����ʹ洢����
    STRU_VECTOR_INFO stru_vec_name_com;//��¼��ϵ�����׼�ж����ڵĴ��������������洢����

    StructVectorClear(stru_vec_name_conf);
    StructVectorClear(stru_vec_name_db);
    StructVectorClear(stru_vec_name_com);
    
    //vec_trigger_name.push_back((string)"TI_ACLINEEND");
    
    //��ȡ�����ļ��еı�׼��������
    string prefix_rep_name = "name_";
    string section_name;
    //��ȡ�����ļ������õĴ����� ����  �洢����
    ret=GetAllConfigInfo(stru_vec_name_conf);
    if(ret!=CHECK_SUCCEED)
    {
        printf("CmpAllTrigger ��ȡ�����ļ���������ʧ��!!!\n");
        return CHECK_FAIL;
    }
    //��ȡ��ϵ���еĴ��������洢���̡�����������
    ret=GetAllTriggerNameFromDb(stru_vec_name_db);
    if(ret!=CHECK_SUCCEED)
    {
        if(ret==CHECK_NOEXIST)
        {
            printf("CmpAllTrigger ��ϵ��emsģʽ��û�д��������������洢����!!!\n");
        }
        else
        {
            printf("CmpAllTrigger ��ȡ��ϵ��emsģʽ�µĴ��������������洢����ʧ��!!!\n");
            return CHECK_FAIL;
        }
    }
    //�ȶԹ�ϵ�����׼�д����������ϵĲ���,����¼�����ڹ�ϵ����׼��Ĵ�������
    
    //�ȶԹ�ϵ���еĴ��������������׼��Ĵ������������Ƿ�һ��
    //���õ�����
    //int sum_conf=stru_vec_name_conf.vec_trigger_name.size()+stru_vec_name_conf.vec_func_name.size()+stru_vec_name_conf.vec_proc_name.size()\
      //          +stru_vec_name_conf.vec_trigger_name_warn.size()+stru_vec_name_conf.vec_func_name_warn.size()+stru_vec_name_conf.vec_proc_name_warn.size();
    int sum_conf = CountStruMemberNum(stru_vec_name_conf);
    //��ϵ���е�����
    //int sum_db=stru_vec_name_db.vec_trigger_name.size()+stru_vec_name_db.vec_func_name.size();
    int sum_db = CountStruMemberNum(stru_vec_name_db);

    if(sum_conf==0)
    {
        if(sum_db==0)
        {
            printf("CmpAllTrigger �����ļ��͹�ϵ��emsģʽ�еĴ��������������洢����������Ϊ0,����ȶ�!!!\n");
            return CHECK_SUCCEED;
        }
        else
        {
            printf("CmpAllTrigger �����ļ������õĴ��������������洢��������Ϊ0,��ϵ��ems�´��������������洢��������Ϊ%d\n",sum_db);
            StructVectorCpy(g_stru_vec_only_db,stru_vec_name_db);
            return CHECK_SUCCEED;
        }
    }
    else
    {
        if(sum_db==0)
        {
            printf("CmpAllTrigger ��ϵ��ems�´��������������洢��������Ϊ0,�����ļ������õĴ��������������洢��������Ϊ%d\n",sum_conf);
            StructVectorCpy(g_stru_vec_only_conf,stru_vec_name_conf);
            return CHECK_SUCCEED;
        }
    }
    //�ȶ��ҳ������ڹ�ϵ����׼��Ĵ�����
    ret=StructVectorCmp(stru_vec_name_conf,stru_vec_name_db,stru_vec_name_com);
    if(ret!=CHECK_SUCCEED)
    {
        printf("CmpAllTrigger �ȶԹ�ϵ�����׼�еĴ��������������洢���������������!!!\n");
        return CHECK_FAIL;
    }

    

    //�ȶԴ����߼�
    //�ȶԷ�Ϊ����;���������ʾ
    //flag Ϊ1��ʾΪ����������ȶ�4��ʾ����������ȶ�2 ��ʾ��������ȶ�5��ʾ��������ȶ�
    // 3��ʾ�洢���̴���ȶ�  6��ʾ�洢���̾���ȶ�
    //�ȶԴ�����
    ret=CmpPartTrigger(stru_vec_name_com.vec_trigger_name,FLAG_TRI_ERR);
    if(ret!=CHECK_SUCCEED)
    {
        printf("�ȶԴ�����ʱ����!!!\n");
        //return CHECK_FAIL;
    }
    //�ȶԺ���
    ret=CmpPartTrigger(stru_vec_name_com.vec_func_name,FLAG_FUNC_ERR);
    if(ret!=CHECK_SUCCEED)
    {
        printf("�ȶԺ���ʱ����!!!\n");
        //return CHECK_FAIL;
    }
    //�ȶԴ洢����
    ret=CmpPartTrigger(stru_vec_name_com.vec_proc_name,FLAG_PROC_ERR);
    if(ret!=CHECK_SUCCEED)
    {
        printf("�ȶԴ洢����ʱ����!!!\n");
        //return CHECK_FAIL;
    }
    //�ȶԾ��津����
    ret=CmpPartTrigger(stru_vec_name_com.vec_trigger_name_warn,FLAG_TRI_WARN);
    if(ret!=CHECK_SUCCEED)
    {
        printf("�ȶԾ��津����ʱ����!!!\n");
        //return CHECK_FAIL;
    }
    //�ȶԾ��溯��
    ret=CmpPartTrigger(stru_vec_name_com.vec_func_name_warn,FLAG_FUNC_WARN);
    if(ret!=CHECK_SUCCEED)
    {
        printf("�ȶԾ��溯��ʱ����!!!\n");
        //return CHECK_FAIL;
    }
    //�ȶԾ���洢����
    ret=CmpPartTrigger(stru_vec_name_com.vec_proc_name_warn,FLAG_PROC_WARN);
    if(ret!=CHECK_SUCCEED)
    {
        printf("�ȶԾ���洢����ʱ����!!!\n");
        //return CHECK_FAIL;
    }
    
	return CHECK_SUCCEED;
}

//��ӡ�ṹ���Ա
int PrintfStruct(STRU_VECTOR_INFO &stru_vec_name,int flag = PRINT_OTHER)
{
    int i;
    if(stru_vec_name.vec_trigger_name.size()>0)
    {
        printf("**********������**********\n");
        for(i=0;i<stru_vec_name.vec_trigger_name.size();i++)
        {
            printf("%d.%s ",(i+1),stru_vec_name.vec_trigger_name[i].c_str());
        }
        printf("\n");
    }

    if(stru_vec_name.vec_func_name.size()>0)
    {
        if(flag == PRINT_ONLY_DB)
        {
            printf("**********������洢����**********\n");
        }
        else
        {
            printf("**********����**********\n");
        }
        for(i=0;i<stru_vec_name.vec_func_name.size();i++)
        {
            printf("%d.%s ",(i+1),stru_vec_name.vec_func_name[i].c_str());
        }
        printf("\n");
    }

    if(stru_vec_name.vec_proc_name.size()>0)
    {
        printf("**********�洢����**********\n");
        for(i=0;i<stru_vec_name.vec_proc_name.size();i++)
        {
            printf("%d.%s ",(i+1),stru_vec_name.vec_proc_name[i].c_str());
        }
        printf("\n");
    }
    if(stru_vec_name.vec_trigger_name_warn.size()>0)
    {
        printf("**********������(����)**********\n");
        for(i=0;i<stru_vec_name.vec_trigger_name_warn.size();i++)
        {
            printf("%d.%s ",(i+1),stru_vec_name.vec_trigger_name_warn[i].c_str());
        }
        printf("\n");
    }
    if(stru_vec_name.vec_func_name_warn.size())
    {
        printf("**********����(����)**********\n");
        for(i=0;i<stru_vec_name.vec_func_name_warn.size();i++)
        {
            printf("%d.%s ",(i+1),stru_vec_name.vec_func_name_warn[i].c_str());
        }
        printf("\n");
    }

    if(stru_vec_name.vec_proc_name_warn.size()>0)
    {
        printf("**********�洢����(����):**********\n");
        for(i=0;i<stru_vec_name.vec_proc_name_warn.size();i++)
        {
            printf("%d.%s ",(i+1),stru_vec_name.vec_proc_name_warn[i].c_str());
        }
        printf("\n");
    }
    return CHECK_SUCCEED;
}




//��ӡ������Ϣ
int PrintfErrInfo()
{
    int succeed_flag=1;
    
    //�����ڹ�ϵ���еĴ�������ʾ
    if(CountStruMemberNum(g_stru_vec_only_db)>0)
    {
        printf("\n####################�����ڹ�ϵ����####################\n");
        PrintfStruct(g_stru_vec_only_db,PRINT_ONLY_DB);
        printf("####################END####################\n\n");
        succeed_flag=0;
    }

    //�����ڱ�׼���еĴ�������ʾ
    if(CountStruMemberNum(g_stru_vec_only_conf)>0)
    {
        printf("\n####################�����ڱ�׼����####################\n");
        PrintfStruct(g_stru_vec_only_conf);
        printf("####################END####################\n\n");
        succeed_flag=0;
    }

    //�ȶԽ������ͬ�Ĵ�������ʾ
    if(CountStruMemberNum(g_stru_vec_diff_name)>0)
    {
        printf("\n####################�ȶԽ������ͬ####################\n");
        PrintfStruct(g_stru_vec_diff_name);
        printf("####################END####################\n\n");
        succeed_flag=0;
    }

    //�ȶԹ����г���Ĵ�������ʾ
    if(CountStruMemberNum(g_stru_vec_err_name)>0)
    {
        printf("\n####################�ȶԹ����г���####################\n");
        PrintfStruct(g_stru_vec_err_name);
        printf("####################END####################\n\n");
        succeed_flag=0;
    }

    //�ȶԹ����г�����Ϣ��ʾ
    if(g_err_str.size()!=0)
    {
        printf("\n####################�ȶԹ����еĳ�����Ϣ####################\n");
        printf("%s",g_err_str.c_str());        
        printf("####################END####################\n\n");
        succeed_flag=0;
    }
    if(succeed_flag == 1)
    {
        printf("#############################################\n");
        printf("##                                         ##\n");
        printf("##  ���������������洢���̱ȶ�ƥ��ɹ�!!!  ##\n");        
        printf("##                                         ##\n");
        printf("#############################################\n");
    }
    else
    {
        printf("#############################################\n");
        printf("##                                         ##\n");
        printf("##      ��  ��  ��  ��  ��  ��  ��!!!      ##\n");        
        printf("##                                         ##\n");
        printf("#############################################\n");

    }
    
    return CHECK_SUCCEED;
}


int main(int argc, char **argv)
{
    int i;
    char server[NAME_LEN]="";
    char user_name[NAME_LEN]="";
    char passwd[NAME_LEN]="";
    ErrorInfo error;
    vector<string> argv_vec_name;
    int argc_flag = 0;
    char *endptr = NULL;

    //ȫ�ֱ�����ʼ�����
    StructVectorClear(g_stru_vec_only_conf);
    StructVectorClear(g_stru_vec_only_db);
    StructVectorClear(g_stru_vec_diff_name);
    g_err_str.clear();
    
    if(argc<4)
    {
        printf("Usage:%s  server  user  passwd  [flag](1.Tri 2.Func 3.Proc) [names]\n",argv[0]);
        exit(0);
    }
    strncpy(server,argv[1],(NAME_LEN-1));
    strncpy(user_name,argv[2],(NAME_LEN-1));
    strncpy(passwd,argv[3],(NAME_LEN-1));
    if(argc>4)
    {
        argc_flag = (int)strtol(argv[4], &endptr, 0); //��ȡ������Ʊ�ʶ
        if(strlen(endptr) != 0)
        {
            printf("�ȶԱ�ʶ�������,����ı�ʶflagΪ:%s\n", argv[4]);
            exit(0);
        }
        //�������ı�ʶ�Ƿ�ʶ��
        if(argc_flag != FLAG_TRI_ERR && argc_flag != FLAG_FUNC_ERR && argc_flag != FLAG_PROC_ERR)
        {
            printf("����ıȶԱ�ʶ��ʶ��,����ı�ʶflagΪ:%d\n", argc_flag);
            exit(0);
        }
        string tmp_string;
        char tmp_char[NAME_LEN];
        for(i=5;i<argc;i++)
        {
            memset(tmp_char,0,NAME_LEN);
            strncpy(tmp_char,argv[i],NAME_LEN-1);
            //������ת��Ϊ��д
            Check_Toupper(tmp_char);
            tmp_string = tmp_char;
            argv_vec_name.push_back(tmp_string);
        }
    }

    //��ȡ��Ŀ¼
    char chHomePath[128] = "";
    strcpy(chHomePath, getenv(HOME_ENV_NAME));
    g_strHomePath = chHomePath;
    if(g_strHomePath.empty())
    {
        printf("��ȡHOME_ENV_NAMEʧ��,�޷�������������");
        exit(0);
    }
    int	ch_value_size = g_strHomePath.size();
    if(chHomePath[ch_value_size - 1] == '/')
    {
        g_strHomePath.erase(ch_value_size - 1, ch_value_size);
    }
    
    //���ݴ������ӹ�ϵ��
    if(!g_CDci.Connect(server, user_name, passwd, &error))
    {
        printf("���ӹ�ϵ��ʧ��,������Ϊ:%d  ������ϢΪ:%s �����ļ�:%s ������:%d\n",error.error_no,error.error_info,error.file_name,error.file_line);
        return CHECK_FAIL;
    }

    //�ȶ�ָ�����������
    if(argc>4)
    {
    	CmpPartTrigger(argv_vec_name,argc_flag);
    }
	//�ȶ������ļ��еĴ�����
    else
    {
    	CmpAllTrigger();
    }

    //������Ϣ����
    PrintfErrInfo();

    //�Ͽ���ϵ��
    memset(&error,0,sizeof(ErrorInfo));
    if(!g_CDci.DisConnect(&error))
    {
        printf("�Ͽ���ϵ��ʧ��,������Ϊ:%d  ������ϢΪ:%s �����ļ�:%s ������:%d\n",error.error_no,error.error_info,error.file_name,error.file_line);
        return CHECK_FAIL;
    }
    return 0;
}


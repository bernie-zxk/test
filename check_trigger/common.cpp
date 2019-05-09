#include "common.h"
//��ֵת�ַ�
string IntToStr(int value)
{
    //��intֵת��Ϊstring����
    string ret_str;
    char buff[12];//��һ��/0λ
    sprintf(buff, "%d", value);
    ret_str = buff;
    return ret_str;
}

//��ĸСдת����д
int Check_Toupper(ALTER char *str)
{
    int i;
    int len=strlen(str);
    for(i=0; i<len;i++)
    {
        str[i]=toupper(str[i]);
    }
    
    return CHECK_SUCCEED;
}
//��ĸ��дתСд
int Check_Tolower(ALTER char *str)
{
    int i;
    int len=strlen(str);
    for(i=0; i<len;i++)
    {
        str[i]=tolower(str[i]);
    }

    return CHECK_SUCCEED;
}
//�ṹ�����
int StructVectorClear(ALTER STRU_VECTOR_INFO &stru)
{
    //��ո�����Ա
    stru.vec_trigger_name.clear();
    stru.vec_func_name.clear();
    stru.vec_proc_name.clear();
    stru.vec_trigger_name_warn.clear();
    stru.vec_func_name_warn.clear();
    stru.vec_proc_name_warn.clear();
    return CHECK_SUCCEED;
}



//�����vector�ṹ��Ŀ���
int StructVectorCpy(OUT STRU_VECTOR_INFO &dest, IN STRU_VECTOR_INFO &src)
{
    int i;
    //������
    for(i=0;i<src.vec_trigger_name.size();i++)
    {
        dest.vec_trigger_name.push_back(src.vec_trigger_name[i]);
    }
    //���津����
    for(i=0;i<src.vec_trigger_name_warn.size();i++)
    {
        dest.vec_trigger_name_warn.push_back(src.vec_trigger_name_warn[i]);
    }
    //����
    for(i=0;i<src.vec_func_name.size();i++)
    {
        dest.vec_func_name.push_back(src.vec_func_name[i]);
    }
    //���溯��
    for(i=0;i<src.vec_func_name_warn.size();i++)
    {
        dest.vec_func_name_warn.push_back(src.vec_func_name_warn[i]);
    }
    //�洢����
    for(i=0;i<src.vec_proc_name.size();i++)
    {
        dest.vec_proc_name.push_back(src.vec_proc_name[i]);
    }
    //����洢����
    for(i=0;i<src.vec_proc_name_warn.size();i++)
    {
        dest.vec_proc_name_warn.push_back(src.vec_proc_name_warn[i]);
    }

    
    return CHECK_SUCCEED;
}

//��ϵ����Դ�ͷ�
int Check_FreeDb(IN ColAttr *attr,IN int attr_num, IN char *data_buf)
{
    g_CDci.FreeReadData(attr, attr_num,data_buf);
    //ret=g_CDci.FreeColAttrData(attr, attr_num);
    return CHECK_SUCCEED;
}
//�ӹ�ϵ���ȡ����
int GetNameFromDb(char *sql,vector<string> &vec_name)
{
    int rec_num=0;
    int attr_num=0;
	ColAttr *attr = NULL;
    char *data_buf=NULL;
    ErrorInfo error;
    int i,ret;
    char tmp_name_char[NAME_LEN]="";
    string tmp_name_str;

      
	if(!g_CDci.ReadData(sql, -1, &rec_num, &attr_num, &attr, &data_buf, &error))
	{
        printf("GetNameFromDb ����ReadData������ȡ��ϵ��emsģʽ�µĴ���������ʱ����,������Ϊ:%d  ������ϢΪ:%s �����ļ�:%s ������:%d\n",\
                    error.error_no,error.error_info,error.file_name,error.file_line);
        return CHECK_FAIL;
	}
    if(rec_num<=0)
    {
        printf("�ڹ�ϵ��emsģʽ�²�������Ҫ��ȡ������\n");
        ret=Check_FreeDb(attr, attr_num, data_buf);
        if(ret!=CHECK_SUCCEED)
        {
            printf("�ͷŹ�ϵ��ӿ���������������ռ�����ݿռ�ʧ��\n");
        }
        return CHECK_NOEXIST;
    }

    int len=0;
    int offset=0;
    offset=0;
    len=attr[0].data_size;
    if(len>=NAME_LEN)
    {
        printf("��ϵ������Ҫ��ȡ�������г��ȴ���%d\n",NAME_LEN);
        ret=Check_FreeDb(attr, attr_num, data_buf);
        if(ret!=CHECK_SUCCEED)
        {
            printf("�ͷŹ�ϵ��ӿ���������������ռ�����ݿռ�ʧ��\n");
        }
        return CHECK_FAIL;
    }
    for(i=0;i<rec_num;i++)
    {
        memset(tmp_name_char,0,NAME_LEN);
        strncpy(tmp_name_char,(data_buf+offset),len);
        Check_Toupper(tmp_name_char);//Сдת���ɴ�д
        tmp_name_str=tmp_name_char;
        vec_name.push_back(tmp_name_str);
        offset+=ONE_ATTR_OFFSET(len);//������һ�����ݵ���ʼ��ַ����һ�����ݵ�ƫ��
    }
    
    //�ͷŵ��ú��������Ŀռ�

    ret=Check_FreeDb(attr, attr_num, data_buf);
    if(ret!=CHECK_SUCCEED)
    {
        printf("�ͷŹ�ϵ��ӿ���������������ռ�����ݿռ�ʧ��\n");
    }
    return CHECK_SUCCEED;
    
}
//�ӹ�ϵ���ȡ��ϵ���д��������������洢��������
int GetAllTriggerNameFromDb(STRU_VECTOR_INFO &stru_vec_name_db)
{
    int ret;
    int tri_noexist_flag=0;
    
    char sql[1024];
    //��ȡemsģʽ�����б�Ĵ�������
    sprintf(sql,"select name from ems.sysdba.systriggers where tableid in \
(select id from ems.sysdba.systables where schid in (select schid from ems.sysdba.sysschemas where name = 'EMS' or name = 'ems'))");    
    ret=GetNameFromDb(sql,stru_vec_name_db.vec_trigger_name);
    if(ret!=CHECK_SUCCEED)
    {
        if(ret==CHECK_NOEXIST)
        {
            printf("�ڹ�ϵ��emsģʽ�²����ڴ�����\n");
            tri_noexist_flag=1;
        }
        else
        {
            printf("��ȡ��ϵ��emsģʽ�µĴ���������ʱʧ��\n");
            return CHECK_FAIL;
        }
        
    }

       //��ȡEMSģʽ�µ����к���
        sprintf(sql,"select name from ems.sysdba.sysprocs where schid in \
    (select schid from ems.sysdba.sysschemas where name = 'EMS' or name = 'ems') and  pfflag = 'N'");    
        ret=GetNameFromDb(sql,stru_vec_name_db.vec_func_name);
        if(ret!=CHECK_SUCCEED)
        {
            if(ret==CHECK_NOEXIST)
            {
                printf("�ڹ�ϵ��emsģʽ�²����ں���\n");
                if(tri_noexist_flag==1)
                {
                    return CHECK_NOEXIST;
                }
                else
                {
                    return CHECK_SUCCEED;
                }
            }
            else
            {
                printf("��ȡ��ϵ��emsģʽ�µĴ���������ʱʧ��\n");
                return CHECK_FAIL;
            }
            
        }
        
    
         //��ȡEMSģʽ�µ����д洢����
        sprintf(sql,"select name from ems.sysdba.sysprocs where schid in \
    (select schid from ems.sysdba.sysschemas where name = 'EMS' or name = 'ems') and  pfflag = 'Y'");    
        ret=GetNameFromDb(sql,stru_vec_name_db.vec_proc_name);
        if(ret!=CHECK_SUCCEED)
        {
            if(ret==CHECK_NOEXIST)
            {
                printf("�ڹ�ϵ��emsģʽ�²����ڴ洢����\n");
                if(tri_noexist_flag==1)
                {
                    return CHECK_NOEXIST;
                }
                else
                {
                    return CHECK_SUCCEED;
                }
            }
            else
            {
                printf("��ȡ��ϵ��emsģʽ�µĴ���������ʱʧ��\n");
                return CHECK_FAIL;
            }
            
        }
        

    return CHECK_SUCCEED;
}
//�ӵ�ǰϵͳ��ϵ���л�ȡָ���Ĵ�����������洢���̵ȴ���
//flag Ϊ1��4��ʾΪ������  2��5  ��ʾ����  3��6 ��ʾ�洢����
int GetTriggerTextFromDb(IN int flag,IN string &name, OUT string &db_tri_string)
{
    int rec_num=0;
    int attr_num=0;
	ColAttr *attr = NULL;
    char *data_buf=NULL;
    unsigned int data_buf_size=0;
    ErrorInfo error;
    int i,ret;
    char tmp_name_char[NAME_LEN]="";
    char err_char[1024];
    
    string tmp_name_str=name;
    
    
    //�������е�Сд��Ϊ��д
    strcpy(tmp_name_char,tmp_name_str.c_str());
    Check_Toupper(tmp_name_char);
    tmp_name_str=tmp_name_char;

    
    char sql[1024];
    if(flag==FLAG_TRI_ERR || flag == FLAG_TRI_WARN)//��ȡ������sql
    {
        sprintf(sql,"SELECT TXT FROM EMS.SYSDBA.SYSTRIGGERS WHERE NAME='%s' AND tableid in \
(select id from ems.sysdba.systables where schid in (select schid from ems.sysdba.sysschemas where name = 'EMS' or name = 'ems'))",tmp_name_str.c_str());    
    }
    else//��ȡ�洢���̺ͺ���sql
    {        
        sprintf(sql,"SELECT TXT FROM EMS.SYSDBA.SYSPROCS WHERE NAME='%s' AND schid in \
(select schid from ems.sysdba.sysschemas where name = 'EMS' or name = 'ems')",tmp_name_str.c_str());    
    }
	if(!g_CDci.ReadDataEx(sql, -1, &rec_num, &attr_num, &attr, &data_buf, &data_buf_size, &error))
	{
        sprintf(err_char,"%s:%s  ����ReadDataEx������ȡ��ϵ���е�%s�����߼�ʱ����,������Ϊ:%d  ������ϢΪ:%s �����ļ�:%s ������:%d\n",\
                    flagdes[flag],tmp_name_str.c_str(),tmp_name_str.c_str(),error.error_no,error.error_info,error.file_name,error.file_line);
    //    printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        return CHECK_FAIL;
	}
    if(rec_num<=0)
    {
        sprintf(err_char,"%s:%s  �ڹ�ϵ���в�����\n",flagdes[flag],tmp_name_str.c_str());
      //  printf("%s",err_char);
        g_err_str = g_err_str+err_char;
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        ret=Check_FreeDb(attr, attr_num, data_buf);
        if(ret!=CHECK_SUCCEED)
        {
            printf("�ͷŹ�ϵ��ӿ���������������ռ�����ݿռ�ʧ��\n");
        }
        return CHECK_NOEXIST;
    }
    if(rec_num>1)
    {
        sprintf(err_char,"%s:%s  �ڹ�ϵ���д��ڵ���������1,���ֶ����EMS.SYSDBA.SYSTRIGGERS/SYSPROCS���µļ�¼����\n",flagdes[flag],tmp_name_str.c_str());
    //    printf("%s",err_char);
        g_err_str = g_err_str+err_char;
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        
        ret=Check_FreeDb(attr, attr_num, data_buf);
        if(ret!=CHECK_SUCCEED)
        {
            printf("�ͷŹ�ϵ��ӿ���������������ռ�����ݿռ�ʧ��\n");
        }
        return CHECK_MORE;
    }

    
    /*printf("attr_num=%d\n",attr_num);
    for(i=0;i<attr_num;i++)
    {
        printf("col_name=%s\n",attr[i].col_name);
        printf("data_type=%hd\n",attr[i].data_type);
        printf("data_size=%d\n",attr[i].data_size);
        printf("data_offset=%d\n",attr[i].data_offset);
        printf("precision=%hd\n",attr[i].precision);
        short tmp=attr[i].scale;
        printf("scale=%hd\n",tmp);
    }*/
   // printf("%d########11111%s\n",data_buf_size,data_buf+ONE_TXT_OFFSET(attr[0].data_size));

    char  *tmp_buf=NULL;
    int len=0;
    int offset=0;
    if(attr[0].data_type==DCI_CLOB)//#define  DCI_CLOB  112  ���ı�����
    {
        offset=ONE_TXT_OFFSET(attr[0].data_size);//���ı�ƫ�Ƽ�������ͨ���Բ�ͬ
        len = data_buf_size-offset;
        
    }
    else
    {
        offset=0;
        len=attr[0].data_size;
    }
    tmp_buf=(char *)malloc((len+1)*sizeof(char));
    if(tmp_buf==NULL)
    {
        sprintf(err_char,"%s:%s  GetTriggerTextFromDb  ����ռ�ʧ��,����ϵͳ�����Ƿ�����!\n",flagdes[flag],tmp_name_str.c_str());
        printf("%s",err_char);
        g_err_str = g_err_str+err_char;
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        
        ret=Check_FreeDb(attr, attr_num, data_buf);
        if(ret!=CHECK_SUCCEED)
        {
            printf("�ͷŹ�ϵ��ӿ���������������ռ�����ݿռ�ʧ��\n");
        }
        return CHECK_FAIL;
    }
  //  printf("%d#############%s\n",len,data_buf+offset);
    memset(tmp_buf,0,((len+1)*sizeof(char)));
    strncpy(tmp_buf,data_buf+offset,len);
   // printf("***************%s\n",tmp_buf);

    db_tri_string=tmp_buf;

    //�ȶ�������  Ȼ���ٿ���
  /*  printf("data_buf_size=%d\n",data_buf_size);
    printf("data_buf name=%s\n",data_buf);
    printf("data_buf txt=%s\n",(data_buf+ONE_TXT_OFFSET(attr[0].data_size)));
*/
    
    //�ͷŵ��ú��������Ŀռ�

    if(tmp_buf!=NULL)
    {
        free(tmp_buf);
        tmp_buf=NULL;
    }
    ret=Check_FreeDb(attr, attr_num, data_buf);
    if(ret!=CHECK_SUCCEED)
    {
        printf("�ͷŹ�ϵ��ӿ���������������ռ�����ݿռ�ʧ��\n");
    }
    return CHECK_SUCCEED;
}


//�ӵ�ǰĿ¼�»�ȡָ���Ĵ�������洢���̻����ȴ���(��׼�߼�����)
//flag Ϊ1��4��ʾΪ������  2��5  ��ʾ����  3��6 ��ʾ�洢����
int GetTriggerTextFromFile(IN int flag,IN string &name, OUT string &file_tri_string)
{
    char tmp_name_char[NAME_LEN]="";
    string tmp_name_str=name;
    char err_char[1024];
    
    
    //�������е�Сд��Ϊ��д
    strcpy(tmp_name_char,name.c_str());
    Check_Toupper(tmp_name_char);
    tmp_name_str=tmp_name_char;

    //�齨��׼�ļ�·��
    string file_path;
    if(flag == FLAG_TRI_ERR)
    {
        file_path=g_strHomePath+PUBLIC_TRI_PATH+tmp_name_str;
    }
    else if(flag == FLAG_FUNC_ERR)
    {
        file_path=g_strHomePath+PUBLIC_FUNC_PATH+tmp_name_str;
    }
    else if(flag == FLAG_PROC_ERR )
    {
        file_path=g_strHomePath+PUBLIC_PROC_PATH+tmp_name_str;
    }
    else if(flag == FLAG_TRI_WARN)
    {
        file_path=g_strHomePath+PUBLIC_TRI_WARN_PATH+tmp_name_str;
    }
    else if(flag == FLAG_FUNC_WARN)
    {
        file_path=g_strHomePath+PUBLIC_FUNC_WARN_PATH+tmp_name_str;
    }
    else if(flag == FLAG_TRI_WARN)
    {
        file_path=g_strHomePath+PUBLIC_PROC_WARN_PATH+tmp_name_str;
    }
    else
    {
        sprintf(err_char,"%s:%s  GetTriggerTextFromFile  �齨��׼�������ļ�·��ʱ,FLAG��ʶ(%d)��ʶ��!!!\n",\
                    flagdes[flag],tmp_name_str.c_str(),flag);
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        return CHECK_FAIL;
    }

    
    struct stat stat_buf;
    if(stat(file_path.c_str(), &stat_buf) < 0)
    {
        sprintf(err_char,"%s:%s  GetTriggerTextFromFile  ��ȡ�ļ�״̬(stat)ʧ��,�����ļ�%s�Ƿ����!!!\n",\
                flagdes[flag],tmp_name_str.c_str(),file_path.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        return CHECK_FAIL;
    }
    int file_len=stat_buf.st_size;
    if (file_len <= 0)
    {
        sprintf(err_char,"%s:%s  GetTriggerTextFromFile  �ļ�����Ϊ0(ZERO) ,�����ļ�%s�Ƿ��������!\n",\
                flagdes[flag],tmp_name_str.c_str(),file_path.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        return CHECK_FAIL;
    }
    //printf("*************file_len=%d\n",file_len);
    char *buff=NULL;
    buff=(char *)malloc((file_len+1)*sizeof(char));
    if(buff==NULL)
    {
        sprintf(err_char,"%s:%s  GetTriggerTextFromFile  ����ռ�ʧ��,����ϵͳ�����Ƿ�����!\n",\
                flagdes[flag],tmp_name_str.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        return CHECK_FAIL;
    }

    memset(buff, 0, file_len);

    FILE *fp=NULL;
    fp = fopen(file_path.c_str(), "r");
    if(fp==NULL)
    {
        
        sprintf(err_char,"%s:%s  GetTriggerTextFromFile  ���ļ�%sʧ��,�����:%d  ������Ϣ:%s\n",\
                flagdes[flag],tmp_name_str.c_str(),file_path.c_str(),errno,strerror(errno));
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        if(buff!=NULL)
        {
            free(buff);
            buff=NULL;
        }
        return CHECK_FAIL;
    }
    
    int read_count=fread(buff, sizeof(char), file_len, fp);
    if(read_count!=file_len)
    {
        sprintf(err_char,"%s:%s  GetTriggerTextFromFile  fread ��ȡ�ļ�����ʧ��,read_count=%d  file_len=%d\n",\
                flagdes[flag],tmp_name_str.c_str(),read_count,file_len);
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//�洢�������
        if(buff!=NULL)
        {
            free(buff);
            buff=NULL;
        }
        fclose(fp);
        fp=NULL;
        return CHECK_FAIL;
    }
    //�ر��ļ�
    fclose(fp);
    fp=NULL;
    
    buff[read_count]='\0';
    
    file_tri_string=buff;
    //printf("##############%s\n",buff);
    if(buff!=NULL)
    {
        free(buff);
        buff=NULL;
    }
    return CHECK_SUCCEED;
}

//ȥ�����һ���ֺ�   ԭ�򴥷����ȴ������Ӳ���;�Ŷ����Ա���ɹ�
//Ϊ�˱������һ��;�Ų�ƥ���������ͳһȥ��
int RmLastSemicolon(string &dest_str)
{
    int ch_value_size = dest_str.size();
    int pos= dest_str.find_last_of(';');
    if((pos != dest_str.npos) && (ch_value_size== (pos+1)))//���һ���ַ�Ϊ��
    {
        dest_str.erase(pos,1);
    }
    return CHECK_SUCCEED;
}
//ɾ��string�е�ָ���ַ�
int RmChar(string &dest_str, char del_c)
{
    int startpos =0;

    //�ҵ�Ҫɾ���ַ���λ��
    startpos = dest_str.find(del_c,startpos);
    while(startpos != dest_str.npos)
    {
        //ɾ��Ҫɾ�����ַ���
        dest_str.erase(startpos,1);
        //�´�ѭ��
        startpos = dest_str.find(del_c,startpos);
    }   
    return CHECK_SUCCEED;
}
//ȥ����õĴ����еĿո񡢻س���TAB�ȵ�  ' ' '\t'   '\n'   '\r'
int RmSpaceAndTab(string &tri_string)
{
    //ȥ���ո�
    RmChar(tri_string,' ');
    
    //ȥ��Tab
    RmChar(tri_string,'\t');

    //ȥ��c�س�
    RmChar(tri_string,'\n');

    //ȥ������ȥ��windows�»س�\r\n   �����µ�\r
    RmChar(tri_string,'\r');

    return CHECK_SUCCEED;
}

//ȥ�� ע����
int RmComment(string &tri_string)
{
    int startpos=0;
    int endpos=0;
    //ȥ�� --ע����
    startpos = tri_string.find("--",startpos);
    while(startpos!=tri_string.npos)
    {
        //�ҵ�--�еĽ���
        endpos = tri_string.find('\n',startpos);
        
        if(endpos ==tri_string.npos)
        //����������һ��Ϊע����,ƥ�䲻��'\n'�����
        {
            //����ĳ���Ϊ�����ַ�������,��֤�����һ��ע��ɾ����
            tri_string.erase(startpos,tri_string.size());
        }
        else
        //������������µ�ע����
        {
            //���--ע����
            tri_string.erase(startpos,(endpos-startpos+1));
        }
        //�ҵ���һ��ע���еĿ�ʼ
        startpos = tri_string.find("--",startpos);
    }
    
    //ȥ��/* */  ע����
    startpos=0;
    endpos=0;
    startpos = tri_string.find("/*",startpos);
    while(startpos!=tri_string.npos)
    {
        //�ҵ�--�еĽ���
        endpos = tri_string.find("*/",startpos);
        //ȥ��/* */ע����
        tri_string.erase(startpos,(endpos-startpos+1));
        //�ҵ���һ��ע���еĿ�ʼ
        startpos = tri_string.find("/*",startpos);
    }
    
    return CHECK_SUCCEED;
}
//��ȡ�����ļ�check_trigger.confָ������
int GetConfigInfo(IN string &section_name,IN string &prefix_rep_name, OUT vector<string> &vec_name)
{
    int i;
    string key_name,tmp_str;
    char ch_value[MAX_PARA_NAME];
    int num;

    CParaManage *pm;
    pm = CParaManage::CreateObject(CHECK_TRI_CONF);
    if(pm==NULL)
    {
        printf("��ȡ�����ļ�%s�е�%sʱ,��ʼ����ȡ����ʧ��!!!\n",CHECK_TRI_CONF,section_name.c_str());
        CParaManage::RemoveObject(pm);
        pm = NULL;
        return CHECK_FAIL;
    }
    if(pm->GetKeyNum((char *)section_name.c_str(), num) < 0)
    {
        printf("��ȡ�����ļ�%s��%s������ʧ��!!!\n",CHECK_TRI_CONF,section_name.c_str());
        CParaManage::RemoveObject(pm);
        pm = NULL;
        return CHECK_FAIL;
    }
    if(num>0)
    {
        for(i=0;i<num;i++)
        {
            key_name=prefix_rep_name+IntToStr(i);
            memset(ch_value,0,MAX_PARA_NAME);
            if(pm->GetKey((char *)section_name.c_str(), (char *)key_name.c_str(), ch_value, MAX_PARA_NAME) < 0)
            {
                printf("��ȡ��%d��%s����ʧ��!!!\n",(i+1),section_name.c_str());
                CParaManage::RemoveObject(pm);
                pm = NULL;
                return CHECK_FAIL;
            }
            Check_Toupper(ch_value);//��Сдת���ɴ�д
            tmp_str=ch_value;
            vec_name.push_back(tmp_str);
        }
    }
    else
    {
        printf("�����ļ�%s�е�%s����Ϊ0!!!\n",CHECK_TRI_CONF,section_name.c_str());
        CParaManage::RemoveObject(pm);
        pm = NULL;
        return CHECK_SUCCEED;
    }
    
    CParaManage::RemoveObject(pm);
    pm = NULL;
    return CHECK_SUCCEED;
}

//��ȡ�����ļ�check_trigger.conf��������
int GetAllConfigInfo(OUT STRU_VECTOR_INFO &stru_vec_name_conf)
{
    int ret;
     //��ȡ�����ļ��еı�׼��������
    string prefix_rep_name = "name_";
    string section_name;
    //��ȡ�����ļ������õĴ����� ����  �洢����
    //������
    section_name ="trigger";
    ret=GetConfigInfo(section_name,prefix_rep_name,stru_vec_name_conf.vec_trigger_name);
    if(ret!=CHECK_SUCCEED)
    {
        printf("GetAllConfigInfo �������ļ��л�ȡ��������Ϣʧ��!!!\n");
        return CHECK_FAIL;
    }
    //����
    section_name="function";
    ret=GetConfigInfo(section_name,prefix_rep_name,stru_vec_name_conf.vec_func_name);
    if(ret!=CHECK_SUCCEED)
    {
        printf("GetAllConfigInfo �������ļ��л�ȡ������Ϣʧ��!!!\n");
        return CHECK_FAIL;
    }
    //�洢����
    section_name="procedure";
    ret=GetConfigInfo(section_name,prefix_rep_name,stru_vec_name_conf.vec_proc_name);
    if(ret!=CHECK_SUCCEED)
    {
        printf("GetAllConfigInfo �������ļ��л�ȡ�洢������Ϣʧ��!!!\n");
        return CHECK_FAIL;
    }
    //���津����
    section_name="trigger_warn";
    ret=GetConfigInfo(section_name,prefix_rep_name,stru_vec_name_conf.vec_trigger_name_warn);
    if(ret!=CHECK_SUCCEED)
    {
        printf("GetAllConfigInfo �������ļ��л�ȡ���津������Ϣʧ��!!!\n");
        return CHECK_FAIL;
    }
    //���溯��
    section_name="function_warn";
    ret=GetConfigInfo(section_name,prefix_rep_name,stru_vec_name_conf.vec_func_name_warn);
    if(ret!=CHECK_SUCCEED)
    {
        printf("GetAllConfigInfo �������ļ��л�ȡ���溯����Ϣʧ��!!!\n");
        return CHECK_FAIL;
    }
    //����洢����
    section_name="procedure_warn";
    ret=GetConfigInfo(section_name,prefix_rep_name,stru_vec_name_conf.vec_proc_name_warn);
    if(ret!=CHECK_SUCCEED)
    {
        printf("GetAllConfigInfo �������ļ��л�ȡ����洢������Ϣʧ��!!!\n");
        return CHECK_FAIL;
    }

    return CHECK_SUCCEED;
}

//����ʶflag�洢���Ƶ�Ŀ�Ľṹ����
int StorageStructMember(STRU_VECTOR_INFO &dest_vector_info,string &name,int flag)
{
    if(flag == FLAG_TRI_ERR)
    {
        dest_vector_info.vec_trigger_name.push_back(name);
    }
    else if(flag == FLAG_FUNC_ERR)
    {
        dest_vector_info.vec_func_name.push_back(name);
    }
    else if(flag == FLAG_PROC_ERR)
    {
        dest_vector_info.vec_proc_name.push_back(name);
    }
    else if(flag == FLAG_TRI_WARN)
    {
        dest_vector_info.vec_trigger_name_warn.push_back(name);
    }
    else if(flag == FLAG_FUNC_WARN)
    {
        dest_vector_info.vec_func_name_warn.push_back(name); 
    }
    else if(flag == FLAG_PROC_WARN)
    {
        dest_vector_info.vec_proc_name_warn.push_back(name);
    }
    else
    {
        //��Ӧ���ߵ��˴�,�ϲ㺯�����Ѿ���֤flagΪʶ��Χ�ڵ�
        printf("�洢�������ȴ��������������洢������ʱ,flag��ʶλ(%d)��ʶ��!!!\n",flag);
        return CHECK_FAIL;
    }
    return CHECK_SUCCEED;
}

//��ȡ��Ա����
int CountStruMemberNum(STRU_VECTOR_INFO &stru_vec_name)
{
    int sum=0;
    
    sum=stru_vec_name.vec_trigger_name.size()+stru_vec_name.vec_func_name.size()+stru_vec_name.vec_proc_name.size()\
                +stru_vec_name.vec_trigger_name_warn.size()+stru_vec_name.vec_func_name_warn.size()+stru_vec_name.vec_proc_name_warn.size();
    
    return sum;
}


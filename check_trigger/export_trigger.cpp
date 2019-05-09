#include "export_trigger.h"

//ָ��·���´���Ŀ¼
int CreateCatalog(string path,string name)
{
    int ret;
    string command_str;
    string local_path = path;
    string catalog_name = name;
    int local_path_len = local_path.length();
    if(local_path[local_path_len - 1] == '/')
    {
       local_path.erase(local_path_len - 1, local_path_len);
    }
    command_str =  "mkdir " + local_path + "/" + catalog_name;
    ret = ServiceExcute(command_str.c_str());


    if(ret != EXPORT_SUCCEED)
    {
        return EXPORT_FAIL;
    }

    return 0;
}


//��ָ��·����д���ļ�
int WriteExportText(string &catalog_path,string &name,string &buff)
{
    string file_path = catalog_path;
    string file_name = name;
    string txt_string = buff;
    int txt_len = txt_string.length();
    int file_path_len = file_path.length();

    if(file_path[file_path_len - 1] == '/')
    {
        file_path.erase(file_path_len - 1, file_path_len);
    }

   string file = file_path + "/" + file_name;
//printf("·�� :%s\n",file_path.c_str());  

  //���ļ�
    FILE *fp=NULL;
    fp = fopen(file.c_str(), "w");
     if(fp==NULL)
    { 
        printf("open export file filed!\n");
        fclose(fp);
        fp=NULL;
        return EXPORT_FAIL;
    }

    int write_count=fwrite(txt_string.c_str(), sizeof(char),txt_len , fp);
    if(write_count != txt_len)
    {
        printf("д���ļ�����ʧ��");
        fclose(fp);
        fp=NULL;
        return EXPORT_FAIL;
    }
    
    //�ر��ļ�
    fclose(fp);
    fp=NULL;

    return 0;
}

//���������ı���׼Ŀ¼
int CreateExportCatalog(string &export_path)
{
	int ret = -1;
    string catalog_path = export_path;
    string catalog_name = EXPORT_NAME_TXT;

    ret = CreateCatalog(catalog_path,catalog_name);
    if(ret != EXPORT_SUCCEED)
    {
        printf("��������Ŀ¼ʧ��\n");
        return EXPORT_FAIL;
    }

    catalog_path = catalog_path  + catalog_name;
    
    ret = CreateCatalog(catalog_path,"trigger");
    if(ret != EXPORT_SUCCEED)
    {
        printf("create Catalog trigger failed\n");
        return EXPORT_FAIL;
    }
    
    ret = CreateCatalog(catalog_path,"function");
    if(ret != EXPORT_SUCCEED)
    {
        printf("create Catalog function failed\n");
        return EXPORT_FAIL;
    }

    ret = CreateCatalog(catalog_path,"procedure");
    if(ret != EXPORT_SUCCEED)
    {
        printf("create Catalog procedure failed\n");
        return EXPORT_FAIL;
    }

    ret = CreateCatalog(catalog_path,"trigger_warn");
    if(ret != EXPORT_SUCCEED)
    {
        printf("create Catalog trigger_warn failed\n");
        return EXPORT_FAIL;
    }
    
    ret = CreateCatalog(catalog_path,"function_warn");
    if(ret != EXPORT_SUCCEED)
    {
        printf("create Catalog function_warn failed\n");
        return EXPORT_FAIL;
    }
    
    ret = CreateCatalog(catalog_path,"procedure_warn");
    if(ret != EXPORT_SUCCEED)
    {
        printf("create Catalog procedure_warn failed\n");
        return EXPORT_FAIL;
    }

    return 0;

}
//1.���������ļ����� 2.�������ݿ⵼�����Զ����������ļ�
int ExportAllTrigger(int model)
{
    int ret;

    if(model == 1)
    {
        ret = ExportByConf();
    }
    else if(model == 2)
    {
        ret = ExportAddConf();
    }
    else 
    {
        printf("1.���������ļ����� 2.�������ݿ⵼�����Զ����������ļ�\n");
        return EXPORT_FAIL;
    }

 
    return ret;
}


int ExportByConf()//****************************************************//
{
    int ret = 0;
    string local_path;
    string db_tri_string;
    STRU_VECTOR_INFO stru_vec_name_conf;
    StructVectorClear(stru_vec_name_conf);
    
    //��ȡ�����ļ������õ�   ������    ����    �洢����
    ret = GetAllConfigInfo(stru_vec_name_conf);
    if(ret!= EXPORT_SUCCEED)
    {
        printf("CmpAllTrigger ��ȡ�����ļ���������ʧ��!!!\n");
        return EXPORT_FAIL;
    }

    //�½������ļ���
    string catalog_path = g_export_path;
    ret = CreateExportCatalog(catalog_path);
    if(ret != EXPORT_SUCCEED)
    {
        printf("���������ı�Ŀ¼ʧ��\n");
        return EXPORT_FAIL;
    }

        //����conf�ļ����½��ļ���
    string file_path = g_home_path + CONF_PATH;
    string backup_file_path = g_export_path + EXPORT_NAME_TXT;
    string command_str = "cp " + file_path + "check_trigger.conf " + backup_file_path;
//printf("����·����%s\n",command_str.c_str());
    ret = ServiceExcute(command_str.c_str());
    if(ret != EXPORT_SUCCEED)
    {
        printf("����conf�ļ�ʧ��!\n");
        return EXPORT_FAIL;
    }
    else
    {
        printf("����conf�ļ��ɹ�!\n");
    }
    ////����struct��ȡ���벢д���½��ļ�����

    ret = WriteTextByStru(stru_vec_name_conf);
    if(ret != EXPORT_SUCCEED)
    {
        printf("�����ı�д������д���\n");
        return EXPORT_FAIL;
    }

  
    return EXPORT_SUCCEED;
}



//����struct��ȡ���벢д���½��ļ�����
int WriteTextByStru(STRU_VECTOR_INFO &stru_vec_name)
{
    int ret;
    int i = 0;
    string db_tri_string;
    string catalog_path;

    
    //TRIGGER
    catalog_path = g_export_path + EXPORT_NAME_TXT;
    string file_path = catalog_path +  "/trigger";
    for(i = 0;i < stru_vec_name.vec_trigger_name.size();i++)
    {
         db_tri_string.clear();
        //��ȡ��ϵ���ж�Ӧ�Ĵ����߼�
        ret = GetTriggerTextFromDb(1,stru_vec_name.vec_trigger_name[i], db_tri_string);
        if(ret!=EXPORT_SUCCEED)
        {   
            g_stru_vec_err_name.vec_trigger_name.push_back(stru_vec_name.vec_trigger_name[i]);
            //printf("��ȡ��ϵ���е�%s:%s����\n",flagdes[1],stru_vec_name.vec_trigger_name[i].c_str());
            continue;
        }   
        
        ret = WriteExportText(file_path,stru_vec_name.vec_trigger_name[i],db_tri_string);
        if(ret != EXPORT_SUCCEED)
        {
            printf("����д��ʧ��\n");
            
           return EXPORT_FAIL; 
        }
   
        
    }

//FUNCTION
    file_path.clear();
    file_path = catalog_path + "/function";
    for(i = 0;i < stru_vec_name.vec_func_name.size();i++)
    {
        db_tri_string.clear();
        //��ȡ��ϵ���ж�Ӧ�Ĵ����߼�
        ret = GetTriggerTextFromDb(2,stru_vec_name.vec_func_name[i], db_tri_string);
        if(ret!=EXPORT_SUCCEED)
        {
            g_stru_vec_err_name.vec_func_name.push_back(stru_vec_name.vec_func_name[i]);
           // printf("��ȡ��ϵ���е�%s:%s����\n",flagdes[2],stru_vec_name.vec_func_name[i].c_str());
			  continue;
        }   
        
        ret = WriteExportText(file_path,stru_vec_name.vec_func_name[i],db_tri_string);
        if(ret != EXPORT_SUCCEED)
        {
            printf("����д��ʧ��\n");
           return EXPORT_FAIL; 
        }
    
        
    }
    
//procedure
    file_path.clear();
    file_path = catalog_path + "/procedure";
    for(i = 0;i < stru_vec_name.vec_proc_name.size();i++)
    {
        db_tri_string.clear();
        //��ȡ��ϵ���ж�Ӧ�Ĵ����߼�
        ret = GetTriggerTextFromDb(3,stru_vec_name.vec_proc_name[i], db_tri_string);
        if(ret!=EXPORT_SUCCEED)
        {
            g_stru_vec_err_name.vec_proc_name.push_back(stru_vec_name.vec_proc_name[i]);
            //printf("��ȡ��ϵ���е�%s:%s����\n",flagdes[3],stru_vec_name.vec_proc_name[i].c_str());
            continue;  
        }   
        
        ret = WriteExportText(file_path,stru_vec_name.vec_proc_name[i],db_tri_string);
        if(ret != EXPORT_SUCCEED)
        {
            printf("����д��ʧ��\n");
           return EXPORT_FAIL; 
        }    
    } 

//trigger_warn
    file_path.clear();
    file_path = catalog_path + "/trigger_warn";
    for(i = 0;i < stru_vec_name.vec_trigger_name_warn.size();i++)
    {
        db_tri_string.clear();
        //��ȡ��ϵ���ж�Ӧ�Ĵ����߼�
        ret = GetTriggerTextFromDb(4,stru_vec_name.vec_trigger_name_warn[i], db_tri_string);
        if(ret!=EXPORT_SUCCEED)
        {
            g_stru_vec_err_name.vec_trigger_name_warn.push_back(stru_vec_name.vec_trigger_name_warn[i]);
            //printf("��ȡ��ϵ���е�%s:%s����\n",flagdes[4],stru_vec_name.vec_trigger_name_warn[i].c_str());
            continue;  
        }   
    
        ret = WriteExportText(file_path,stru_vec_name.vec_trigger_name_warn[i],db_tri_string);
        if(ret != EXPORT_SUCCEED)
       {
            printf("����д��ʧ��\n");
           return EXPORT_FAIL; 
        }    
    }

//function_warn
    file_path.clear();
    file_path = catalog_path + "/function_warn";
    for(i = 0;i < stru_vec_name.vec_func_name_warn.size();i++)
    {
        db_tri_string.clear();
        //��ȡ��ϵ���ж�Ӧ�Ĵ����߼�
        ret = GetTriggerTextFromDb(5,stru_vec_name.vec_func_name_warn[i], db_tri_string);
        if(ret!=EXPORT_SUCCEED)
        {
            g_stru_vec_err_name.vec_func_name_warn.push_back(stru_vec_name.vec_func_name_warn[i]);
            //printf("��ȡ��ϵ���е�%s:%s����\n",flagdes[5],stru_vec_name.vec_func_name_warn[i].c_str());
            continue;  
        }   
        
        ret = WriteExportText(file_path,stru_vec_name.vec_func_name_warn[i],db_tri_string);
        if(ret != EXPORT_SUCCEED)
        {
            printf("����д��ʧ��\n");
           return EXPORT_FAIL; 
        }    
    }
    
    //procedure_warn
        file_path.clear();
        file_path = catalog_path + "/procedure_warn";
        for(i = 0;i < stru_vec_name.vec_proc_name_warn.size();i++)
        {
            db_tri_string.clear();
            //��ȡ��ϵ���ж�Ӧ�Ĵ����߼�
            ret = GetTriggerTextFromDb(6,stru_vec_name.vec_proc_name_warn[i], db_tri_string);
            if(ret!=EXPORT_SUCCEED)
            {
                g_stru_vec_err_name.vec_proc_name_warn.push_back(stru_vec_name.vec_proc_name_warn[i]);
                //printf("��ȡ��ϵ���е�%s:%s����\n",flagdes[6],stru_vec_name.vec_proc_name_warn[i].c_str());
                continue;  
            }   
            
            ret = WriteExportText(file_path,stru_vec_name.vec_proc_name_warn[i],db_tri_string);
            if(ret != EXPORT_SUCCEED)
            {
                printf("����д��ʧ��\n");
               return EXPORT_FAIL; 
            }    
        }


return 0;
}

//�������ݿ⵼�����Զ����������ļ�
int ExportAddConf()
{
    int ret = 0;
    STRU_VECTOR_INFO stru_vec_name_db;
    StructVectorClear(stru_vec_name_db);
    
    //��ȡ��ϵ���еĴ��������洢���̡�����������
    ret=GetAllTriggerNameFromDb(stru_vec_name_db);
    if(ret!=EXPORT_SUCCEED)
    {
        if(ret==EXPORT_SUCCEED)
        {
            printf("CmpAllTrigger ��ϵ��emsģʽ��û�д��������������洢����!!!\n");
        }
        else
        {
            printf("CmpAllTrigger ��ȡ��ϵ��emsģʽ�µĴ��������������洢����ʧ��!!!\n");
            return EXPORT_FAIL;
        }
    }
    
    //�½������ļ���
    string catalog_path = g_export_path;
    ret = CreateExportCatalog(catalog_path);
    if(ret != EXPORT_SUCCEED)
    {
        printf("���������ı�Ŀ¼ʧ��\n");
        return EXPORT_FAIL;
    }

    //����ȡname д��conf�ļ���
    ret = WriteConf(stru_vec_name_db);
    if(ret != EXPORT_SUCCEED)
    {
        printf("conf�ļ�д��ʧ��\n");
        return EXPORT_FAIL;
    }
   //����struct��ȡ���벢д���½��ļ�����
   ret = WriteTextByStru(stru_vec_name_db);
   if(ret != EXPORT_SUCCEED)
   {
       printf("д������д���");
       return EXPORT_FAIL;
   }


    return EXPORT_SUCCEED;
}

//дconf�ļ�
int WriteConf(STRU_VECTOR_INFO &str_vec_name)
{
    string file_path;
    file_path = g_export_path + EXPORT_NAME_TXT + "/check_trigger.conf";

      //���ļ�
    FILE *fp=NULL;
    fp = fopen(file_path.c_str(), "w");
     if(fp==NULL)
    { 
        printf("open export file filed!\n");
        fclose(fp);
        fp=NULL;
        return EXPORT_FAIL;
    }
    //trigger
    string txt = "[trigger]\n";
    for(int i = 0;i < str_vec_name.vec_trigger_name.size();i++)
    {  
        txt = txt + "name_" + IntToStr(i)+ "= " + str_vec_name.vec_trigger_name[i] + "\n";
    }

    //function
    txt = txt + "[function]\n";
     for(int i = 0;i < str_vec_name.vec_func_name.size();i++)
    {  
        txt = txt + "name_" + IntToStr(i) + "= " + str_vec_name.vec_func_name[i] + "\n";
    }

    //procedure
    txt = txt + "[procedure]\n";
     for(int i = 0;i < str_vec_name.vec_proc_name.size();i++)
    {  
        txt = txt + "name_" + IntToStr(i) + "= " + str_vec_name.vec_proc_name[i] + "\n";
    }    
    //trigger_warn
    txt = txt + "[trigger_warn]\n";
    //function_warn
    txt = txt + "[function_warn]\n";
    //procedure_warn
    txt = txt + "[procedure_warn]\n";

    //д��
    int txt_len = txt.size();
    int write_count = fwrite(txt.c_str(),sizeof(char),txt_len,fp);
    if(write_count != txt_len)
    {
        printf("д���ļ�����ʧ��\n");
        //�ر��ļ�
        fclose(fp);
        fp=NULL;
        return EXPORT_FAIL;
    }

    //�ر��ļ�
    fclose(fp);
    fp=NULL;
    
return 0;
}

int PrintfErrInfo()
{

    //�ȶԹ����г�����Ϣ��ʾ
    if(g_err_str.size()!=0)
    {
        printf("\n####################���������еĳ�����Ϣ####################\n");
        printf("%s",g_err_str.c_str());        
        printf("####################END####################\n\n");

    }

    
    return EXPORT_SUCCEED;
}

int main(int argc, char **argv)
{
    int ret = -1;
    char server[NAME_LEN]="";
    char user_name[NAME_LEN]="";
    char passwd[NAME_LEN]="";
    char catalog_name[NAME_LEN]="";
    ErrorInfo error;
    int argc_flag = 0;
    char *endptr = NULL;

    //��������ʽ
    if(argc<5 || argc>6)
    {
        printf("Usage:%s  server  user  passwd  flag(1.���������ļ����� 2.�������ݿ⵼�����Զ����������ļ�) [����Ŀ¼����]\n",argv[0]);
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
            printf("�ȶԵ����������,����ı�ʶflagΪ:%s\n", argv[4]);
            exit(0);
        }
        //�������ı�ʶ�Ƿ�ʶ��
        if(argc_flag != 1 && argc_flag != 2)
        {
            printf("����ĵ�����ʶ��ʶ��,����ı�ʶflagΪ:%d\n", argc_flag);
        printf("Usage:%s  server  user  passwd  flag(1.���������ļ����� 2.�������ݿ⵼�����Զ����������ļ�) [����Ŀ¼����]\n",argv[0]);
            exit(0);
        }
    }

    //��ȡ��Ŀ¼
    char chHomePath[128] = "";
    strcpy(chHomePath, getenv(HOME_ENV_NAME));
    g_home_path = chHomePath;
    if(g_home_path.empty())
    {
        printf("��ȡHOME_ENV_NAMEʧ��,�޷�������������\n");
        exit(0);
    }
    int	ch_value_size = g_home_path.size();
    if(chHomePath[ch_value_size - 1] == '/')
    {
        g_home_path.erase(ch_value_size - 1, ch_value_size);
    }
    
    //���ݴ������ӹ�ϵ��
    if(!g_CDci.Connect(server, user_name, passwd, &error))
    {
        printf("���ӹ�ϵ��ʧ��,������Ϊ:%d  ������ϢΪ:%s �����ļ�:%s ������:%d\n",error.error_no,error.error_info,error.file_name,error.file_line);
        return EXPORT_FAIL;
    }


    
    if(argc == 6)
    {

        strncpy(catalog_name,argv[5],(NAME_LEN-1));
        string catalog_path = g_home_path + EXPORT_PATH;
 
        EXPORT_NAME_TXT = catalog_name;
        g_export_path = g_home_path + EXPORT_PATH;
    }
    else if(argc == 5)
    {
        g_export_path = g_home_path + EXPORT_PATH;
    }

    if(argc_flag == 1)
    {
        ret = ExportAllTrigger(1);
    }
    else if(argc_flag == 2)
    {
        ret = ExportAllTrigger(2);
    }

   PrintfErrInfo();
   if(ret == EXPORT_SUCCEED)
   {
        printf("�����ɹ�������·��Ϊ:%s\n",g_export_path.c_str());
   }
   else
   {
        printf("����ʧ��!\n");
   }

    //�Ͽ���ϵ��
    memset(&error,0,sizeof(ErrorInfo));
    if(!g_CDci.DisConnect(&error))
    {
        printf("�Ͽ���ϵ��ʧ��,������Ϊ:%d  ������ϢΪ:%s �����ļ�:%s ������:%d\n",error.error_no,error.error_info,error.file_name,error.file_line);
        return EXPORT_FAIL;
    }
    return 0;
}


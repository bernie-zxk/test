#include "export_trigger.h"

//指定路径下创建目录
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


//往指定路径下写入文件
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
//printf("路径 :%s\n",file_path.c_str());  

  //打开文件
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
        printf("写入文件内容失败");
        fclose(fp);
        fp=NULL;
        return EXPORT_FAIL;
    }
    
    //关闭文件
    fclose(fp);
    fp=NULL;

    return 0;
}

//创建导出文本标准目录
int CreateExportCatalog(string &export_path)
{
	int ret = -1;
    string catalog_path = export_path;
    string catalog_name = EXPORT_NAME_TXT;

    ret = CreateCatalog(catalog_path,catalog_name);
    if(ret != EXPORT_SUCCEED)
    {
        printf("创建导出目录失败\n");
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
//1.基于配置文件导出 2.基于数据库导出并自动生成配置文件
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
        printf("1.基于配置文件导出 2.基于数据库导出并自动生成配置文件\n");
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
    
    //获取配置文件中配置的   触发器    函数    存储过程
    ret = GetAllConfigInfo(stru_vec_name_conf);
    if(ret!= EXPORT_SUCCEED)
    {
        printf("CmpAllTrigger 获取配置文件所有内容失败!!!\n");
        return EXPORT_FAIL;
    }

    //新建导出文件夹
    string catalog_path = g_export_path;
    ret = CreateExportCatalog(catalog_path);
    if(ret != EXPORT_SUCCEED)
    {
        printf("创建导出文本目录失败\n");
        return EXPORT_FAIL;
    }

        //拷贝conf文件到新建文件夹
    string file_path = g_home_path + CONF_PATH;
    string backup_file_path = g_export_path + EXPORT_NAME_TXT;
    string command_str = "cp " + file_path + "check_trigger.conf " + backup_file_path;
//printf("拷贝路径：%s\n",command_str.c_str());
    ret = ServiceExcute(command_str.c_str());
    if(ret != EXPORT_SUCCEED)
    {
        printf("拷贝conf文件失败!\n");
        return EXPORT_FAIL;
    }
    else
    {
        printf("拷贝conf文件成功!\n");
    }
    ////根据struct获取代码并写入新建文件夹中

    ret = WriteTextByStru(stru_vec_name_conf);
    if(ret != EXPORT_SUCCEED)
    {
        printf("代码文本写入过程有错误\n");
        return EXPORT_FAIL;
    }

  
    return EXPORT_SUCCEED;
}



//根据struct获取代码并写入新建文件夹中
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
        //获取关系库中对应的代码逻辑
        ret = GetTriggerTextFromDb(1,stru_vec_name.vec_trigger_name[i], db_tri_string);
        if(ret!=EXPORT_SUCCEED)
        {   
            g_stru_vec_err_name.vec_trigger_name.push_back(stru_vec_name.vec_trigger_name[i]);
            //printf("获取关系库中的%s:%s出错\n",flagdes[1],stru_vec_name.vec_trigger_name[i].c_str());
            continue;
        }   
        
        ret = WriteExportText(file_path,stru_vec_name.vec_trigger_name[i],db_tri_string);
        if(ret != EXPORT_SUCCEED)
        {
            printf("代码写入失败\n");
            
           return EXPORT_FAIL; 
        }
   
        
    }

//FUNCTION
    file_path.clear();
    file_path = catalog_path + "/function";
    for(i = 0;i < stru_vec_name.vec_func_name.size();i++)
    {
        db_tri_string.clear();
        //获取关系库中对应的代码逻辑
        ret = GetTriggerTextFromDb(2,stru_vec_name.vec_func_name[i], db_tri_string);
        if(ret!=EXPORT_SUCCEED)
        {
            g_stru_vec_err_name.vec_func_name.push_back(stru_vec_name.vec_func_name[i]);
           // printf("获取关系库中的%s:%s出错\n",flagdes[2],stru_vec_name.vec_func_name[i].c_str());
			  continue;
        }   
        
        ret = WriteExportText(file_path,stru_vec_name.vec_func_name[i],db_tri_string);
        if(ret != EXPORT_SUCCEED)
        {
            printf("代码写入失败\n");
           return EXPORT_FAIL; 
        }
    
        
    }
    
//procedure
    file_path.clear();
    file_path = catalog_path + "/procedure";
    for(i = 0;i < stru_vec_name.vec_proc_name.size();i++)
    {
        db_tri_string.clear();
        //获取关系库中对应的代码逻辑
        ret = GetTriggerTextFromDb(3,stru_vec_name.vec_proc_name[i], db_tri_string);
        if(ret!=EXPORT_SUCCEED)
        {
            g_stru_vec_err_name.vec_proc_name.push_back(stru_vec_name.vec_proc_name[i]);
            //printf("获取关系库中的%s:%s出错\n",flagdes[3],stru_vec_name.vec_proc_name[i].c_str());
            continue;  
        }   
        
        ret = WriteExportText(file_path,stru_vec_name.vec_proc_name[i],db_tri_string);
        if(ret != EXPORT_SUCCEED)
        {
            printf("代码写入失败\n");
           return EXPORT_FAIL; 
        }    
    } 

//trigger_warn
    file_path.clear();
    file_path = catalog_path + "/trigger_warn";
    for(i = 0;i < stru_vec_name.vec_trigger_name_warn.size();i++)
    {
        db_tri_string.clear();
        //获取关系库中对应的代码逻辑
        ret = GetTriggerTextFromDb(4,stru_vec_name.vec_trigger_name_warn[i], db_tri_string);
        if(ret!=EXPORT_SUCCEED)
        {
            g_stru_vec_err_name.vec_trigger_name_warn.push_back(stru_vec_name.vec_trigger_name_warn[i]);
            //printf("获取关系库中的%s:%s出错\n",flagdes[4],stru_vec_name.vec_trigger_name_warn[i].c_str());
            continue;  
        }   
    
        ret = WriteExportText(file_path,stru_vec_name.vec_trigger_name_warn[i],db_tri_string);
        if(ret != EXPORT_SUCCEED)
       {
            printf("代码写入失败\n");
           return EXPORT_FAIL; 
        }    
    }

//function_warn
    file_path.clear();
    file_path = catalog_path + "/function_warn";
    for(i = 0;i < stru_vec_name.vec_func_name_warn.size();i++)
    {
        db_tri_string.clear();
        //获取关系库中对应的代码逻辑
        ret = GetTriggerTextFromDb(5,stru_vec_name.vec_func_name_warn[i], db_tri_string);
        if(ret!=EXPORT_SUCCEED)
        {
            g_stru_vec_err_name.vec_func_name_warn.push_back(stru_vec_name.vec_func_name_warn[i]);
            //printf("获取关系库中的%s:%s出错\n",flagdes[5],stru_vec_name.vec_func_name_warn[i].c_str());
            continue;  
        }   
        
        ret = WriteExportText(file_path,stru_vec_name.vec_func_name_warn[i],db_tri_string);
        if(ret != EXPORT_SUCCEED)
        {
            printf("代码写入失败\n");
           return EXPORT_FAIL; 
        }    
    }
    
    //procedure_warn
        file_path.clear();
        file_path = catalog_path + "/procedure_warn";
        for(i = 0;i < stru_vec_name.vec_proc_name_warn.size();i++)
        {
            db_tri_string.clear();
            //获取关系库中对应的代码逻辑
            ret = GetTriggerTextFromDb(6,stru_vec_name.vec_proc_name_warn[i], db_tri_string);
            if(ret!=EXPORT_SUCCEED)
            {
                g_stru_vec_err_name.vec_proc_name_warn.push_back(stru_vec_name.vec_proc_name_warn[i]);
                //printf("获取关系库中的%s:%s出错\n",flagdes[6],stru_vec_name.vec_proc_name_warn[i].c_str());
                continue;  
            }   
            
            ret = WriteExportText(file_path,stru_vec_name.vec_proc_name_warn[i],db_tri_string);
            if(ret != EXPORT_SUCCEED)
            {
                printf("代码写入失败\n");
               return EXPORT_FAIL; 
            }    
        }


return 0;
}

//基于数据库导出并自动生成配置文件
int ExportAddConf()
{
    int ret = 0;
    STRU_VECTOR_INFO stru_vec_name_db;
    StructVectorClear(stru_vec_name_db);
    
    //获取关系库中的触发器、存储过程、函数的名称
    ret=GetAllTriggerNameFromDb(stru_vec_name_db);
    if(ret!=EXPORT_SUCCEED)
    {
        if(ret==EXPORT_SUCCEED)
        {
            printf("CmpAllTrigger 关系库ems模式下没有触发器、函数、存储过程!!!\n");
        }
        else
        {
            printf("CmpAllTrigger 获取关系库ems模式下的触发器、函数、存储过程失败!!!\n");
            return EXPORT_FAIL;
        }
    }
    
    //新建导出文件夹
    string catalog_path = g_export_path;
    ret = CreateExportCatalog(catalog_path);
    if(ret != EXPORT_SUCCEED)
    {
        printf("创建导出文本目录失败\n");
        return EXPORT_FAIL;
    }

    //将获取name 写入conf文件中
    ret = WriteConf(stru_vec_name_db);
    if(ret != EXPORT_SUCCEED)
    {
        printf("conf文件写入失败\n");
        return EXPORT_FAIL;
    }
   //根据struct获取代码并写入新建文件夹中
   ret = WriteTextByStru(stru_vec_name_db);
   if(ret != EXPORT_SUCCEED)
   {
       printf("写入过程有错误");
       return EXPORT_FAIL;
   }


    return EXPORT_SUCCEED;
}

//写conf文件
int WriteConf(STRU_VECTOR_INFO &str_vec_name)
{
    string file_path;
    file_path = g_export_path + EXPORT_NAME_TXT + "/check_trigger.conf";

      //打开文件
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

    //写入
    int txt_len = txt.size();
    int write_count = fwrite(txt.c_str(),sizeof(char),txt_len,fp);
    if(write_count != txt_len)
    {
        printf("写入文件内容失败\n");
        //关闭文件
        fclose(fp);
        fp=NULL;
        return EXPORT_FAIL;
    }

    //关闭文件
    fclose(fp);
    fp=NULL;
    
return 0;
}

int PrintfErrInfo()
{

    //比对过程中出错信息提示
    if(g_err_str.size()!=0)
    {
        printf("\n####################导出过程中的出错信息####################\n");
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

    //检查输入格式
    if(argc<5 || argc>6)
    {
        printf("Usage:%s  server  user  passwd  flag(1.基于配置文件导出 2.基于数据库导出并自动生成配置文件) [导出目录名称]\n",argv[0]);
        exit(0);
    }
    strncpy(server,argv[1],(NAME_LEN-1));
    strncpy(user_name,argv[2],(NAME_LEN-1));
    strncpy(passwd,argv[3],(NAME_LEN-1));
    if(argc>4)
    {
        argc_flag = (int)strtol(argv[4], &endptr, 0); //获取表号限制标识
        if(strlen(endptr) != 0)
        {
            printf("比对导出输入出错,输入的标识flag为:%s\n", argv[4]);
            exit(0);
        }
        //检测输入的标识是否识别
        if(argc_flag != 1 && argc_flag != 2)
        {
            printf("输入的导出标识不识别,输入的标识flag为:%d\n", argc_flag);
        printf("Usage:%s  server  user  passwd  flag(1.基于配置文件导出 2.基于数据库导出并自动生成配置文件) [导出目录名称]\n",argv[0]);
            exit(0);
        }
    }

    //获取家目录
    char chHomePath[128] = "";
    strcpy(chHomePath, getenv(HOME_ENV_NAME));
    g_home_path = chHomePath;
    if(g_home_path.empty())
    {
        printf("获取HOME_ENV_NAME失败,无法启动发布服务\n");
        exit(0);
    }
    int	ch_value_size = g_home_path.size();
    if(chHomePath[ch_value_size - 1] == '/')
    {
        g_home_path.erase(ch_value_size - 1, ch_value_size);
    }
    
    //根据传参连接关系库
    if(!g_CDci.Connect(server, user_name, passwd, &error))
    {
        printf("连接关系库失败,错误码为:%d  错误信息为:%s 出错文件:%s 出错行:%d\n",error.error_no,error.error_info,error.file_name,error.file_line);
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
        printf("导出成功，导出路径为:%s\n",g_export_path.c_str());
   }
   else
   {
        printf("导出失败!\n");
   }

    //断开关系库
    memset(&error,0,sizeof(ErrorInfo));
    if(!g_CDci.DisConnect(&error))
    {
        printf("断开关系库失败,错误码为:%d  错误信息为:%s 出错文件:%s 出错行:%d\n",error.error_no,error.error_info,error.file_name,error.file_line);
        return EXPORT_FAIL;
    }
    return 0;
}


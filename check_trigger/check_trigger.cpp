#include "check_trigger.h"


//自定义结构体单个成员比对
int StructMemberCmp(ALTER vector <string> &vec_conf,ALTER vector <string> &vec_db,OUT vector <string> &vec_com,OUT vector <string> &vec_err);
//自定义结构体比对
int StructVectorCmp(IN STRU_VECTOR_INFO &stu_vec_name_conf,IN STRU_VECTOR_INFO &stu_vec_name_db,OUT STRU_VECTOR_INFO &stu_vec_name_com);
//划分代码段，划分两部分   一部分头   一部分程序逻辑   以BEGIN为分隔
int AnalyseTri(IN string name,IN int flag,OUT string &head_db_string,ALTER string &db_string,OUT string &head_file_string,ALTER string &file_string);
//比对关系库和标准库中的触发器
int CmpDbAndFileTri(IN string name,IN int flag,IN string &db_tri_string,IN string file_tri_string);
//比对指定触发器代码逻辑
//flag 为1表示为触发器错误比对4表示触发器警告比对2 表示函数错误比对5表示函数警告比对
// 3 表示存储过程错误比对  6表示存储过程警告比对
int CmpOneTrigger(IN string name,IN int flag);
//比对多个触发器或函数或存储过程
//flag 为1表示为触发器错误比对4表示触发器警告比对2 表示函数错误比对5表示函数警告比对
// 3 表示存储过程错误比对  6表示存储过程警告比对
int CmpPartTrigger(IN vector<string> &vec_name, IN int flag);
//比对全部触发器
int CmpAllTrigger();
//打印结构体成员
int PrintfStruct(STRU_VECTOR_INFO &stru_vec_name,int flag = PRINT_OTHER);
//打印错误信息
int PrintfErrInfo();


//结构体中单个成员的比对     vec_com记录关系库和标准库里都存在的触发器等   
//vec_err记录仅存在标准库的触发器等
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
        //如果没找到则该触发器、函数、存储过程仅在标准里存在
        if(findflag!=1)
        {
            vec_err.push_back(vec_conf[i]);
        }
    }
    return CHECK_SUCCEED;
}

//结构体比对
int StructVectorCmp(IN STRU_VECTOR_INFO &stu_vec_name_conf,IN STRU_VECTOR_INFO &stu_vec_name_db,OUT STRU_VECTOR_INFO &stu_vec_name_com)
{
    //关系库中的触发器存在于vec_name_db的vec_trigger_name成员内
    //关系库中的存储过程和函数存在与vec_name_db的vec_func_name成员内
    STRU_VECTOR_INFO tmp_stru_vec_name_conf;
    STRU_VECTOR_INFO tmp_stru_vec_name_db;
    StructVectorClear(tmp_stru_vec_name_conf);
    StructVectorClear(tmp_stru_vec_name_db);
    
    StructVectorCpy(tmp_stru_vec_name_conf,stu_vec_name_conf);
    StructVectorCpy(tmp_stru_vec_name_db,stu_vec_name_db);

    //比对触发器   记录关系库和标准里都存在的触发器及仅在标准里存在的触发器
    StructMemberCmp(tmp_stru_vec_name_conf.vec_trigger_name,tmp_stru_vec_name_db.vec_trigger_name,
                        stu_vec_name_com.vec_trigger_name,g_stru_vec_only_conf.vec_trigger_name);
    
    //比对函数
    StructMemberCmp(tmp_stru_vec_name_conf.vec_func_name,tmp_stru_vec_name_db.vec_func_name,
                        stu_vec_name_com.vec_func_name,g_stru_vec_only_conf.vec_func_name);
    
    //比对存储过程
    StructMemberCmp(tmp_stru_vec_name_conf.vec_proc_name,tmp_stru_vec_name_db.vec_func_name,
                        stu_vec_name_com.vec_proc_name,g_stru_vec_only_conf.vec_proc_name);
    
    //比对警告触发器
    StructMemberCmp(tmp_stru_vec_name_conf.vec_trigger_name_warn,tmp_stru_vec_name_db.vec_trigger_name,
                        stu_vec_name_com.vec_trigger_name_warn,g_stru_vec_only_conf.vec_trigger_name_warn);

    //警告函数
    StructMemberCmp(tmp_stru_vec_name_conf.vec_func_name_warn,tmp_stru_vec_name_db.vec_func_name,
                        stu_vec_name_com.vec_func_name_warn,g_stru_vec_only_conf.vec_func_name_warn);
    
    //警告存储过程
    StructMemberCmp(tmp_stru_vec_name_conf.vec_proc_name_warn,tmp_stru_vec_name_db.vec_func_name,
                        stu_vec_name_com.vec_proc_name_warn,g_stru_vec_only_conf.vec_proc_name_warn);
    //获取仅在关系库存在的触发器、函数、存储过程
    StructVectorCpy(g_stru_vec_only_db,tmp_stru_vec_name_db);
    
    return CHECK_SUCCEED;
}

//划分代码段，划分两部分   一部分头   一部分程序逻辑   以BEGIN为分隔
int AnalyseTri(IN string name,IN int flag,OUT string &head_db_string,ALTER string &db_string,OUT string &head_file_string,ALTER string &file_string)
{
    char err_char[1024];
    int pos=0;
    int len=0;
    head_db_string = db_string;
    head_file_string = file_string;
    //关系库中的代码处理
    //去除头
    //找到分割位置 begin
    pos = db_string.find("begin",0);
    if(pos == db_string.npos)
    {
        sprintf(err_char,"%s:%s  调用AnalyseTri函数时对关系库中%s代码进行分割时，未找到'begin'分割位置!!!\n",\
                    flagdes[flag],name.c_str(),name.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
        return CHECK_FAIL;
    }
    //begin  分割位置前字符串的长度
    len = pos;
    db_string.erase(0,len);

    
    //去除尾
    pos=0;
    len=0;
    //找到分割位置 begin
    pos = head_db_string.find("begin",0);
    if(pos == head_db_string.npos)
    {
        sprintf(err_char,"%s:%s  调用AnalyseTri函数时对关系库中%s代码进行分割时，未找到'begin'分割位置!!!\n",\
                    flagdes[flag],name.c_str(),name.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
        return CHECK_FAIL;
    }
    //begin  分割位置后的字符串全部删除
    head_db_string.erase(pos,head_db_string.size());

    //标准库中的代码处理
       //去除头
        //找到分割位置 begin
    pos = file_string.find("begin",0);
    if(pos == file_string.npos)
    {
        sprintf(err_char,"%s:%s  调用AnalyseTri函数时对标准库中%s代码进行分割时，未找到'begin'分割位置!!!\n",\
                    flagdes[flag],name.c_str(),name.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
        return CHECK_FAIL;
    }
        //begin  分割位置前字符串的长度
    len = pos;
    file_string.erase(0,len);
           
      //去除尾
    pos=0;
    len=0;
      //找到分割位置 begin
    pos = head_file_string.find("begin",0);
    if(pos == head_file_string.npos)
    {
        sprintf(err_char,"%s:%s  调用AnalyseTri函数时对关系库中%s代码进行分割时，未找到'begin'分割位置!!!\n",\
                    flagdes[flag],name.c_str(),name.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
        return CHECK_FAIL;
    }
      //begin  分割位置后的字符串全部删除
    head_file_string.erase(pos,head_file_string.size());
    
    return CHECK_SUCCEED;
}
//比对关系库和标准库中的触发器
int CmpDbAndFileTri(IN string name,IN int flag,IN string &db_tri_string,IN string file_tri_string)
{
    char err_char[1024];
    string head_db_tri_string;
    string head_file_tri_string;
    string tmp_db_tri_string = db_tri_string;
    string tmp_file_tri_string = file_tri_string;
    int ret;
    
    //去除两部分的注释行
    RmComment(tmp_db_tri_string);
    RmComment(tmp_file_tri_string);
    
   // printf("关系库  去掉注释 len=%d, txt=%s\n",tmp_db_tri_string.size(),tmp_db_tri_string.c_str());
    
   // printf("标准库  去掉注释 len=%d, txt=%s\n",tmp_file_tri_string.size(),tmp_file_tri_string.c_str());

    //将字符转换成小写
    //关系库
    char *data_char=NULL;
    data_char =(char *)malloc(sizeof(char) * tmp_db_tri_string.size()+1);
    if(data_char==NULL)
    {
        sprintf(err_char,"%s:%s  调用CmpDbAndFileTri函数时对关系库中%s代码进行小写转换时,申请空间失败!!!\n",\
                    flagdes[flag],name.c_str(),name.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
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

    //标准库
    data_char =(char *)malloc(sizeof(char) * tmp_file_tri_string.size()+1);
    if(data_char==NULL)
    {
        sprintf(err_char,"%s:%s  调用CmpDbAndFileTri函数时对标准库%s代码进行小写转换时,申请空间失败!!!\n",\
                    flagdes[flag],name.c_str(),name.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
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
   // printf("关系库  转换小写 len=%d, txt=%s\n",tmp_db_tri_string.size(),tmp_db_tri_string.c_str());
    
   // printf("标准库  转换小写 len=%d, txt=%s\n",tmp_file_tri_string.size(),tmp_file_tri_string.c_str());
    
    
    //划分两部分   一部分头   一部分程序逻辑   以BEGIN为分隔
    ret=AnalyseTri(name,flag,head_db_tri_string,tmp_db_tri_string,head_file_tri_string,tmp_file_tri_string);
    if(ret!=CHECK_SUCCEED)
    {
        printf("分割标准库、关系库中的代码段时出错!!!\n");
        return CHECK_FAIL;
    }
    //头部部分不进行比对,只比对代码逻辑部分,认为逻辑一致即认为触发器相同
    //head_db_tri_string  head_file_tri_string暂时不处理
    
   // printf("关系库  头 len=%d, txt=%s\n",head_db_tri_string.size(),head_db_tri_string.c_str());
   // printf("关系库  逻辑体 len=%d, txt=%s\n",tmp_db_tri_string.size(),tmp_db_tri_string.c_str());
   // printf("标准库  头 len=%d, txt=%s\n",head_file_tri_string.size(),head_file_tri_string.c_str());
   // printf("标准库  逻辑体 len=%d, txt=%s\n",tmp_file_tri_string.size(),tmp_file_tri_string.c_str());

    //开始处理代码逻辑段比对
        //去除两部分的空格 回车   tab等
    RmSpaceAndTab(tmp_db_tri_string);
    RmSpaceAndTab(tmp_file_tri_string);

    //如果代码中最后一个字符为";"号则去掉,
    //去掉最后一个分号   原因触发器等代码最后加不加;号都可以编译成功
    //为了避免最后一个;号不匹配的问题则统一去掉
    RmLastSemicolon(tmp_db_tri_string);
    RmLastSemicolon(tmp_file_tri_string);
    
    //printf("关系库 %s:%s  逻辑体 去掉回车和空格  len=%d, txt=%s\n",flagdes[flag],name.c_str(),tmp_db_tri_string.size(),tmp_db_tri_string.c_str());
    //printf("标准库 %s:%s  逻辑体 去掉回车和空格  len=%d, txt=%s\n",flagdes[flag],name.c_str(),tmp_file_tri_string.size(),tmp_file_tri_string.c_str());
        //按字符串进行比对
    if(tmp_db_tri_string != tmp_file_tri_string)
    {
        StorageStructMember(g_stru_vec_diff_name,name,flag);
    }
    //组建反馈信息
    return CHECK_SUCCEED;
}

//比对指定触发器代码逻辑
//flag 为1表示为触发器错误比对4表示触发器警告比对2 表示函数错误比对5表示函数警告比对
// 3 表示存储过程错误比对  6表示存储过程警告比对
int CmpOneTrigger(IN string name,IN int flag)
{
    string db_tri_string;
    string file_tri_string;

    //获取关系库中对应的代码逻辑
    int ret = GetTriggerTextFromDb(flag,name, db_tri_string);
    if(ret!=CHECK_SUCCEED)
    {
        printf("获取关系库中的%s:%s出错\n",flagdes[flag],name.c_str());
        return CHECK_FAIL;
    }

    //获取标准库里对应的代码逻辑
    ret = GetTriggerTextFromFile(flag,name, file_tri_string);
    if(ret!=CHECK_SUCCEED)
    {
        printf("获取标准库中的%s:%s出错\n",flagdes[flag],name.c_str());
        return CHECK_FAIL;
    }

    
    //printf("关系库  len=%d, txt=%s\n",db_tri_string.size(),db_tri_string.c_str());
    
    //printf("标准库  len=%d, txt=%s\n",file_tri_string.size(),file_tri_string.c_str());
    //比对两部分代码是否一致
    ret = CmpDbAndFileTri(name,flag,db_tri_string, file_tri_string);
    if(ret!=CHECK_SUCCEED)
    {
        printf("比对%s:%s出错\n",flagdes[flag],name.c_str());
        return CHECK_FAIL;
    }
    //组建反馈信息
   return CHECK_SUCCEED; 
}
//比对多个触发器或函数或存储过程
//flag 为1表示为触发器错误比对4表示触发器警告比对2 表示函数错误比对5表示函数警告比对
// 3 表示存储过程错误比对  6表示存储过程警告比对
int CmpPartTrigger(IN vector<string> &vec_name, IN int flag)
{
    int i;
    string tmp_name;
    //循环调用触发器的比对
    for(i=0;i<vec_name.size();i++)
    {
        tmp_name=vec_name[i];
        CmpOneTrigger(tmp_name,flag);
    }
	return CHECK_SUCCEED;
}

//比对全部触发器
int CmpAllTrigger()
{
	int ret;

    STRU_VECTOR_INFO stru_vec_name_conf;
    STRU_VECTOR_INFO stru_vec_name_db;//用到该结构体第一个成员vec_trigger_name记录关系库中的触发器，
                            //第二个成员vec_func_name记录函数和存储过程
    STRU_VECTOR_INFO stru_vec_name_com;//记录关系库与标准中都存在的触发器、函数、存储过程

    StructVectorClear(stru_vec_name_conf);
    StructVectorClear(stru_vec_name_db);
    StructVectorClear(stru_vec_name_com);
    
    //vec_trigger_name.push_back((string)"TI_ACLINEEND");
    
    //获取配置文件中的标准触发器名
    string prefix_rep_name = "name_";
    string section_name;
    //获取配置文件中配置的触发器 函数  存储过程
    ret=GetAllConfigInfo(stru_vec_name_conf);
    if(ret!=CHECK_SUCCEED)
    {
        printf("CmpAllTrigger 获取配置文件所有内容失败!!!\n");
        return CHECK_FAIL;
    }
    //获取关系库中的触发器、存储过程、函数的名称
    ret=GetAllTriggerNameFromDb(stru_vec_name_db);
    if(ret!=CHECK_SUCCEED)
    {
        if(ret==CHECK_NOEXIST)
        {
            printf("CmpAllTrigger 关系库ems模式下没有触发器、函数、存储过程!!!\n");
        }
        else
        {
            printf("CmpAllTrigger 获取关系库ems模式下的触发器、函数、存储过程失败!!!\n");
            return CHECK_FAIL;
        }
    }
    //比对关系库与标准中触发器数量上的差异,并记录仅存在关系库或标准里的触发器等
    
    //比对关系库中的触发器等数量与标准里的触发器等数量是否一致
    //配置的数量
    //int sum_conf=stru_vec_name_conf.vec_trigger_name.size()+stru_vec_name_conf.vec_func_name.size()+stru_vec_name_conf.vec_proc_name.size()\
      //          +stru_vec_name_conf.vec_trigger_name_warn.size()+stru_vec_name_conf.vec_func_name_warn.size()+stru_vec_name_conf.vec_proc_name_warn.size();
    int sum_conf = CountStruMemberNum(stru_vec_name_conf);
    //关系库中的数量
    //int sum_db=stru_vec_name_db.vec_trigger_name.size()+stru_vec_name_db.vec_func_name.size();
    int sum_db = CountStruMemberNum(stru_vec_name_db);

    if(sum_conf==0)
    {
        if(sum_db==0)
        {
            printf("CmpAllTrigger 配置文件和关系库ems模式中的触发器、函数、存储过程数量均为0,无需比对!!!\n");
            return CHECK_SUCCEED;
        }
        else
        {
            printf("CmpAllTrigger 配置文件中配置的触发器、函数、存储过程数量为0,关系库ems下触发器、函数、存储过程数量为%d\n",sum_db);
            StructVectorCpy(g_stru_vec_only_db,stru_vec_name_db);
            return CHECK_SUCCEED;
        }
    }
    else
    {
        if(sum_db==0)
        {
            printf("CmpAllTrigger 关系库ems下触发器、函数、存储过程数量为0,配置文件中配置的触发器、函数、存储过程数量为%d\n",sum_conf);
            StructVectorCpy(g_stru_vec_only_conf,stru_vec_name_conf);
            return CHECK_SUCCEED;
        }
    }
    //比对找出仅存在关系库或标准里的触发器
    ret=StructVectorCmp(stru_vec_name_conf,stru_vec_name_db,stru_vec_name_com);
    if(ret!=CHECK_SUCCEED)
    {
        printf("CmpAllTrigger 比对关系库与标准中的触发器、函数、存储过程数量差异出错!!!\n");
        return CHECK_FAIL;
    }

    

    //比对代码逻辑
    //比对分为错误和警告两种提示
    //flag 为1表示为触发器错误比对4表示触发器警告比对2 表示函数错误比对5表示函数警告比对
    // 3表示存储过程错误比对  6表示存储过程警告比对
    //比对触发器
    ret=CmpPartTrigger(stru_vec_name_com.vec_trigger_name,FLAG_TRI_ERR);
    if(ret!=CHECK_SUCCEED)
    {
        printf("比对触发器时出错!!!\n");
        //return CHECK_FAIL;
    }
    //比对函数
    ret=CmpPartTrigger(stru_vec_name_com.vec_func_name,FLAG_FUNC_ERR);
    if(ret!=CHECK_SUCCEED)
    {
        printf("比对函数时出错!!!\n");
        //return CHECK_FAIL;
    }
    //比对存储过程
    ret=CmpPartTrigger(stru_vec_name_com.vec_proc_name,FLAG_PROC_ERR);
    if(ret!=CHECK_SUCCEED)
    {
        printf("比对存储过程时出错!!!\n");
        //return CHECK_FAIL;
    }
    //比对警告触发器
    ret=CmpPartTrigger(stru_vec_name_com.vec_trigger_name_warn,FLAG_TRI_WARN);
    if(ret!=CHECK_SUCCEED)
    {
        printf("比对警告触发器时出错!!!\n");
        //return CHECK_FAIL;
    }
    //比对警告函数
    ret=CmpPartTrigger(stru_vec_name_com.vec_func_name_warn,FLAG_FUNC_WARN);
    if(ret!=CHECK_SUCCEED)
    {
        printf("比对警告函数时出错!!!\n");
        //return CHECK_FAIL;
    }
    //比对警告存储过程
    ret=CmpPartTrigger(stru_vec_name_com.vec_proc_name_warn,FLAG_PROC_WARN);
    if(ret!=CHECK_SUCCEED)
    {
        printf("比对警告存储过程时出错!!!\n");
        //return CHECK_FAIL;
    }
    
	return CHECK_SUCCEED;
}

//打印结构体成员
int PrintfStruct(STRU_VECTOR_INFO &stru_vec_name,int flag = PRINT_OTHER)
{
    int i;
    if(stru_vec_name.vec_trigger_name.size()>0)
    {
        printf("**********触发器**********\n");
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
            printf("**********函数或存储过程**********\n");
        }
        else
        {
            printf("**********函数**********\n");
        }
        for(i=0;i<stru_vec_name.vec_func_name.size();i++)
        {
            printf("%d.%s ",(i+1),stru_vec_name.vec_func_name[i].c_str());
        }
        printf("\n");
    }

    if(stru_vec_name.vec_proc_name.size()>0)
    {
        printf("**********存储过程**********\n");
        for(i=0;i<stru_vec_name.vec_proc_name.size();i++)
        {
            printf("%d.%s ",(i+1),stru_vec_name.vec_proc_name[i].c_str());
        }
        printf("\n");
    }
    if(stru_vec_name.vec_trigger_name_warn.size()>0)
    {
        printf("**********触发器(警告)**********\n");
        for(i=0;i<stru_vec_name.vec_trigger_name_warn.size();i++)
        {
            printf("%d.%s ",(i+1),stru_vec_name.vec_trigger_name_warn[i].c_str());
        }
        printf("\n");
    }
    if(stru_vec_name.vec_func_name_warn.size())
    {
        printf("**********函数(警告)**********\n");
        for(i=0;i<stru_vec_name.vec_func_name_warn.size();i++)
        {
            printf("%d.%s ",(i+1),stru_vec_name.vec_func_name_warn[i].c_str());
        }
        printf("\n");
    }

    if(stru_vec_name.vec_proc_name_warn.size()>0)
    {
        printf("**********存储过程(警告):**********\n");
        for(i=0;i<stru_vec_name.vec_proc_name_warn.size();i++)
        {
            printf("%d.%s ",(i+1),stru_vec_name.vec_proc_name_warn[i].c_str());
        }
        printf("\n");
    }
    return CHECK_SUCCEED;
}




//打印错误信息
int PrintfErrInfo()
{
    int succeed_flag=1;
    
    //仅存在关系库中的触发器提示
    if(CountStruMemberNum(g_stru_vec_only_db)>0)
    {
        printf("\n####################仅存在关系库中####################\n");
        PrintfStruct(g_stru_vec_only_db,PRINT_ONLY_DB);
        printf("####################END####################\n\n");
        succeed_flag=0;
    }

    //仅存在标准库中的触发器提示
    if(CountStruMemberNum(g_stru_vec_only_conf)>0)
    {
        printf("\n####################仅存在标准库中####################\n");
        PrintfStruct(g_stru_vec_only_conf);
        printf("####################END####################\n\n");
        succeed_flag=0;
    }

    //比对结果不相同的触发器提示
    if(CountStruMemberNum(g_stru_vec_diff_name)>0)
    {
        printf("\n####################比对结果不相同####################\n");
        PrintfStruct(g_stru_vec_diff_name);
        printf("####################END####################\n\n");
        succeed_flag=0;
    }

    //比对过程中出错的触发器提示
    if(CountStruMemberNum(g_stru_vec_err_name)>0)
    {
        printf("\n####################比对过程中出错####################\n");
        PrintfStruct(g_stru_vec_err_name);
        printf("####################END####################\n\n");
        succeed_flag=0;
    }

    //比对过程中出错信息提示
    if(g_err_str.size()!=0)
    {
        printf("\n####################比对过程中的出错信息####################\n");
        printf("%s",g_err_str.c_str());        
        printf("####################END####################\n\n");
        succeed_flag=0;
    }
    if(succeed_flag == 1)
    {
        printf("#############################################\n");
        printf("##                                         ##\n");
        printf("##  触发器、函数、存储过程比对匹配成功!!!  ##\n");        
        printf("##                                         ##\n");
        printf("#############################################\n");
    }
    else
    {
        printf("#############################################\n");
        printf("##                                         ##\n");
        printf("##      请  检  测  比  对  结  果!!!      ##\n");        
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

    //全局变量初始化清空
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
        argc_flag = (int)strtol(argv[4], &endptr, 0); //获取表号限制标识
        if(strlen(endptr) != 0)
        {
            printf("比对标识输入出错,输入的标识flag为:%s\n", argv[4]);
            exit(0);
        }
        //检测输入的标识是否识别
        if(argc_flag != FLAG_TRI_ERR && argc_flag != FLAG_FUNC_ERR && argc_flag != FLAG_PROC_ERR)
        {
            printf("输入的比对标识不识别,输入的标识flag为:%d\n", argc_flag);
            exit(0);
        }
        string tmp_string;
        char tmp_char[NAME_LEN];
        for(i=5;i<argc;i++)
        {
            memset(tmp_char,0,NAME_LEN);
            strncpy(tmp_char,argv[i],NAME_LEN-1);
            //将名字转换为大写
            Check_Toupper(tmp_char);
            tmp_string = tmp_char;
            argv_vec_name.push_back(tmp_string);
        }
    }

    //获取家目录
    char chHomePath[128] = "";
    strcpy(chHomePath, getenv(HOME_ENV_NAME));
    g_strHomePath = chHomePath;
    if(g_strHomePath.empty())
    {
        printf("获取HOME_ENV_NAME失败,无法启动发布服务");
        exit(0);
    }
    int	ch_value_size = g_strHomePath.size();
    if(chHomePath[ch_value_size - 1] == '/')
    {
        g_strHomePath.erase(ch_value_size - 1, ch_value_size);
    }
    
    //根据传参连接关系库
    if(!g_CDci.Connect(server, user_name, passwd, &error))
    {
        printf("连接关系库失败,错误码为:%d  错误信息为:%s 出错文件:%s 出错行:%d\n",error.error_no,error.error_info,error.file_name,error.file_line);
        return CHECK_FAIL;
    }

    //比对指定多个触发器
    if(argc>4)
    {
    	CmpPartTrigger(argv_vec_name,argc_flag);
    }
	//比对配置文件中的触发器
    else
    {
    	CmpAllTrigger();
    }

    //错误信息处理
    PrintfErrInfo();

    //断开关系库
    memset(&error,0,sizeof(ErrorInfo));
    if(!g_CDci.DisConnect(&error))
    {
        printf("断开关系库失败,错误码为:%d  错误信息为:%s 出错文件:%s 出错行:%d\n",error.error_no,error.error_info,error.file_name,error.file_line);
        return CHECK_FAIL;
    }
    return 0;
}


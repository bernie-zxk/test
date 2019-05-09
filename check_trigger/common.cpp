#include "common.h"
//数值转字符
string IntToStr(int value)
{
    //将int值转换为string类型
    string ret_str;
    char buff[12];//留一个/0位
    sprintf(buff, "%d", value);
    ret_str = buff;
    return ret_str;
}

//字母小写转换大写
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
//字母大写转小写
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
//结构体清空
int StructVectorClear(ALTER STRU_VECTOR_INFO &stru)
{
    //清空各个成员
    stru.vec_trigger_name.clear();
    stru.vec_func_name.clear();
    stru.vec_proc_name.clear();
    stru.vec_trigger_name_warn.clear();
    stru.vec_func_name_warn.clear();
    stru.vec_proc_name_warn.clear();
    return CHECK_SUCCEED;
}



//定义的vector结构体的拷贝
int StructVectorCpy(OUT STRU_VECTOR_INFO &dest, IN STRU_VECTOR_INFO &src)
{
    int i;
    //触发器
    for(i=0;i<src.vec_trigger_name.size();i++)
    {
        dest.vec_trigger_name.push_back(src.vec_trigger_name[i]);
    }
    //警告触发器
    for(i=0;i<src.vec_trigger_name_warn.size();i++)
    {
        dest.vec_trigger_name_warn.push_back(src.vec_trigger_name_warn[i]);
    }
    //函数
    for(i=0;i<src.vec_func_name.size();i++)
    {
        dest.vec_func_name.push_back(src.vec_func_name[i]);
    }
    //警告函数
    for(i=0;i<src.vec_func_name_warn.size();i++)
    {
        dest.vec_func_name_warn.push_back(src.vec_func_name_warn[i]);
    }
    //存储过程
    for(i=0;i<src.vec_proc_name.size();i++)
    {
        dest.vec_proc_name.push_back(src.vec_proc_name[i]);
    }
    //警告存储过程
    for(i=0;i<src.vec_proc_name_warn.size();i++)
    {
        dest.vec_proc_name_warn.push_back(src.vec_proc_name_warn[i]);
    }

    
    return CHECK_SUCCEED;
}

//关系库资源释放
int Check_FreeDb(IN ColAttr *attr,IN int attr_num, IN char *data_buf)
{
    g_CDci.FreeReadData(attr, attr_num,data_buf);
    //ret=g_CDci.FreeColAttrData(attr, attr_num);
    return CHECK_SUCCEED;
}
//从关系库获取名字
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
        printf("GetNameFromDb 调用ReadData函数获取关系库ems模式下的触发器名称时出错,错误码为:%d  错误信息为:%s 出错文件:%s 出错行:%d\n",\
                    error.error_no,error.error_info,error.file_name,error.file_line);
        return CHECK_FAIL;
	}
    if(rec_num<=0)
    {
        printf("在关系库ems模式下不存在想要获取的内容\n");
        ret=Check_FreeDb(attr, attr_num, data_buf);
        if(ret!=CHECK_SUCCEED)
        {
            printf("释放关系库接口申请的属性描述空间和数据空间失败\n");
        }
        return CHECK_NOEXIST;
    }

    int len=0;
    int offset=0;
    offset=0;
    len=attr[0].data_size;
    if(len>=NAME_LEN)
    {
        printf("关系库中想要获取的名称列长度大于%d\n",NAME_LEN);
        ret=Check_FreeDb(attr, attr_num, data_buf);
        if(ret!=CHECK_SUCCEED)
        {
            printf("释放关系库接口申请的属性描述空间和数据空间失败\n");
        }
        return CHECK_FAIL;
    }
    for(i=0;i<rec_num;i++)
    {
        memset(tmp_name_char,0,NAME_LEN);
        strncpy(tmp_name_char,(data_buf+offset),len);
        Check_Toupper(tmp_name_char);//小写转换成大写
        tmp_name_str=tmp_name_char;
        vec_name.push_back(tmp_name_str);
        offset+=ONE_ATTR_OFFSET(len);//跳到下一条数据的起始地址即下一条数据的偏移
    }
    
    //释放调用函数产生的空间

    ret=Check_FreeDb(attr, attr_num, data_buf);
    if(ret!=CHECK_SUCCEED)
    {
        printf("释放关系库接口申请的属性描述空间和数据空间失败\n");
    }
    return CHECK_SUCCEED;
    
}
//从关系库获取关系库中触发器、函数、存储过程名字
int GetAllTriggerNameFromDb(STRU_VECTOR_INFO &stru_vec_name_db)
{
    int ret;
    int tri_noexist_flag=0;
    
    char sql[1024];
    //获取ems模式下所有表的触发器名
    sprintf(sql,"select name from ems.sysdba.systriggers where tableid in \
(select id from ems.sysdba.systables where schid in (select schid from ems.sysdba.sysschemas where name = 'EMS' or name = 'ems'))");    
    ret=GetNameFromDb(sql,stru_vec_name_db.vec_trigger_name);
    if(ret!=CHECK_SUCCEED)
    {
        if(ret==CHECK_NOEXIST)
        {
            printf("在关系库ems模式下不存在触发器\n");
            tri_noexist_flag=1;
        }
        else
        {
            printf("获取关系库ems模式下的触发器名称时失败\n");
            return CHECK_FAIL;
        }
        
    }

       //获取EMS模式下的所有函数
        sprintf(sql,"select name from ems.sysdba.sysprocs where schid in \
    (select schid from ems.sysdba.sysschemas where name = 'EMS' or name = 'ems') and  pfflag = 'N'");    
        ret=GetNameFromDb(sql,stru_vec_name_db.vec_func_name);
        if(ret!=CHECK_SUCCEED)
        {
            if(ret==CHECK_NOEXIST)
            {
                printf("在关系库ems模式下不存在函数\n");
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
                printf("获取关系库ems模式下的触发器名称时失败\n");
                return CHECK_FAIL;
            }
            
        }
        
    
         //获取EMS模式下的所有存储过程
        sprintf(sql,"select name from ems.sysdba.sysprocs where schid in \
    (select schid from ems.sysdba.sysschemas where name = 'EMS' or name = 'ems') and  pfflag = 'Y'");    
        ret=GetNameFromDb(sql,stru_vec_name_db.vec_proc_name);
        if(ret!=CHECK_SUCCEED)
        {
            if(ret==CHECK_NOEXIST)
            {
                printf("在关系库ems模式下不存在存储过程\n");
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
                printf("获取关系库ems模式下的触发器名称时失败\n");
                return CHECK_FAIL;
            }
            
        }
        

    return CHECK_SUCCEED;
}
//从当前系统关系库中获取指定的触发器或函数或存储过程等代码
//flag 为1、4表示为触发器  2、5  表示函数  3、6 表示存储过程
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
    
    
    //将名字中的小写改为大写
    strcpy(tmp_name_char,tmp_name_str.c_str());
    Check_Toupper(tmp_name_char);
    tmp_name_str=tmp_name_char;

    
    char sql[1024];
    if(flag==FLAG_TRI_ERR || flag == FLAG_TRI_WARN)//获取触发器sql
    {
        sprintf(sql,"SELECT TXT FROM EMS.SYSDBA.SYSTRIGGERS WHERE NAME='%s' AND tableid in \
(select id from ems.sysdba.systables where schid in (select schid from ems.sysdba.sysschemas where name = 'EMS' or name = 'ems'))",tmp_name_str.c_str());    
    }
    else//获取存储过程和函数sql
    {        
        sprintf(sql,"SELECT TXT FROM EMS.SYSDBA.SYSPROCS WHERE NAME='%s' AND schid in \
(select schid from ems.sysdba.sysschemas where name = 'EMS' or name = 'ems')",tmp_name_str.c_str());    
    }
	if(!g_CDci.ReadDataEx(sql, -1, &rec_num, &attr_num, &attr, &data_buf, &data_buf_size, &error))
	{
        sprintf(err_char,"%s:%s  调用ReadDataEx函数获取关系库中的%s代码逻辑时出错,错误码为:%d  错误信息为:%s 出错文件:%s 出错行:%d\n",\
                    flagdes[flag],tmp_name_str.c_str(),tmp_name_str.c_str(),error.error_no,error.error_info,error.file_name,error.file_line);
    //    printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
        return CHECK_FAIL;
	}
    if(rec_num<=0)
    {
        sprintf(err_char,"%s:%s  在关系库中不存在\n",flagdes[flag],tmp_name_str.c_str());
      //  printf("%s",err_char);
        g_err_str = g_err_str+err_char;
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
        ret=Check_FreeDb(attr, attr_num, data_buf);
        if(ret!=CHECK_SUCCEED)
        {
            printf("释放关系库接口申请的属性描述空间和数据空间失败\n");
        }
        return CHECK_NOEXIST;
    }
    if(rec_num>1)
    {
        sprintf(err_char,"%s:%s  在关系库中存在的数量超过1,请手动检测EMS.SYSDBA.SYSTRIGGERS/SYSPROCS表下的记录数据\n",flagdes[flag],tmp_name_str.c_str());
    //    printf("%s",err_char);
        g_err_str = g_err_str+err_char;
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
        
        ret=Check_FreeDb(attr, attr_num, data_buf);
        if(ret!=CHECK_SUCCEED)
        {
            printf("释放关系库接口申请的属性描述空间和数据空间失败\n");
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
    if(attr[0].data_type==DCI_CLOB)//#define  DCI_CLOB  112  大文本类型
    {
        offset=ONE_TXT_OFFSET(attr[0].data_size);//大文本偏移计算与普通属性不同
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
        sprintf(err_char,"%s:%s  GetTriggerTextFromDb  申请空间失败,请检测系统磁盘是否已满!\n",flagdes[flag],tmp_name_str.c_str());
        printf("%s",err_char);
        g_err_str = g_err_str+err_char;
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
        
        ret=Check_FreeDb(attr, attr_num, data_buf);
        if(ret!=CHECK_SUCCEED)
        {
            printf("释放关系库接口申请的属性描述空间和数据空间失败\n");
        }
        return CHECK_FAIL;
    }
  //  printf("%d#############%s\n",len,data_buf+offset);
    memset(tmp_buf,0,((len+1)*sizeof(char)));
    strncpy(tmp_buf,data_buf+offset,len);
   // printf("***************%s\n",tmp_buf);

    db_tri_string=tmp_buf;

    //比对属性名  然后再拷贝
  /*  printf("data_buf_size=%d\n",data_buf_size);
    printf("data_buf name=%s\n",data_buf);
    printf("data_buf txt=%s\n",(data_buf+ONE_TXT_OFFSET(attr[0].data_size)));
*/
    
    //释放调用函数产生的空间

    if(tmp_buf!=NULL)
    {
        free(tmp_buf);
        tmp_buf=NULL;
    }
    ret=Check_FreeDb(attr, attr_num, data_buf);
    if(ret!=CHECK_SUCCEED)
    {
        printf("释放关系库接口申请的属性描述空间和数据空间失败\n");
    }
    return CHECK_SUCCEED;
}


//从当前目录下获取指定的触发器或存储过程或函数等代码(标准逻辑代码)
//flag 为1、4表示为触发器  2、5  表示函数  3、6 表示存储过程
int GetTriggerTextFromFile(IN int flag,IN string &name, OUT string &file_tri_string)
{
    char tmp_name_char[NAME_LEN]="";
    string tmp_name_str=name;
    char err_char[1024];
    
    
    //将名字中的小写改为大写
    strcpy(tmp_name_char,name.c_str());
    Check_Toupper(tmp_name_char);
    tmp_name_str=tmp_name_char;

    //组建标准文件路径
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
        sprintf(err_char,"%s:%s  GetTriggerTextFromFile  组建标准触发器文件路径时,FLAG标识(%d)不识别!!!\n",\
                    flagdes[flag],tmp_name_str.c_str(),flag);
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
        return CHECK_FAIL;
    }

    
    struct stat stat_buf;
    if(stat(file_path.c_str(), &stat_buf) < 0)
    {
        sprintf(err_char,"%s:%s  GetTriggerTextFromFile  获取文件状态(stat)失败,请检测文件%s是否存在!!!\n",\
                flagdes[flag],tmp_name_str.c_str(),file_path.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
        return CHECK_FAIL;
    }
    int file_len=stat_buf.st_size;
    if (file_len <= 0)
    {
        sprintf(err_char,"%s:%s  GetTriggerTextFromFile  文件长度为0(ZERO) ,请检测文件%s是否包含内容!\n",\
                flagdes[flag],tmp_name_str.c_str(),file_path.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
        return CHECK_FAIL;
    }
    //printf("*************file_len=%d\n",file_len);
    char *buff=NULL;
    buff=(char *)malloc((file_len+1)*sizeof(char));
    if(buff==NULL)
    {
        sprintf(err_char,"%s:%s  GetTriggerTextFromFile  申请空间失败,请检测系统磁盘是否已满!\n",\
                flagdes[flag],tmp_name_str.c_str());
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
        return CHECK_FAIL;
    }

    memset(buff, 0, file_len);

    FILE *fp=NULL;
    fp = fopen(file_path.c_str(), "r");
    if(fp==NULL)
    {
        
        sprintf(err_char,"%s:%s  GetTriggerTextFromFile  打开文件%s失败,错误号:%d  错误信息:%s\n",\
                flagdes[flag],tmp_name_str.c_str(),file_path.c_str(),errno,strerror(errno));
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
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
        sprintf(err_char,"%s:%s  GetTriggerTextFromFile  fread 读取文件内容失败,read_count=%d  file_len=%d\n",\
                flagdes[flag],tmp_name_str.c_str(),read_count,file_len);
        printf("%s",err_char);
        g_err_str=g_err_str+err_char;
        
        StorageStructMember(g_stru_vec_err_name,name,flag);//存储出错的名
        if(buff!=NULL)
        {
            free(buff);
            buff=NULL;
        }
        fclose(fp);
        fp=NULL;
        return CHECK_FAIL;
    }
    //关闭文件
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

//去掉最后一个分号   原因触发器等代码最后加不加;号都可以编译成功
//为了避免最后一个;号不匹配的问题则统一去掉
int RmLastSemicolon(string &dest_str)
{
    int ch_value_size = dest_str.size();
    int pos= dest_str.find_last_of(';');
    if((pos != dest_str.npos) && (ch_value_size== (pos+1)))//最后一个字符为空
    {
        dest_str.erase(pos,1);
    }
    return CHECK_SUCCEED;
}
//删除string中的指定字符
int RmChar(string &dest_str, char del_c)
{
    int startpos =0;

    //找到要删除字符的位置
    startpos = dest_str.find(del_c,startpos);
    while(startpos != dest_str.npos)
    {
        //删除要删除的字符串
        dest_str.erase(startpos,1);
        //下次循环
        startpos = dest_str.find(del_c,startpos);
    }   
    return CHECK_SUCCEED;
}
//去掉获得的代码中的空格、回车、TAB等等  ' ' '\t'   '\n'   '\r'
int RmSpaceAndTab(string &tri_string)
{
    //去除空格
    RmChar(tri_string,' ');
    
    //去除Tab
    RmChar(tri_string,'\t');

    //去除c回车
    RmChar(tri_string,'\n');

    //去除兼容去除windows下回车\r\n   遗留下的\r
    RmChar(tri_string,'\r');

    return CHECK_SUCCEED;
}

//去掉 注释行
int RmComment(string &tri_string)
{
    int startpos=0;
    int endpos=0;
    //去掉 --注释行
    startpos = tri_string.find("--",startpos);
    while(startpos!=tri_string.npos)
    {
        //找到--行的结束
        endpos = tri_string.find('\n',startpos);
        
        if(endpos ==tri_string.npos)
        //处理最后最后一行为注释行,匹配不到'\n'的情况
        {
            //清除的长度为整个字符串长度,保证将最后一行注释删除掉
            tri_string.erase(startpos,tri_string.size());
        }
        else
        //处理正常情况下的注释行
        {
            //清除--注释行
            tri_string.erase(startpos,(endpos-startpos+1));
        }
        //找到下一个注释行的开始
        startpos = tri_string.find("--",startpos);
    }
    
    //去掉/* */  注释行
    startpos=0;
    endpos=0;
    startpos = tri_string.find("/*",startpos);
    while(startpos!=tri_string.npos)
    {
        //找到--行的结束
        endpos = tri_string.find("*/",startpos);
        //去掉/* */注释行
        tri_string.erase(startpos,(endpos-startpos+1));
        //找到下一个注释行的开始
        startpos = tri_string.find("/*",startpos);
    }
    
    return CHECK_SUCCEED;
}
//读取配置文件check_trigger.conf指定内容
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
        printf("读取配置文件%s中的%s时,初始化读取工具失败!!!\n",CHECK_TRI_CONF,section_name.c_str());
        CParaManage::RemoveObject(pm);
        pm = NULL;
        return CHECK_FAIL;
    }
    if(pm->GetKeyNum((char *)section_name.c_str(), num) < 0)
    {
        printf("获取配置文件%s中%s的数量失败!!!\n",CHECK_TRI_CONF,section_name.c_str());
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
                printf("获取第%d个%s名字失败!!!\n",(i+1),section_name.c_str());
                CParaManage::RemoveObject(pm);
                pm = NULL;
                return CHECK_FAIL;
            }
            Check_Toupper(ch_value);//将小写转换成大写
            tmp_str=ch_value;
            vec_name.push_back(tmp_str);
        }
    }
    else
    {
        printf("配置文件%s中的%s数量为0!!!\n",CHECK_TRI_CONF,section_name.c_str());
        CParaManage::RemoveObject(pm);
        pm = NULL;
        return CHECK_SUCCEED;
    }
    
    CParaManage::RemoveObject(pm);
    pm = NULL;
    return CHECK_SUCCEED;
}

//读取配置文件check_trigger.conf所有内容
int GetAllConfigInfo(OUT STRU_VECTOR_INFO &stru_vec_name_conf)
{
    int ret;
     //获取配置文件中的标准触发器名
    string prefix_rep_name = "name_";
    string section_name;
    //获取配置文件中配置的触发器 函数  存储过程
    //触发器
    section_name ="trigger";
    ret=GetConfigInfo(section_name,prefix_rep_name,stru_vec_name_conf.vec_trigger_name);
    if(ret!=CHECK_SUCCEED)
    {
        printf("GetAllConfigInfo 从配置文件中获取触发器信息失败!!!\n");
        return CHECK_FAIL;
    }
    //函数
    section_name="function";
    ret=GetConfigInfo(section_name,prefix_rep_name,stru_vec_name_conf.vec_func_name);
    if(ret!=CHECK_SUCCEED)
    {
        printf("GetAllConfigInfo 从配置文件中获取函数信息失败!!!\n");
        return CHECK_FAIL;
    }
    //存储过程
    section_name="procedure";
    ret=GetConfigInfo(section_name,prefix_rep_name,stru_vec_name_conf.vec_proc_name);
    if(ret!=CHECK_SUCCEED)
    {
        printf("GetAllConfigInfo 从配置文件中获取存储过程信息失败!!!\n");
        return CHECK_FAIL;
    }
    //警告触发器
    section_name="trigger_warn";
    ret=GetConfigInfo(section_name,prefix_rep_name,stru_vec_name_conf.vec_trigger_name_warn);
    if(ret!=CHECK_SUCCEED)
    {
        printf("GetAllConfigInfo 从配置文件中获取警告触发器信息失败!!!\n");
        return CHECK_FAIL;
    }
    //警告函数
    section_name="function_warn";
    ret=GetConfigInfo(section_name,prefix_rep_name,stru_vec_name_conf.vec_func_name_warn);
    if(ret!=CHECK_SUCCEED)
    {
        printf("GetAllConfigInfo 从配置文件中获取警告函数信息失败!!!\n");
        return CHECK_FAIL;
    }
    //警告存储过程
    section_name="procedure_warn";
    ret=GetConfigInfo(section_name,prefix_rep_name,stru_vec_name_conf.vec_proc_name_warn);
    if(ret!=CHECK_SUCCEED)
    {
        printf("GetAllConfigInfo 从配置文件中获取警告存储过程信息失败!!!\n");
        return CHECK_FAIL;
    }

    return CHECK_SUCCEED;
}

//按标识flag存储名称到目的结构体中
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
        //不应该走到此处,上层函数中已经保证flag为识别范围内的
        printf("存储出错或不相等触发器、函数、存储过程名时,flag标识位(%d)不识别!!!\n",flag);
        return CHECK_FAIL;
    }
    return CHECK_SUCCEED;
}

//获取成员数量
int CountStruMemberNum(STRU_VECTOR_INFO &stru_vec_name)
{
    int sum=0;
    
    sum=stru_vec_name.vec_trigger_name.size()+stru_vec_name.vec_func_name.size()+stru_vec_name.vec_proc_name.size()\
                +stru_vec_name.vec_trigger_name_warn.size()+stru_vec_name.vec_func_name_warn.size()+stru_vec_name.vec_proc_name_warn.size();
    
    return sum;
}


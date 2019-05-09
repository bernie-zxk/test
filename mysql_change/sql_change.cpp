#include "sql_change.h"

CSqlTool::CSqlTool()
{

}
CSqlTool::~CSqlTool()
{

}
#if 0
void CSqlTool::PrintExportResult(const char * sql_str, const TSqlAnlsResult & sql_anls_result)
{
    int         i;

    cout << " Now Print the result of sql analyzer " << endl;
    cout << "--------------------------------------" << endl;

    cout << " 1. Original SQL is :  " << sql_str << endl;

    cout << " 2. sql type is    : " << sql_anls_result.sql_type << endl;
    cout << " 3. table name is  : " << sql_anls_result.table_name << endl;

    cout << " 4. column num is  : " << (short)sql_anls_result.column_name_vec.size() << endl;
    for (i = 0; i < sql_anls_result.column_name_vec.size(); i++)
    {
        cout << "    4." << i << "  name is   : " << sql_anls_result.column_name_vec[i] << endl;
    }

    cout << " 5. value num is   : " << (short)sql_anls_result.column_value_vec.size() << endl;
    for (i = 0; i < sql_anls_result.column_value_vec.size(); i++)
    {
        cout << "    5." << i << "  value is : ";
        if (sql_anls_result.column_value_vec[i].is_null == TRUE)
        {
            cout << "NULL";
        }
        else
        {
            switch (sql_anls_result.column_value_vec[i].const_type)
            {
                case    INTPRE_DATATYPE_INT :
                    cout << sql_anls_result.column_value_vec[i].const_value.int_value << "|INT";
                    break;

                case    INTPRE_DATATYPE_DATETIME :
                    cout << sql_anls_result.column_value_vec[i].const_value.int_value << "|DTM";
                    break;

                case    INTPRE_DATATYPE_REAL :
                    cout << sql_anls_result.column_value_vec[i].const_value.real_value << "|REL";
                    break;

                case    INTPRE_DATATYPE_STRING :
                    cout << sql_anls_result.column_value_vec[i].const_value.string_value << "|STR";
                    break;

                case    INTPRE_DATATYPE_BINARY :
                    cout << sql_anls_result.column_value_vec[i].const_value.binary_value << "|BIN";
                    break;

                case    INTPRE_DATATYPE_KEYID :
                    cout << sql_anls_result.column_value_vec[i].const_value.key_id_value <<
                         "|" << sql_anls_result.column_value_vec[i].const_value.key_id_value;
                    break;

                default :
                    cout << "UNKOWN DATATYPE " << sql_anls_result.column_value_vec[i].const_type;
                    break;

            }
        }
        cout << endl;

    }

    cout << " 6. orderby num is : " << (short)sql_anls_result.order_name_vec.size() << endl;
    for (i = 0; i < sql_anls_result.order_name_vec.size(); i++)
    {
        cout << "    6." << i << "  name is  : " << sql_anls_result.order_name_vec[i] << endl;
    }

    cout << "----------------- WhereAntiPorland -------------------" << endl;
    for (i = 0; i < sql_anls_result.where_code_vec.size(); i++)
    {
        cout << i << " : ";

        if (sql_anls_result.where_code_vec[i].code_type == CODE_TYPE_IDENTIFIER)
        {
            cout << sql_anls_result.where_code_vec[i].code_info.iden_name;
        }
        else if (sql_anls_result.where_code_vec[i].code_type == CODE_TYPE_CONSTANT)
        {
            if (sql_anls_result.where_code_vec[i].code_info.const_info.is_null == TRUE)
            {
                cout << "NULL";
            }
            else
            {
                switch (sql_anls_result.where_code_vec[i].code_info.const_info.const_type)
                {
                    case    INTPRE_DATATYPE_INT :
                        cout << sql_anls_result.where_code_vec[i].code_info.const_info.const_value.int_value << "|INT";
                        break;

                    case    INTPRE_DATATYPE_DATETIME :
                        cout << sql_anls_result.where_code_vec[i].code_info.const_info.const_value.int_value << "|DTM";
                        break;

                    case    INTPRE_DATATYPE_REAL :
                        cout << sql_anls_result.where_code_vec[i].code_info.const_info.const_value.real_value << "|REL";
                        break;

                    case    INTPRE_DATATYPE_STRING :
                        cout << sql_anls_result.where_code_vec[i].code_info.const_info.const_value.string_value << "|STR";
                        break;

                    case    INTPRE_DATATYPE_BINARY :
                        cout << sql_anls_result.where_code_vec[i].code_info.const_info.const_value.binary_value << "|BIN";
                        break;


                    default :
                        cout << "UNKOWN DATATYPE " << sql_anls_result.where_code_vec[i].code_info.const_info.const_type;
                        break;

                }
            }
        }
        else if (sql_anls_result.where_code_vec[i].code_type == CODE_TYPE_OPERATOR)
        {
            switch (sql_anls_result.where_code_vec[i].code_info.operator_code)
            {
                case    OPERATOR_CODE_ADD :
                    cout << "ADD";
                    break;

                case    OPERATOR_CODE_SUB :
                    cout << "SUB";
                    break;

                case    OPERATOR_CODE_MUL :
                    cout << "MUL";
                    break;

                case    OPERATOR_CODE_DIV :
                    cout << "DIV";
                    break;

                case    OPERATOR_CODE_IDV :
                    cout << "IDV";
                    break;

                case    OPERATOR_CODE_MOD :
                    cout << "MOD";
                    break;

                case    OPERATOR_CODE_ANB :
                    cout << "ANB";
                    break;

                case    OPERATOR_CODE_ORB :
                    cout << "ORB";
                    break;

                case    OPERATOR_CODE_XOR :
                    cout << "XOR";
                    break;

                case    OPERATOR_CODE_EQU :
                    cout << "EQU";
                    break;

                case    OPERATOR_CODE_NEQ :
                    cout << "NEQ";
                    break;

                case    OPERATOR_CODE_GRT :
                    cout << "GRT";
                    break;

                case    OPERATOR_CODE_GEQ :
                    cout << "GEQ";
                    break;

                case    OPERATOR_CODE_LSS :
                    cout << "LSS";
                    break;

                case    OPERATOR_CODE_LEQ :
                    cout << "LEQ";
                    break;

                case    OPERATOR_CODE_POS :
                    cout << "POS";
                    break;

                case    OPERATOR_CODE_NEG :
                    cout << "NEG";
                    break;

                case    OPERATOR_CODE_AND :
                    cout << "AND";
                    break;

                case    OPERATOR_CODE_ORR :
                    cout << "ORR";
                    break;

                case    OPERATOR_CODE_NOT :
                    cout << "NOT";
                    break;

                case    OPERATOR_CODE_LKE :
                    cout << "LKE";
                    break;

                default :
                    cout << " UNKNOWN OPERATOR " << sql_anls_result.where_code_vec[i].code_info.operator_code;
                    break;

            }
        }
        cout << endl;

    }

    cout << "--------------------- This is the END ------------------------" << endl;
}
void CSqlTool::printfResult(const TSqlAnlsResult & sql_anls_result)
{
    int field_num ;
    short data_type;
    string column_name_string;
    string field_value;
    field_num = sql_anls_result.column_name_vec.size();
    cout  << " begin--------------------------------------" << endl;
    for(int i=0; i<field_num; i++)
    {
        data_type = sql_anls_result.column_value_vec[i].const_type;
        field_value.clear();
        field_value = GetValueByConstInfo(data_type,sql_anls_result.column_value_vec[i]);
        cout << "域名: " << sql_anls_result.column_name_vec[i];
        cout << "域值: "  << field_value << endl;
    }


    cout  << " --------------------------------------endl" << endl;

}

#endif
int CSqlTool::SqlProcess(const StrSeq& sql_request)
{

    int                         i,sql_num;
    string                      sql_str;
    vector<string>              sql_vec;
    map<long,long>              id_relation;
    AnlsResult                  allResult;
    vector<TRequestInfo>   tableRequestVec;
    vector<TTableColumnInfo>    anlsColInfoVec;
    TSqlAnlsResult  sqlAnlsResult;

    cout << "SqlProcess has been called! " << endl;
    cout << endl;

    // 接收参数

    sql_num = sql_request.size();
    if (sql_num <= 0)
    {
        cout << "sql_num = 0" << endl;
        return  FALSE;
    }

    for (i = 0; i < sql_num; i++)
    {
        sql_vec.push_back(sql_request[i].c_str());
    }

    for (i = 0; i < sql_num; i++)
    {
        cout << sql_vec[i] << endl;
    }

    // 通过预分析得到每个表的域信息结构
    if (GetTableColInfo(sql_vec,tableRequestVec,anlsColInfoVec) == FALSE)
    {
        cout << " GetTableColInfo 出错！" << endl;
        return  FALSE;
    }
    //sql分析结果整理
    if (AllSqlAnalys(sql_vec,tableRequestVec,anlsColInfoVec,allResult) == FALSE)
    {
        cout << " AllSqlAnalys 出错！" << endl;
        return  FALSE;
    }
	//id替换生成id对照
    if(AllIdChange(allResult.Insert) == FALSE)
    {
        cout << " AllIdChange 出错！" << endl;
        return  FALSE;

    }
	//量测SQL处理
    if(AllMeasChange(allResult) == FALSE)
    {
        cout << " AllMeasChange 出错！" << endl;
        return  FALSE;

    }
    cout <<endl;
    cout << "******************************************************************************"<<endl;
    for(i=0; i<allResult.Insert.size(); i++)
    {
        for(int j=0; j<allResult.Insert[i].VOnesqlResult.size(); j++)
        {
            cout << "Insertsql: " <<  allResult.Insert[i].VOnesqlResult[j].sql << endl;
        }

    }

    for(i=0; i<allResult.Update.size(); i++)
    {
        for(int j=0; j<allResult.Update[i].VOnesqlResult.size(); j++)
        {
            cout << "Updatesql: " <<  allResult.Update[i].VOnesqlResult[j].sql << endl;
        }

    }
    cout << "******************************************************************************"<<endl;


    return  TRUE;
}



bool CSqlTool::GetTableColInfo(vector<string>& sql_vec,
                               vector<TRequestInfo>& tableRequestVec,
                               vector<TTableColumnInfo>& anlsColInfoVec)
{
    int                     i, j;
    int                     sql_type;
    TPreAnalysInfo          pre_anlys_info;
    char                    err_msg[MAX_ERRMSG_LEN];
    TRequestInfo            table_request_info;
    TTableColumnInfo        table_column_info;


    for (i = 0; i < sql_vec.size(); i++)
    {
        table_request_info.err_flag       = false;
        table_request_info.sql_type       = -1;
        table_request_info.err_msg        = "";
        table_request_info.col_info_index = -1;
//cout <<"*****************************" << endl;
        if(PreAnalysProcess(sql_vec[i],pre_anlys_info) == FALSE)
        {
            table_request_info.err_flag = true;
            table_request_info.err_msg  = pre_anlys_info.err_msg;
            table_request_info.sql_type = sql_type;
            return FALSE;
        }
        // SELECT类型不考虑
        else if (pre_anlys_info.sql_type == SQL_TYPE_SELECT)
        {
            table_request_info.err_flag = true;
            table_request_info.err_msg  = " 语法分析预分析过程错误 ：本服务不适用于SELECT子句 ";
            table_request_info.sql_type = sql_type;
            return FALSE;
        }
        else if (pre_anlys_info.sql_type == SQL_TYPE_DELETE)
        {
            table_request_info.sql_type = pre_anlys_info.sql_type;
        }
        // 对于其他情况，生成每个表的域信息
        else
        {
            table_request_info.sql_type = pre_anlys_info.sql_type;
            for (j = 0; j < anlsColInfoVec.size(); j++)
            {
                if (strcmp(pre_anlys_info.table_name, (char *)anlsColInfoVec[j].table_name.c_str()) == 0)
                {
                    table_request_info.col_info_index = j;
                    break;
                }
            }
            if (table_request_info.col_info_index == -1)    // 未在列的定义中找到
            {
                // 根据表名检索该表的表号以及域的定义信息
                if (GetTableColInfoByTableName(pre_anlys_info.table_name, table_column_info) == FALSE)
                {
                    table_request_info.err_flag = true;
                    table_request_info.err_msg  = " 检索该表的表和域信息时出错 ";
                }
                else
                {
                    anlsColInfoVec.push_back(table_column_info);
                    table_request_info.col_info_index = anlsColInfoVec.size() - 1;
                }
            }
        }

        tableRequestVec.push_back(table_request_info);

    }

    return  true;
}

bool CSqlTool::PreAnalysProcess(string& sql_str,TPreAnalysInfo& preAnalysInfo)
{

    char                    err_msg[MAX_ERRMSG_LEN];

    CSqlSyntaxAnalyser      sqlSynAnls;

    if (sqlSynAnls.Initialize((char *)sql_str.c_str()) == FALSE)
    {
        preAnalysInfo.err_msg= " 语法分析初始化错误 ";

        cout << preAnalysInfo.err_msg << endl;
        return FALSE;
    }
    // 预分析SQL类型和表名
    if (sqlSynAnls.SqlPreAnalysProcess(preAnalysInfo.table_name, preAnalysInfo.sql_type) == FALSE)
    {
        sqlSynAnls.GetError(err_msg);
        preAnalysInfo.err_msg= "SqlPreAnalysProcess 出错";
        preAnalysInfo.err_msg+= err_msg;

        cout << preAnalysInfo.err_msg << endl;
        return FALSE;
    }

    return TRUE;
}

#if 0
bool CSqlTool::GetTableColInfoByTableName(const char * table_name, TTableColumnInfo& table_column_info)
{
    int             i,rec_num;
    int             ret_value;
    char            tmp_str[50];
    string          sql_string;
    TAnalysColumnInfo       anls_col_info;

    struct  TTableNo
    {
        int     table_no;
    }       table_no_stru;

    ColInfo_Stru attr_info;
    attr_info.data_size = sizeof(int);
    attr_info.data_type = C_DATATYPE_INT;


    TColumnInfo *column_info_ptr;
    ColInfo_Stru col_info[6];



    // 根据表名得到相应的表号
    sql_string = "select table_id from sys_table_info where table_name_eng = '";
    sql_string += table_name;
    sql_string += "'";

    //执行sql语句  获取内容
    if(g_db_dci.ReadData(sql_string.c_str() < 0))
    {
        string err = g_db_dci.ErrMsg();
        printf(" GetTableColInfoByTableName 检索出错, 错误信息为:%s \n",err);
        g_db_dci.FreeReadData();
        return FALSE;
    }
    rec_num = g_db_dci.RowNum();

    if (rec_num != 1)
    {
        cout << " 该表不存在或表号不唯一 " << endl;
        g_db_dci.FreeReadData();

        return  FALSE;
    }

    //对齐数据
    ret_value = g_db_dci.GetAlignResult(&table_no_stru,&attr_info,sizeof(TTableNo));
    if (ret_value < 0)
    {
        cout << " GetTableInfo 内存分配出错 -- 对齐 " << endl;
        g_db_dci.FreeReadData();

        return  FALSE;
    }

    g_db_dci.FreeReadData();



    table_column_info.table_no   = table_no_stru.table_no;
    table_column_info.table_name = table_name;
    table_column_info.all_col_vec.clear();
    table_column_info.anls_col_vec.clear();

    // 根据表号得到所有域的信息
    sql_string = "select column_id, column_name_eng, column_name_chn,data_type,data_length,is_key from sys_column_info where table_id = ";
    sprintf(tmp_str, "%d", table_no_stru.table_no);
    sql_string += tmp_str;
    sql_string += " order by column_id";


    col_info[0].data_type = C_DATATYPE_SHORT;
    col_info[0].data_size = sizeof(short);
    col_info[1].data_type = C_DATATYPE_UCHAR;
    col_info[1].data_size = 32;
    col_info[2].data_type = C_DATATYPE_UCHAR;
    col_info[2].data_size = 64;
    col_info[3].data_type = C_DATATYPE_UCHAR;
    col_info[3].data_size = 1;
    col_info[4].data_type = C_DATATYPE_SHORT;
    col_info[4].data_size = sizeof(short);
    col_info[5].data_type = C_DATATYPE_UCHAR;
    col_info[5].data_size = 1;

    //执行sql语句  获取内容
    if(g_db_dci.ReadData(sql_string.c_str() < 0)
{
    string err = g_db_dci.ErrMsg();
        printf(" GetTableColInfoByTableName1 检索出错, 错误信息为:%s \n",err);
        g_db_dci.FreeReadData();
        return FALSE;
    }
    rec_num = g_db_dci.RowNum();

              if (rec_num <= 0)
{
    cout << " GetTableColInfoByTableName 该表没有一个域  " << endl;
    g_db_dci.FreeReadData();

        return  FALSE;
    }

    column_info_ptr = new TColumnInfo[rec_num];
    ret_value = g_db_dci.GetAlignResult(column_info_ptr,col_info,sizeof(TColumnInfo));
    if (ret_value < 0)
{
    cout << " GetTableInfo 内存分配出错 -- 对齐 " << endl;
    delete [] column_info_ptr;
        g_db_dci.FreeReadData();

        return  FALSE;
    }


    for (i = 0; i < rec_num; i++)
{
    table_column_info.all_col_vec.push_back(column_info_ptr[i]);
    }

    delete [] column_info_ptr;
    g_db_dci.FreeReadData();



#if 1 //新系统显示类型与程序处理类型一致不需要转换   注释掉  shiyong20171121    
    // 将统一数据类型转换为C++数据类型和相应的长度
    for (i = 0; i < rec_num; i++)
{
    switch (table_column_info.all_col_vec[i].column_type)
        {
            case    UNI_DATATYPE_CHAR :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_STRING;
                table_column_info.all_col_vec[i].column_length += 1;
                break;

            case    UNI_DATATYPE_STRING :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_STRING;
                break;

            case    UNI_DATATYPE_UCHAR :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_UCHAR;
                table_column_info.all_col_vec[i].column_length = sizeof(uchar);
                break;

            case    UNI_DATATYPE_SHORT :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_SHORT;
                table_column_info.all_col_vec[i].column_length = sizeof(short);
                break;

            case    UNI_DATATYPE_INT :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_INT;
                table_column_info.all_col_vec[i].column_length = sizeof(int);
                break;

            case    UNI_DATATYPE_FLOAT :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_FLOAT;
                table_column_info.all_col_vec[i].column_length = sizeof(float);
                break;

            case    UNI_DATATYPE_DOUBLE :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_DOUBLE;
                table_column_info.all_col_vec[i].column_length = sizeof(double);
                break;

            case    UNI_DATATYPE_DATETIME :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_DATETIME;
                table_column_info.all_col_vec[i].column_length = sizeof(long);
                break;

            case    UNI_DATATYPE_KEYID :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_KEYID;
                table_column_info.all_col_vec[i].column_length = sizeof(TKeyID);
                break;

            case    UNI_DATATYPE_APPKEYID :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_APPKEYID;
                table_column_info.all_col_vec[i].column_length = sizeof(TAppKeyID);
                break;

            case    UNI_DATATYPE_APPID :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_APPID;
                table_column_info.all_col_vec[i].column_length = sizeof(TAppID);
                break;

            case    UNI_DATATYPE_BINARY :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_BINARY;
                break;

            case    UNI_DATATYPE_TEXT :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_TEXT;
                break;

            case    UNI_DATATYPE_IMAGE :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_IMAGE;
                break;

            case    UNI_DATATYPE_LONG :
                table_column_info.all_col_vec[i].column_type   = C_DATATYPE_LONG;
                table_column_info.all_col_vec[i].column_length = sizeof(long);
                break;

            default :
                break;

        }
    }
#endif
    // 填充语法分析所需要的域信息序列
    for (i = 0; i < rec_num; i++)
{
    strcpy(anls_col_info.col_name, table_column_info.all_col_vec[i].column_name);
        anls_col_info.col_type = table_column_info.all_col_vec[i].column_type;

        table_column_info.anls_col_vec.push_back(anls_col_info);
    }

    return  true;
}

string CSqlTool::GetValueByConstInfo(const short data_type, const TConstInfo & const_info)
{
    char            tmp_str[4096];
    string        value_string;

    if (const_info.is_null == TRUE)
    {
//                UDataValue  null_value_union;
//                null_value_union.c_default(0);

//                return  null_value_union;
        //sprintf(tmp_str, "");
        value_string = "";
        return value_string;
    }
    else
    {
        switch (const_info.const_type)
        {
            case    INTPRE_DATATYPE_INT :
                sprintf(tmp_str, "%ld", const_info.const_value.int_value);
                break;

            case    INTPRE_DATATYPE_REAL :
                sprintf(tmp_str, "%f", const_info.const_value.real_value);
                break;

            case    INTPRE_DATATYPE_STRING :
                sprintf(tmp_str, "%s", const_info.const_value.string_value);
                break;

            case    INTPRE_DATATYPE_BINARY :
                sprintf(tmp_str, "%s", const_info.const_value.binary_value);
                break;

            case    INTPRE_DATATYPE_DATETIME :
                sprintf(tmp_str, "%ld", const_info.const_value.int_value);
                break;

            case    INTPRE_DATATYPE_KEYID :
                //sprintf(tmp_str, "%010d%04d", const_info.const_value.key_id_value.record_id,
                //                              const_info.const_value.key_id_value.column_id);
                sprintf(tmp_str, "");
                break;

            case    INTPRE_DATATYPE_APPKEYID :
                sprintf(tmp_str, "%010d%020ld", const_info.const_value.app_key_id_value.app_id,
                        const_info.const_value.app_key_id_value.key_id);
                cout<<"++++ INTPRE_DATATYPE_APPKEYID tmp_str: <"<<tmp_str<<">"<<endl;
                break;

            case    INTPRE_DATATYPE_APPID :
                //sprintf(tmp_str, "%010d%010d", const_info.const_value.app_id_value.app_id,
                //                              const_info.const_value.app_id_value.record_id);
                sprintf(tmp_str, "");
                break;

            case    INTPRE_DATATYPE_TEXT :
                sprintf(tmp_str, "");
                break;

            case    INTPRE_DATATYPE_IMAGE :
                sprintf(tmp_str, "");
                break;

            default :
                sprintf(tmp_str, "");
                break;

        }
    }
    value_string = tmp_str;
    return value_string;
}


#endif


#if 0
bool CSqlTool::AllSqlAnalys(vector<string>& sql_vec,
                            vector<TRequestInfo>& tableRequestVec,
                            vector<TTableColumnInfo>& anlsColInfoVec,AnlsResult& anlsResult)
{
    int i,j,sql_num,col_index;
    bool is_find;
//  int field_num,field_no;
    OneSqlResult   oneSqlResult;
    ClassifyResult tmp_Classify_Result;
    TTableInfo     table_info;
    TSqlAnlsResult  sqlAnlsResult;

    sql_num = sql_vec.size();
    if (sql_num <= 0)
    {
        cout << "sql_num = 0" << endl;
        return  FALSE;
    }

    for (i = 0; i < sql_num; i++)
    {
        if(tableRequestVec[i].sql_type == SQL_TYPE_DELETE)
        {
            anlsResult.Delete_vec.push_back(sql_vec[i]);
            continue;
        }
        oneSqlResult.sql.clear();
        StructVectorClear(oneSqlResult.sqlAnlsResult);

        oneSqlResult.sql = sql_vec[i];
        col_index = tableRequestVec[i].col_info_index;
        if(AnalysProcess(sql_vec[i],anlsColInfoVec[col_index].anls_col_vec,oneSqlResult.sqlAnlsResult) == FALSE)
        {
            cout << "AnalysProcess 出错" << endl;
            return FALSE;
        }
        //获取表信息
        if(GetTableInfo(anlsColInfoVec[col_index].table_no,table_info) == FALSE)
        {
            cout << " 获取表信息失败!" << endl;
            return FALSE;
        }

        switch(tableRequestVec[i].sql_type)
        {
            case SQL_TYPE_INSERT:
            {
                is_find = false;
                for(j=0; j<anlsResult.Insert.size(); j++)
                {
                    if(anlsResult.Insert[j].table_column_info.table_no == anlsColInfoVec[col_index].table_no)
                    {
                        anlsResult.Insert[j].VOnesqlResult.push_back(oneSqlResult);
                        is_find = true;
                        break;
                    }

                }

                if(!is_find)
                {
                    tmp_Classify_Result.table_info = table_info;
                    tmp_Classify_Result.table_column_info = anlsColInfoVec[col_index];
                    tmp_Classify_Result.VOnesqlResult.clear();
                    tmp_Classify_Result.VOnesqlResult.push_back(oneSqlResult);
                    anlsResult.Insert.push_back(tmp_Classify_Result);

                }

                break;

            }
            case SQL_TYPE_UPDATE:
            {
                is_find = false;
                for(j=0; j<anlsResult.Update.size(); j++)
                {
                    if(anlsResult.Update[j].table_column_info.table_no == anlsColInfoVec[col_index].table_no)
                    {
                        anlsResult.Update[j].VOnesqlResult.push_back(oneSqlResult);
                        is_find = true;
                        break;
                    }
                }
				
                if(!is_find)
                {
                    tmp_Classify_Result.table_info = table_info;
                    tmp_Classify_Result.table_column_info = anlsColInfoVec[col_index];
                    tmp_Classify_Result.VOnesqlResult.clear();
                    tmp_Classify_Result.VOnesqlResult.push_back(oneSqlResult);
                    anlsResult.Update.push_back(tmp_Classify_Result);
                }


                break;
            }

            default:
                cout << "未识别sql类型" << endl;
                return FALSE;
                break;

        }


//    cout << "oneSqlResult :sql_vec:" << sql_vec[i]<<endl;
//    cout << endl;

    }

    return TRUE;
}


bool CSqlTool::AnalysProcess(string& sql_str,
                             VECAnalysColumnInfo&  anls_col_vec,
                             TSqlAnlsResult&  sqlAnlsResult)
{
    char                    err_msg[MAX_ERRMSG_LEN];

    CSqlSyntaxAnalyser      sqlSynAnls;

    // 进行语法分析，判断是否有语法错误
    sqlSynAnls.Initialize((char *)sql_str.c_str());
    sqlSynAnls.PutAnalysColumnInfo(true, anls_col_vec, true);
    if (sqlSynAnls.SqlAnalysProcess() == FALSE)
    {
        cout << " Original SQL is   :  " << sql_str << endl;
        cout << " 语法分析出错 " << endl;
        sqlSynAnls.GetError(err_msg);
        cout << " 错误原因  ： " << err_msg << endl;
        return FALSE;
    }
    sqlSynAnls.ExportTable(sqlAnlsResult);
//  PrintExportResult(sql_str.c_str(), sqlAnlsResult);
    return TRUE;
}
//结构体清空
int CSqlTool::StructVectorClear(TSqlAnlsResult& sqlAnlsResult)
{
    //清空各个成员
    sqlAnlsResult.sql_type = -1;
    sqlAnlsResult.table_name.clear();
    sqlAnlsResult.column_name_vec.clear();
    sqlAnlsResult.column_value_vec.clear();
    sqlAnlsResult.order_name_vec.clear();
    sqlAnlsResult.where_code_vec.clear();

    return TRUE;
}




// 查询数据库得到表的全部信息
// 输入参数 ：  table_no        表号
// 输出参数 ：  DB_SERVER_SUCCESS               成功
//              DB_SERVER_ERR_NO_TABLE          表不存在
//              DB_SERVER_ERR_QUERY_FAIL        查询过程出错
//              DB_SERVER_ERR_MEMORY_FAIL       内存分配出错

bool CSqlTool::GetTableInfo(int table_no, TTableInfo & table_all_info)
{
    char            query_string[1000];
    int            ret_value;

    int             rec_num=0;
    struct  TTableInfo * TTableInfo_ptr;
    ColInfo_Stru col_info[4];


    col_info[0].data_type = C_DATATYPE_INT;
    col_info[0].data_size = sizeof(int);
    col_info[1].data_type = C_DATATYPE_UCHAR;
    col_info[1].data_size = 32;
    col_info[2].data_type = C_DATATYPE_SHORT;
    col_info[2].data_size = sizeof(short);
    col_info[3].data_type = C_DATATYPE_INT;
    col_info[3].data_size = sizeof(int);



    sprintf(query_string, "select table_id, table_name_eng,key_generate_type,reserved_4 from sys_table_info where table_id = %d", table_no);

    //执行sql语句  获取内容
    if(g_db_dci.ReadData(query_string.c_str() < 0)
{
    string err = g_db_dci.ErrMsg();
        printf(" GetTableInfo 检索出错, 错误信息为:%s \n",err);
        g_db_dci.FreeReadData();
        return FALSE;
    }
    rec_num = g_db_dci.RowNum();

              if (rec_num != 1)
{
    cout << " 该表不存在或表号不唯一 " << endl;
    g_db_dci.FreeReadData();

        return  FALSE;
    }

    TTableInfo_ptr = new TTableInfo[rec_num];
    ret_value = g_db_dci.GetAlignResult(TTableInfo_ptr,col_info,sizeof(TTableInfo));
    if (ret_value < 0)
{
    cout << " GetTableInfo 内存分配出错 -- 对齐 " << endl;
    delete [] TTableInfo_ptr;
        g_db_dci.FreeReadData();

        return  FALSE;
    }

    delete [] TTableInfo_ptr;
    g_db_dci.FreeReadData();



    return  TRUE;

}

#endif



#if 0
bool CSqlTool::AllIdChange(VClassifyResult& InsertResult)
{
    cout << "AllIdChange has been called! " << endl;

    int i,table_num;
    table_num = InsertResult.size();

    for(i=0; i<table_num; i++)
    {
        if(TableIdChange(InsertResult[i]) == FALSE)
        {
            printf("TableIdChange baocuo");
        }

    }

    return TRUE;

}


bool CSqlTool::TableIdChange(ClassifyResult& tableResult)
{
    int i,j,sql_num,field_num;
    short                   data_type;

    long            max_key_id,old_id;
    char            key_string[100];
    string          sql_string;
    string          value_command;
    string          field_value;
    bool is_key;

    long      last_max_key_id=-1;//记录的一条记录的longid
    long      last_region_id_num=-1;//记录上一条记录的区域号
    long      last_get_key_id = -1;
    char * table_name;
    char * column_name;

    long table_no = tableResult.table_column_info.table_no;

    table_name = (char*)malloc(32*sizeof(char));
    sprintf(table_name,tableResult.table_column_info.table_name.c_str());


    //如果当前记录与上一条记录区域一致 则人为上条记录的longid为最大id
    sql_num = tableResult.VOnesqlResult.size();
    //每次处理一条sql
    for(i=0; i<sql_num; i++)
    {

        // 构造 SQL 语句
        field_num = tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec.size();

        sql_string    = "insert into ";
        sql_string   += tableResult.VOnesqlResult[i].sqlAnlsResult.table_name;
        sql_string   += " (";
        value_command = " values (";
        //每次添加一个域的信息
        for(j=0; j<field_num; j++)
        {
            max_key_id = -1;
            old_id     = -1;
            //添加域名
            sql_string += tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec[j];
            //添加域值
            data_type = tableResult.VOnesqlResult[i].sqlAnlsResult.column_value_vec[j].const_type;
            field_value.clear();
            field_value = GetValueByConstInfo(data_type,tableResult.VOnesqlResult[i].sqlAnlsResult.column_value_vec[j]);

            is_key = -1;

            is_key = IS_Key(tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec[j].c_str(),tableResult.table_column_info);
            //该表主键自动生成且该域是主键时自动生成主键
            if((is_key == 1) && (tableResult.table_info.key_generate_type == 1))
            {
                column_name = (char*)malloc(32*sizeof(char));
                snprintf(column_name,32,"%s",tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec[j].c_str());

                //判断是否分区
                if(tableResult.table_info.reserved_4 == 1)
                {

                    //获取region_id_num
                    long region_id_num = GetRegionId(tableResult.VOnesqlResult[i].sqlAnlsResult);
                    max_key_id = GetMaxKeyIDInRegion(region_id_num,column_name,table_no,table_name,
                                                     tableResult.VOnesqlResult[i].sqlAnlsResult, last_region_id_num,last_max_key_id);
                }
                else
                {
                    max_key_id = GetMaxKeyID(column_name,table_name,table_no,last_get_key_id);//----------
                }

                if(max_key_id < 0)
                {
                    cout << "	 GetMaxKeyID 失败! " << endl;
                    return FALSE;

                }
                cout << "max_key_id " << max_key_id<<endl;
                max_key_id += 1;
                if(last_get_key_id > 0)
                {
                    last_get_key_id += 1;
                }
                if(last_region_id_num!=-1)// 优化区域最大longid逻辑
                {
                    last_max_key_id +=1;
                }
                sprintf(key_string, "%ld", max_key_id);
                value_command += key_string;

                //生成id对照表
                old_id = atol(field_value.c_str());
                id_relation[old_id] = max_key_id;

                free(column_name);

            }
            else
            {
                if (tableResult.VOnesqlResult[i].sqlAnlsResult.column_value_vec[j].is_null == true)
                {
                    value_command += "NULL";
                }
                else
                {
                    value_command += field_value;
                }

            }
            if(j < field_num - 1)
            {
                sql_string    += ",";
                value_command += ",";

            }


        }

        sql_string   += ")";
        value_command += ")";

        sql_string += value_command;

        tableResult.VOnesqlResult[i].sql.clear();
        tableResult.VOnesqlResult[i].sql = sql_string;

    }

    free(table_name);


    return TRUE;
}

bool CSqlTool::IS_Key(const char* column_name,TTableColumnInfo& table_column_info)
{
    int             i;
    int             col_num;

    col_num = table_column_info.all_col_vec.size();
    for (i = 0; i < col_num; i++)
    {
        if (strcmp(table_column_info.all_col_vec[i].column_name, column_name) == 0)
        {
            if(table_column_info.all_col_vec[i].is_key == 1)
            {

                return  true;
            }
            else
            {
                return  false;

            }
        }
    }

    return  false;


}

long CSqlTool::GetRegionId(TSqlAnlsResult&  sqlAnlsResult)
{
    int i,column_num;
    short data_type;
    string field_value;
    long region_id = 0;
    column_num = sqlAnlsResult.column_name_vec.size();

    for(i=0; i<column_num; i++)
    {
        if(sqlAnlsResult.column_name_vec[i] == "region_id")
        {
            data_type = sqlAnlsResult.column_value_vec[i].const_type;
            field_value.clear();
            field_value = GetValueByConstInfo(data_type,sqlAnlsResult.column_value_vec[i]);
            if(field_value == "")
            {
                return 0;
            }
            else
            {
                region_id = atol(field_value.c_str());
                return region_id;
            }

        }

    }
    return 0;
}

// 检索得到当前表和域的最大关键字值
// 输入参数 ：  column_name             域名
//              table_name              表名
//              table_no                表号
// 返回值   ：  max_key_id              当前最大的关键字值，如果执行过程出错，返回 -1
long CSqlTool::GetMaxKeyID(char * column_name, char * table_name, long table_no,long& last_max_key_id)
{
    long             max_key_id;
    long             ret_value;
    char            sql_string[200];
    struct  TMaxID
    {
        long     max_id;
    } max_id_stru;


    ColInfo_Stru attr_info;


    attr_info.data_size = sizeof(long);
    attr_info.data_type = C_DATATYPE_LONG;




    if(last_max_key_id != -1)
    {
        return last_max_key_id;
    }

    max_key_id = -1;

    sprintf(sql_string, "select max(%s) from %s", column_name, table_name);

    //执行sql语句  获取内容
    if(g_db_dci.ReadData(sql_string.c_str() < 0))
    {
        string err = g_db_dci.ErrMsg();
        printf(" GetMaxKeyID 检索出错, 错误信息为:%s \n",err);
        g_db_dci.FreeReadData();
        return FALSE;
    }

    //对齐数据
    ret_value = g_db_dci.GetAlignResult(&max_id_stru,&attr_info,sizeof(TMaxID));
    if (ret_value < 0)
    {
        cout << " GetTableInfo 内存分配出错 -- 对齐 " << endl;
        g_db_dci.FreeReadData();

        return  FALSE;
    }

    g_db_dci.FreeReadData();


    max_key_id = max_id_stru.max_id;
    if (max_key_id == 0)
    {

        max_key_id = table_no << 48;
    }


    if (table_no != (max_key_id >> 48))
    {
        cout << " 自动生成关键字错误(表号不匹配) max_key_id ="  << max_key_id<<",  table_no = "<<table_no << endl;
        return  -1;
    }


    last_max_key_id = max_key_id;

    return  max_key_id;

}

long CSqlTool::GetMaxKeyIDInRegion(const long region_id_num,
                                   const char* key_column_name,
                                   const long table_no,
                                   const char* table_name,
                                   const TSqlAnlsResult&  sqlAnlsResult,
                                   long &last_region_id_num,long &last_max_key_id)
{
    long             max_key_id;
    long             ret_value;
    char            sql_string[200];
    short                   data_type;
    string           field_value;
    int             rec_num;


    long    region_id = 0;
    long    tmp_long = 0;
    bool    has_region = false;
    string  region_column_name;



    long st_id_var = 0;
    long bv_id_var = 0;


    struct  TRegionId
    {
        int region_id;
    } region_id_stru;

    ColInfo_Stru attr_info;

    attr_info.data_size = sizeof(int);
    attr_info.data_type = C_DATATYPE_INT;


    cout << "GetMaxKeyIDInRegion has been called! " << endl;



    if ( region_id_num > 0 )
    {
        region_id = region_id_num;
        has_region = true;
    }
    else    //没有填写region_id
    {

        //对region_id没有填写情况进行处理
        //图形信息表、电压类型表、电力公司表、区域表、厂站表、T交流线段表，交流线段表、直流线段表、公式定义表,断面信息表
        //如果没有region_id信息,则返回-1
        // 新增428  直流接地端表
        if ((table_no == 94) || (table_no == 405) || (table_no == 401)
            || (table_no == 403)|| (table_no == 404) || (table_no == 413)
            || (table_no == 414)|| (table_no == 426) || (table_no == 428) || (table_no == 437)
            || (table_no == 605))
        {
            return -1;
        }
        else if( table_no == 438 )  //断面表使用sec_id来计算所属区域
        {
            region_column_name = "sec_id";
            for (int i = 0; i < sqlAnlsResult.column_name_vec.size(); i++)
            {
                if(sqlAnlsResult.column_name_vec[i] == region_column_name)
                {

                    data_type = sqlAnlsResult.column_value_vec[i].const_type;
                    field_value.clear();
                    field_value = GetValueByConstInfo(data_type,sqlAnlsResult.column_value_vec[i]);
                    if(field_value == "")
                    {
                        cout << "###Get sec_id value fail!" << endl;
                        return -1;
                    }
                    else
                    {
                        tmp_long = atol(field_value.c_str());

                        if(tmp_long <=0)
                        {
                            return -1;
                        }

                        sprintf(sql_string, "select region_id from secinfo where id=%ld", tmp_long);

                        //执行sql语句  获取内容
                        if(g_db_dci.ReadData(sql_string.c_str() < 0))
                        {
                            string err = g_db_dci.ErrMsg();
                            printf(" GetMaxKeyIDInRegion1 检索出错, 错误信息为:%s \n",err);
                            g_db_dci.FreeReadData();
                            return FALSE;
                        }
                        //对齐数据
                        ret_value = g_db_dci.GetAlignResult(&region_id_stru,&attr_info,sizeof(TRegionId));
                        if (ret_value < 0)
                        {
                            cout << " GetMaxKeyIDInRegion1 内存分配出错 -- 对齐 " << endl;
                            g_db_dci.FreeReadData();

                            return  FALSE;
                        }


                        region_id = region_id_stru.region_id;
                        has_region = true;

                        g_db_dci.FreeReadData();

                    }
                }

            }
        }
        else
        {
            for (int i = 0; i < sqlAnlsResult.column_name_vec.size(); i++)
            {
                if(sqlAnlsResult.column_name_vec[i] == "st_id")
                {
                    data_type = sqlAnlsResult.column_value_vec[i].const_type;
                    field_value.clear();
                    field_value = GetValueByConstInfo(data_type,sqlAnlsResult.column_value_vec[i]);
                    if(field_value == "")
                    {
                        cout << "###Get sec_id value fail!" << endl;
                        return -1;
                    }
                    else
                    {
                        tmp_long = atol(field_value.c_str());

                        if(tmp_long <=0)
                        {
                            return -1;//厂站ID不能为空
                        }
                        else
                        {
                            st_id_var = tmp_long;

                        }
                    }
                }
                else if (sqlAnlsResult.column_name_vec[i] == "bv_id")
                {
                    data_type = sqlAnlsResult.column_value_vec[i].const_type;
                    field_value.clear();
                    field_value = GetValueByConstInfo(data_type,sqlAnlsResult.column_value_vec[i]);
                    if(field_value == "")
                    {
                        cout << "###Get bv_id value fail!" << endl;
                        return -1;
                    }
                    else
                    {
                        tmp_long = atol(field_value.c_str());
                        if (tmp_long <=0)
                        {
                            bv_id_var = 0;
                        }
                        else
                        {
                            bv_id_var = tmp_long;
                        }

                    }

                }

            }

            // 当电压类型不为空时，断设备所属电压等级的region_id
            if(bv_id_var != 0)
            {
                sprintf(sql_string, "select region_id from voltagelevel where st_id=%ld and bv_id=%ld", st_id_var, bv_id_var);


                //执行sql语句  获取内容
                if(g_db_dci.ReadData(sql_string.c_str() < 0))
                {
                    string err = g_db_dci.ErrMsg();
                    printf(" GetMaxKeyIDInRegion2 检索出错, 错误信息为:%s \n",err);
                    g_db_dci.FreeReadData();
                    return FALSE;
                }

                rec_num = g_db_dci.RowNum();
                if(rec_num==0)
                {
                    region_id = 0;  //NULL情况 region_id赋值为0
                }
                else                 	
                {
                     //对齐数据
                    ret_value = g_db_dci.GetAlignResult(&region_id_stru,&attr_info,sizeof(TRegionId));
                    if (ret_value < 0)
                    {
                        cout << " GetMaxKeyIDInRegion2 内存分配出错 -- 对齐 " << endl;
                        g_db_dci.FreeReadData();

                        return  FALSE;
                    }


                    region_id = region_id_stru.region_id;
                    has_region = true;
                }

                g_db_dci.FreeReadData();


//              cout<<"====In GetMaxKeyIDInRegion 从电压等级表中取得  region_id = "<<region_id<<endl;
            }

            // 当bv_id为空  或  电压等级表中region_id为空时
            // 获取厂站表中region_id
            if(region_id == 0)
            {
                sprintf(sql_string, "select region_id from substation where id=%ld", st_id_var);

				//执行sql语句  获取内容
				if(g_db_dci.ReadData(sql_string.c_str() < 0))
				{
					string err = g_db_dci.ErrMsg();
					printf(" GetMaxKeyIDInRegion3 检索出错, 错误信息为:%s \n",err);
					g_db_dci.FreeReadData();
					return FALSE;
				}
				//对齐数据
				ret_value = g_db_dci.GetAlignResult(&region_id_stru,&attr_info,sizeof(TRegionId));
				if (ret_value < 0)
				{
					cout << " GetMaxKeyIDInRegion3 内存分配出错 -- 对齐 " << endl;
					g_db_dci.FreeReadData();
				
					return	FALSE;
				}
				
				
				region_id = region_id_stru.region_id;
				has_region = true;
				
				g_db_dci.FreeReadData();

//              cout<<"====In GetMaxKeyIDInRegion 从厂站表中取得  region_id = "<<region_id<<endl;
            }


        }

    }   // end of if ( region_id_num > 0 )

//--------------------------------------
    if (!has_region)
    {
        return -1;
    }
//    else
//    {
//        int tmp_int;
//        tmp_int = region_id;
//        if (find(g_modify_region_vec.begin(),g_modify_region_vec.end(),tmp_int) == g_modify_region_vec.end())
//        {
//            return -2;
//        }
//    }
//-----------------------------------bernie

    struct  TMaxID
    {
        long     max_id;
    } max_id_stru;


    ColInfo_Stru attr_info1;


    attr_info1.data_size = sizeof(long);
    attr_info1.data_type = C_DATATYPE_LONG;

    max_key_id = -1;

    if (region_id > 0)
    {
        //add byshiyong 20160802  优化获取最大longid  减少与历史库交互
        if (last_region_id_num == region_id)
        {
            return last_max_key_id;
        }
        int tmp_int;
        tmp_int =  region_id << 24;

        sprintf(sql_string, "select max(%s) from %s where %s&0xFF000000 = %d", key_column_name, table_name, key_column_name, tmp_int);
    }
    else    // 是否没有必要,上述g_modify_region_vec.end() 已经保证 record_id > 0
    {
        sprintf(sql_string, "select max(%s) from %s", key_column_name, table_name);
    }

    //执行sql语句  获取内容
    if(g_db_dci.ReadData(sql_string.c_str() < 0))
    {
        string err = g_db_dci.ErrMsg();
        printf(" GetMaxKeyIDInRegion4 检索出错, 错误信息为:%s \n",err);
        g_db_dci.FreeReadData();
        return FALSE;
    }
	
    //对齐数据
    ret_value = g_db_dci.GetAlignResult(&max_id_stru,&attr_info,sizeof(TMaxID));
    if (ret_value < 0)
    {
        cout << " GetMaxKeyIDInRegion4 内存分配出错 -- 对齐 " << endl;
        g_db_dci.FreeReadData();

        return  max_key_id;
    }
	max_key_id = max_id_stru.max_id;

    g_db_dci.FreeReadData();


    //max_key_id = max_id_stru.max_id;
    if (max_key_id == 0)
    {

        if (region_id > 0)
        {
            int tmp_int;
            tmp_int =  region_id << 24;
            max_key_id = (table_no << 48)|tmp_int;
        }
        else
        {
            max_key_id = table_no << 48;
        }
    }

    if (table_no != (max_key_id >> 48))
    {
        cout << " 自动生成关键字错误(表号不匹配) max_key_id ="  << max_key_id<<",  table_no = "<<table_no << endl;
        return  -1;
    }


    //add byshiyong 20160802  优化获取最大longid  减少与历史库交互
    //仅记录最近的区域大于0的状态  其他情况不记录
    if(region_id>0)
    {
        //记录区域id  和查到的最大longid
        last_region_id_num = region_id;
        last_max_key_id = max_key_id;
    }//针对获取到了区域id但是区域id不大于0的情况
    //理论上经过上面g_modify_region_vec.end() 的判断 已经保证 record_id > 0
    else
    {
        long tmp_region_id =(max_key_id&0xFF000000)>>24;
        if(tmp_region_id>0 && last_region_id_num == tmp_region_id)
            //仅记录区域大于0的状态     同时也 避免-1的情况last_region_id_num-1代表未记录状态
        {
            // 更新上一记录区域状态
            //避免无区域获取方式获取最大的longid和上一条有区域数据longid
            //同属一区域,导致上一状态区域记录的longid不是最新最大的
            last_max_key_id = max_key_id;
        }
    }



    return  max_key_id;

}




#endif


#if 0
bool CSqlTool::AllMeasChange(AnlsResult& anlsResult)
{
    cout << "AllMeasChange has been called! " << endl;

    int i,table_num;
    int table_no;
    table_num = anlsResult.Insert.size();
    for(i=0; i<table_num; i++)
    {
        table_no = anlsResult.Insert[i].table_column_info.table_no;
        if((table_no == 431) || (table_no == 432))
        {
            if(InsertTableRecord(anlsResult.Insert[i]) == FALSE)
            {
                printf("插入sql更改量测id出错!");
                return FALSE;
            }
        }

    }

    table_num = anlsResult.Update.size();
    for(i=0; i<table_num; i++)
    {
        table_no = anlsResult.Update[i].table_column_info.table_no;
        if((table_no == 431) || (table_no == 432))
        {
            if(UpdateTableRecord(anlsResult.Update[i]) == FALSE)
            {
                printf("更新sql更改量测id出错!");
                return FALSE;
            }

        }

    }

    return TRUE;
}


bool CSqlTool::InsertTableRecord(ClassifyResult& tableResult)
{
    int i,j,sql_num,field_num;
    short                   data_type;

    long           tmp_id;
    char           tmp_str[64];
    string          sql_string;
    string          value_command;
    string          field_value;
    bool is_key;
    map<long,long>::iterator iter;


    sql_num = tableResult.VOnesqlResult.size();
    //每次处理一条sql
    for(i=0; i<sql_num; i++)
    {
        // 构造 SQL 语句
        field_num = tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec.size();
        sql_string    = "insert into ";
        sql_string   += tableResult.VOnesqlResult[i].sqlAnlsResult.table_name;
        sql_string   += " (";
        value_command = " values (";
        //每次添加一个域的信息
        for(j=0; j<field_num; j++)
        {

            //添加域名
            sql_string += tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec[j];
            cout << "*******field_name "<< tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec[j]<<endl;
            //添加域值
            data_type = tableResult.VOnesqlResult[i].sqlAnlsResult.column_value_vec[j].const_type;
            field_value.clear();
            field_value = GetValueByConstInfo(data_type,tableResult.VOnesqlResult[i].sqlAnlsResult.column_value_vec[j]);

            is_key = -1;

            is_key = IS_Key(tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec[j].c_str(),tableResult.table_column_info);
            //该表主键自动生成且该域是主键时自动生成主键
            if((is_key == 1) && (tableResult.table_info.key_generate_type == 1))
            {

                tmp_id = atol(field_value.c_str());
                iter = id_relation.find(tmp_id);

                if(iter!=id_relation.end())
                {
                    long key_id = iter->second;
                    sprintf(tmp_str, "%ld", key_id);
                    field_value.clear();
                    field_value = tmp_str;
                }
            }
            else if((strcmp(tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec[j].c_str(), "pnt_id") == 0))
            {
                tmp_id = atol(field_value.c_str());

                //取出属性号记录
                long field_no =  (tmp_id & 0x0000ffff00000000) >> 32;
                //pnt_id 属性号置0
                long pnt_id_no =  (tmp_id & 0xffff0000ffffffff);
                //获取id对照表id
                iter = id_relation.find(pnt_id_no);
                if(iter!=id_relation.end())
                {
                    long key_id = iter->second;
                    //置上属性号
                    long pnt_id = (key_id | (field_no << 32) );

                    sprintf(tmp_str, "%ld", pnt_id);
                    field_value.clear();
                    field_value = tmp_str;


                }


            }
            else if((strcmp(tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec[j].c_str(), "alg_id") == 0))
            {
                tmp_id = atol(field_value.c_str());

                //取出属性号记录
                long field_no =  (tmp_id & 0x0000ffff00000000) >> 32;
                //alg_id 属性号置0
                long alg_id_no =  (tmp_id & 0xffff0000ffffffff);

                cout << "*************id 值"<< alg_id_no<<endl;
                //获取id对照表id
                iter = id_relation.find(alg_id_no);
                if(iter!=id_relation.end())
                {
                    long key_id = iter->second;
                    //置上属性号
                    long alg_id = (key_id |(field_no << 32));

                    sprintf(tmp_str, "%ld", alg_id);
                    field_value.clear();
                    field_value = tmp_str;


                }


            }


            if (tableResult.VOnesqlResult[i].sqlAnlsResult.column_value_vec[j].is_null == true)
            {
                value_command += "NULL";
            }
            else
            {
                value_command += field_value;
            }
            if(j < field_num - 1)
            {
                sql_string    += ",";
                value_command += ",";

            }

        }

        sql_string   += ")";
        value_command += ")";

        sql_string += value_command;

        tableResult.VOnesqlResult[i].sql.clear();
        tableResult.VOnesqlResult[i].sql = sql_string;



    }



    return TRUE;
}

bool CSqlTool::UpdateTableRecord(ClassifyResult& tableResult)
{
    int field_num,i,j,sql_num;
    short data_type;
    long tmp_id,key_id;
    char tmp_str[64];

    string field_value;
    string sql_string;
    string where_string;
    map<long,long>::iterator iter;

    sql_num = tableResult.VOnesqlResult.size();
    for(i=0; i<sql_num; i++)
    {
        // 构造 SQL 语句
        field_num = tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec.size();


        // WHERE 子句
        if (GetWhereExpressionByAnlsResult(where_string,tableResult.VOnesqlResult[i].sqlAnlsResult) == FALSE)
        {
            cout << " 根据语法分析结果生成WHERE表达式出错 " << endl;
            return  false;
        }

        // SET 子句
        sql_string  = "update ";
        sql_string += tableResult.VOnesqlResult[i].sqlAnlsResult.table_name;
        sql_string += " set ";
        for (j = 0; j < field_num; j++)
        {

            sql_string += tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec[j];
            sql_string += "=";

            if (tableResult.VOnesqlResult[i].sqlAnlsResult.column_value_vec[j].is_null == true)
            {
                sql_string += "NULL";
            }
            else
            {

                data_type = tableResult.VOnesqlResult[i].sqlAnlsResult.column_value_vec[j].const_type;
                field_value.clear();
                field_value = GetValueByConstInfo(data_type,tableResult.VOnesqlResult[i].sqlAnlsResult.column_value_vec[j]);



                if((strcmp(tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec[j].c_str(), "pnt_id") == 0))
                {
                    tmp_id = atol(field_value.c_str());

                    //取出属性号记录
                    long field_no =  (tmp_id & 0x0000ffff00000000) >> 32;
                    //pnt_id 属性号置0
                    long pnt_id_no =  (tmp_id & 0xffff0000ffffffff);
                    //获取id对照表id
                    iter = id_relation.find(pnt_id_no);
                    if(iter!=id_relation.end())
                    {
                        key_id = iter->second;
                        //置上属性号
                        long pnt_id = (key_id |(field_no << 32));

                        sprintf(tmp_str, "%ld", pnt_id);
                        field_value.clear();
                        field_value = tmp_str;

                    }


                }
                if((strcmp(tableResult.VOnesqlResult[i].sqlAnlsResult.column_name_vec[j].c_str(), "alg_id") == 0))
                {
                    tmp_id = atol(field_value.c_str());

                    //取出属性号记录
                    long field_no =  (tmp_id & 0x0000ffff00000000) >> 32;
                    //alg_id 属性号置0
                    long alg_id_no =  (tmp_id & 0xffff0000ffffffff);
                    //获取id对照表id
                    iter = id_relation.find(alg_id_no);
                    if(iter!=id_relation.end())
                    {
                        key_id = iter->second;
                        //置上属性号
                        long alg_id = (key_id |(field_no << 32));

                        sprintf(tmp_str, "%ld", alg_id);
                        field_value.clear();
                        field_value = tmp_str;

                    }


                }


                if((strcmp(field_value.c_str(), "0") == 0))
                {
                    sql_string += "NULL";
                }
                else
                {
                    sql_string += field_value;
                }
            }

            if (i < (field_num - 1))
            {
                sql_string += ",";
            }


        }
        sql_string += where_string;


        tableResult.VOnesqlResult[i].sql.clear();
        tableResult.VOnesqlResult[i].sql = sql_string;



    }




    return TRUE;
}



// 从运行堆栈中得到并移除处于栈顶的元素
// 输入参数 ：  where_stack             运行堆栈
// 输出参数 ：  where_stack             变化以后的堆栈
//              str                     栈顶字符串
// 返回值   ：  TRUE                    成功
//              FALSE                   失败，堆栈为空
bool CSqlTool::PopFromStack(stack<string> & where_stack, string & str)
{
    if (where_stack.empty() == true)
    {
        return  false;
    }

    str = where_stack.top();
    where_stack.pop();

    return  true;
}


// 向运行堆栈中压入一个字符串元素
// 输入参数 ：  where_stack             运行堆栈
//              str                     字符串元素
// 输出参数 ：  where_stack             变化以后的堆栈
// 返回值   ：  无
void CSqlTool::PushIntoStack(stack<string> & where_stack, string & str)
{
    where_stack.push(str);
}
// 根据TConstInfo型的常量，再结合不同的商用数据库语法，转换成为字符串
// 输入参数 ：  const_info              常量
// 输出参数 ：  const_string            经过转换以后的字符串
// 返回值   ：  TRUE                    成功
//              FALSE                   失败
bool CSqlTool::GetSqlFormatByConstInfo(const TConstInfo & const_info, string & const_string)
{
    char            tmp_str[4096];
    struct tm *     tmp_time;

    const_string = "";

    if (const_info.is_null == TRUE)
    {
        const_string = "NULL";

        return  true;
    }
    switch (const_info.const_type)
    {
        case    INTPRE_DATATYPE_INT :
            sprintf(tmp_str, "%ld", const_info.const_value.int_value);
            break;

        case    INTPRE_DATATYPE_REAL :
            sprintf(tmp_str, "%f", const_info.const_value.real_value);
            break;

        case    INTPRE_DATATYPE_STRING :
            sprintf(tmp_str, "'%s'", const_info.const_value.string_value);
            break;

        case    INTPRE_DATATYPE_BINARY :
            sprintf(tmp_str, "%s",   const_info.const_value.binary_value);
            break;

        case    INTPRE_DATATYPE_DATETIME :
            tmp_time = localtime((const time_t *)&const_info.const_value.int_value);
            sprintf(tmp_str, "to_date('%04d-%02d-%02d %02d:%02d:%02d', 'yyyy-mm-dd hh24:mi:ss')",
                    tmp_time->tm_year + 1900,
                    tmp_time->tm_mon + 1,
                    tmp_time->tm_mday,
                    tmp_time->tm_hour,
                    tmp_time->tm_min,
                    tmp_time->tm_sec);
            break;

        case    INTPRE_DATATYPE_KEYID :
            cout << " GetSqlFormatByConstInfo 暂时不支持KEYID类型 " << endl;
            return  false;

        case    INTPRE_DATATYPE_APPKEYID :
            cout << " GetSqlFormatByConstInfo 暂时不支持APPKEYID类型 " << endl;
            return  false;

        case    INTPRE_DATATYPE_APPID :
            cout << " GetSqlFormatByConstInfo 暂时不支持APPID类型 " << endl;
            return  false;

        case    INTPRE_DATATYPE_TEXT :
            cout << " GetSqlFormatByConstInfo 暂时不支持TEXT类型 " << endl;
            return  false;

        case    INTPRE_DATATYPE_IMAGE :
            cout << " GetSqlFormatByConstInfo 暂时不支持IMAGE类型 " << endl;
            return  false;

        default :
            cout << " GetSqlFormatByConstInfo 不认识的数据类型 " << const_info.const_type << endl;
            return  false;

    }

    const_string = tmp_str;

    return  true;
}

// 根据操作符和操作数，再结合不同的商用数据库语法，拼接成为新的表达式
// 最后两边加上括号
// 输入参数 ：  operator_code           操作符
//              operand_1               操作符左边的操作数，对于单目操作符不用
//              operand_2               操作符右边的操作数，对于单目操作符使用
// 输出参数 ：  const_string            经过转换以后的字符串
// 返回值   ：  TRUE                    成功
//              FALSE                   失败
bool CSqlTool::GetSqlFormatByOperator(const int operator_code, const string operand_1,
                                      const string operand_2, string & expression)
{
    char *          tmp_str;

    tmp_str = (char *)malloc((operand_1.length() + operand_2.length() + 100) * sizeof(char));

    switch (operator_code)
    {
        case    OPERATOR_CODE_ADD :
            sprintf(tmp_str, "%s + %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_SUB :
            sprintf(tmp_str, "%s - %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_MUL :
            sprintf(tmp_str, "%s * %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_DIV :
            sprintf(tmp_str, "%s / %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_IDV :
            sprintf(tmp_str, "floor(%s / %s)", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_MOD :
            sprintf(tmp_str, "mod(%s, %s)", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_ANB :
            sprintf(tmp_str, "bit_op_and(%s, %s)", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_ORB :
            sprintf(tmp_str, "bit_op_or(%s, %s)", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_XOR :
            sprintf(tmp_str, "bit_op_xor(%s, %s)", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_EQU :
            if (operand_2 == "NULL")     // 如果右边的操作数是NULL的话，用 is 操作符
            {
                sprintf(tmp_str, "%s is %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            }
            else
            {
                sprintf(tmp_str, "%s = %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            }
            break;

        case    OPERATOR_CODE_NEQ :
            if (operand_2 == "NULL")     // 如果右边的操作数是NULL的话，用 is not 操作符
            {
                sprintf(tmp_str, "%s is not %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            }
            else
            {
                sprintf(tmp_str, "%s != %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            }
            break;

        case    OPERATOR_CODE_GRT :
            sprintf(tmp_str, "%s > %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_GEQ :
            sprintf(tmp_str, "%s >= %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_LSS :
            sprintf(tmp_str, "%s < %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_LEQ :
            sprintf(tmp_str, "%s <= %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_LKE :
            sprintf(tmp_str, "%s like %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_POS :
            sprintf(tmp_str, "%s", (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_NEG :
            sprintf(tmp_str, "-%s", (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_AND :
            sprintf(tmp_str, "%s and %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_ORR :
            sprintf(tmp_str, "%s or %s", (char *)operand_1.c_str(), (char *)operand_2.c_str());
            break;

        case    OPERATOR_CODE_NOT :
            sprintf(tmp_str, "not %s", (char *)operand_2.c_str());
            break;

        default :
            cout << " GetSqlFormatByOperator 不认识的操作符 " << operator_code << endl;
            free(tmp_str);
            return  false;
    }

    expression  = "(";
    expression += tmp_str;
    expression += ")";

    free(tmp_str);

    return  true;

}


// 根据语法分析的WHERE子句逆波兰序列，根据不同的数据库种类构造符合该数据库的相应的WHERE子句
// 主要算法 ：  设置一个运行堆栈，系统从逆波兰序列中取出常量或者标识符PUSH进堆栈，遇到运算符
//              时，从堆栈中POP出一个或者两个操作数（根据操作符的固有属性），然后根据不同的
//              数据库种类生成相应的语句，PUSH进堆栈顶。这期间，根据运算符的优先等级决定是否
//              在两边加上括号。循环此过程，最后就可以得到符合某个特定数据库SQL语法的WHERE子句。
// 输入参数 ：
// 输出参数 ：  where_str               生成的WHERE子句
// 返回值   ：  TRUE                    成功
//              FALSE                   失败
bool CSqlTool::GetWhereExpressionByAnlsResult(string & where_str,
        TSqlAnlsResult& sqlAnlsResult)
{
    int             i;
    int             where_code_len;
    int             last_type;
    stack<string>   where_stack;
    string          stack_string;
    string          operand_1;
    string          operand_2;

    where_str = "";

    where_code_len = sqlAnlsResult.where_code_vec.size();
    if (where_code_len == 0)
    {
        return  true;
    }

    for (i = 0; i < where_code_len; i++)
    {
        // 如果是标识符，则将标识符压入运行堆栈
        if (sqlAnlsResult.where_code_vec[i].code_type == CODE_TYPE_IDENTIFIER)
        {
            stack_string = sqlAnlsResult.where_code_vec[i].code_info.iden_name;
            PushIntoStack(where_stack, stack_string);
        }
        // 如果是常量，则将常量转换为字符串压入运行堆栈
        else if (sqlAnlsResult.where_code_vec[i].code_type == CODE_TYPE_CONSTANT)
        {
            if (GetSqlFormatByConstInfo(sqlAnlsResult.where_code_vec[i].code_info.const_info, stack_string) == FALSE)
            {
                cout << " GetSqlFormatByConstInfo 出错 " << endl;
                return  false;
            }
            PushIntoStack(where_stack, stack_string);
        }
        // 如果是操作符，则根据操作符的属性决定POP出一个还是两个元素出堆栈，
        // 然后拼接出表达式压入运行堆栈。
        // 如果当前操作符的优先级高于上一个操作符的优先级（也就是当前操作符
        // 的优先级代码小于上一个操作符的优先级代码），则需要加上括号。
        // 目前，先暂不考虑根据优先级别加上括号（上述的算法似乎有问题），
        // 而是在拼接的时候加上括号
        else if (sqlAnlsResult.where_code_vec[i].code_type == CODE_TYPE_OPERATOR)
        {
            if (PopFromStack(where_stack, operand_2) == FALSE)
            {
                cout << " 堆栈为空 出错 " << endl;
                return  false;
            }
            if ((sqlAnlsResult.where_code_vec[i].code_info.operator_code == OPERATOR_CODE_NOT) ||
                (sqlAnlsResult.where_code_vec[i].code_info.operator_code == OPERATOR_CODE_POS) ||
                (sqlAnlsResult.where_code_vec[i].code_info.operator_code == OPERATOR_CODE_NEG))
            {
                operand_1 = "";
            }
            else
            {
                if (PopFromStack(where_stack, operand_1) == FALSE)
                {
                    cout << " 堆栈为空 出错 " << endl;
                    return  false;
                }
            }

            // 根据操作符和两个操作数拼接新的表达式，此时返回的表达式两边自动加上括号
            if (GetSqlFormatByOperator(sqlAnlsResult.where_code_vec[i].code_info.operator_code,
                                       operand_1, operand_2, stack_string) == FALSE)
            {
                cout << " GetSqlFormatByOperator 出错 " << endl;
                return  false;
            }
            PushIntoStack(where_stack, stack_string);
        }
        else
        {
            cout << " WHERE的逆波兰序列中存在不认识的代码 " <<
                 (short)sqlAnlsResult.where_code_vec[i].code_type << endl;
            return  false;
        }

        last_type = sqlAnlsResult.where_code_vec[i].code_type;
    }

    // 最后取栈顶的字符串就是最终的WHERE子句
    if (last_type != CODE_TYPE_OPERATOR)
    {
        cout << " WHERE的逆波兰序列最后没有以操作符结尾 " << last_type << endl;
        return  false;
    }

    if (PopFromStack(where_stack, stack_string) == FALSE)
    {
        cout << " 堆栈为空，出错 " << endl;
        return  false;
    }

    where_str = " where ";
    where_str += stack_string;

    return  true;
}

/*
bool CSqlTool::measChange(TSqlAnlsResult& sqlAnlsResult,int table_no)
{
    int             i,j,field_num;
    int              key_num;
    short           data_type;
    long            id_value;
    long            pnt_id_value;
    char            tmp_str[128];

    string         sql_string;
    string         where_string;
    string        field_value;


    // 构造 SQL 语句
    field_num = sqlAnlsResult.column_name_vec.size();


    // WHERE 子句
    if (GetWhereExpressionByAnlsResult(where_string,sqlAnlsResult) == FALSE)
    {
        cout << " 根据语法分析结果生成WHERE表达式出错 " << endl;
        return  false;
    }
    // SET 子句
    sql_string  = "update ";
    sql_string += sqlAnlsResult.table_name;
    sql_string += " set ";

    for (j = 0; j < field_num; j++)
    {

        sql_string += sqlAnlsResult.column_name_vec[i];
        sql_string += "=";

        if (sqlAnlsResult.column_value_vec[i].is_null == true)
        {
            sql_string += "NULL";
        }
        else
        {

            data_type = sqlAnlsResult.column_value_vec[i].const_type;
            field_value.clear();
            field_value = GetValueByConstInfo(data_type,sqlAnlsResult.column_value_vec[i]);

            if((strcmp(sqlAnlsResult.column_name_vec[i].c_str(), "id") == 0))
            {
                id_value = atol(field_value.c_str());
                id_value = id_relation[id_value];

                sprintf(tmp_str, "%ld", id_value);
                field_value.clear();
                field_value = tmp_str;

            }

            if((strcmp(sqlAnlsResult.column_name_vec[i].c_str(), "pnt") == 0))
            {
                pnt_id_value = atol(field_value.c_str());

                //取出属性号记录
                long field_no =  (pnt_id_value & 0x0000ffff00000000) >> 32;
                //pnt_id 属性号置0
                long pnt_id_no =  (pnt_id_value & 0xffff0000ffffffff);
                //获取id对照表id
                pnt_id_no =  id_relation[pnt_id_no];
                //置上属性号
                pnt_id = (pnt_id_no &(field_no << 32));

            }

            //zhangkeheng add for deal with foreign key = 0 then set to NULL
//            if ((modified_column_vec[record_column_vec[i].relevant_no[j]].reference_mode == 1) && (strcmp(field_value, "0") == 0))
            if((strcmp(field_value, "0") == 0))
            {
                sql_string += "NULL";
            }
            else
            {
                sql_string += field_value;
            }
        }

        if (j < (field_num - 1))
        {
            sql_string += ",";
        }

        // 如果记录所属应用被修改，需要记住新的和旧的记录所属应用
        // 旧的记录所属应用需要从数据库中检索得到
//        if (table_all_info.is_record_app == 1)
//        {
//            if (strcmp(modified_column_vec[record_column_vec[i].relevant_no[j]].column_name, "record_app") == 0)
//            {
//                new_app_vec[i] = modify_request_stru.seq_update[i].seq_field_info[j].field_value.c_int();
//                old_app_vec[i] = GetRecordAppByWhere(table_all_info.table_name_eng, (char *)where_string.c_str());
//                //old_app_vec[i] = -1;
//            }
//        }
    }

    sql_string += where_string;


    return TRUE;

}


*/
#endif







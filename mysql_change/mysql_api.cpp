#include "mysql_api.h"




/************************************************************
**FUNCTION:
**  CMySqlApi()  ��ʼ��
************************************************************/
CMySqlApi::CMySqlApi()
{
    m_rowNum = 0;
    m_colNum = 0;
}

/************************************************************
**FUNCTION:
**  ~CMySqlApi() �ͷŽ����
************************************************************/
CMySqlApi::~CMySqlApi()
{

}

int CMySqlApi::DbConnect(const char *host,const char *user_name,const char *user_passwd,const char *dbname)
{
    char err_char[1024];
    if(NULL == mysql_init(&m_mySql))
    {
        printf("�ڴ治�㣡\n");
        m_errMsg = "�ڴ治�㣡";
        return MYSQL_FAIL;
    }

    if (!mysql_real_connect(&m_mySql,host,user_name,user_passwd,dbname,3306,NULL,0))
    {
        sprintf(err_char,"Failed to connect to database: Error: %s",mysql_error(&m_mySql));
        m_errMsg = err_char;
        cout<<err_char<<endl;

        return MYSQL_FAIL;
    }

    printf("connected\n");
    return MYSQL_SUCCEED;

}

int CMySqlApi::DbDisconnect()
{
    mysql_close(&m_mySql);
    printf("DbDisconnect\n");

    return MYSQL_SUCCEED;
}

int CMySqlApi::DoAlign(int* x, int b)
{
    int t = ~((b) - 1);
    *x= (((*x) + (b) - 1) & t);
    return 0;
}

int CMySqlApi::GetMaxDataTypeLength(int data_type,int attr_length,int * MaxLength)
{
    int data_type_length;

    if(data_type == C_MYSQL_TYPE_STRING)
    {
        data_type_length = 1;
    }
    else
    {
        data_type_length=attr_length;

    }
    if(*MaxLength<data_type_length)
        *MaxLength=data_type_length;
    return MYSQL_SUCCEED;

}
long CMySqlApi::TransStrTimeToLong(const char *timep,const char *format)
{
    time_t value = -1;
    struct tm tm = {0};

    if(strptime(timep,format,&tm) != NULL)
    {
        value = mktime(&tm);
    }
/*    else
    {
        printf("ʱ��ת��ʧ��\n");
        return MYSQL_FAIL;
    }
*/
/* 
    time_t t = value;
    struct tm * gmt = gmtime(&t);
    //printf("time: %d",t);
    printf(" Year: %d", gmt->tm_year + 1900);
    printf(" Month: %d", gmt->tm_mon + 1);
    printf(" Day: %d", gmt->tm_mday);
    printf(" Hour: %d", gmt->tm_hour + 8);
    printf(" Minute: %d", gmt->tm_min);
    printf(" Second: %d\n", gmt->tm_sec);

*/

    return (long)value;
}


int CMySqlApi::ReadData(const char * sql)
{
    int ret = -1;
    m_rowNum = 0;
    m_colNum = 0;
    char  err_char[1024];

    ret = mysql_query(&m_mySql,sql);
    if(!ret)
    {
        m_pRes=mysql_store_result(&m_mySql);
        if(m_pRes)
        {
            //��������Ŀ
            m_rowNum = mysql_num_rows(m_pRes);
            //����Ŀ
            m_colNum = mysql_num_fields(m_pRes);
            // printf("row %d m_colNum%d\n",m_rowNum,m_colNum);
            return MYSQL_SUCCEED;
        }
        else
        {
            sprintf(err_char,"��ȡ�����ʧ��! Error: %s",mysql_error(&m_mySql));
            m_errMsg = err_char;
            cout<<err_char<<endl;
            return MYSQL_FAIL;
        }
    }

    return MYSQL_FAIL;
}


void CMySqlApi::FreeReadData()
{
    mysql_free_result(m_pRes);
    printf("free m_pRes\n");

}

int CMySqlApi::GetResult(ResultInfo & result_vec)
{
    int row =0;
    int col = 0;
    char err_char[1024];
    MYSQL_ROW my_row;//������
    MYSQL_FIELD *my_field;//������
    vector<string> tmp_row;

    for(row = 0; row < m_rowNum; row++)
    {
        my_row = mysql_fetch_row(m_pRes);
        tmp_row.clear();
        for(col = 0; col < m_colNum; col++)
        {
            if(my_row[col])
            {
                tmp_row.push_back(my_row[col]);
            }
            else
            {
                sprintf(err_char,"�����,��%d����¼�ĵ�%d��Ϊ��!",row,col);
                m_errMsg = err_char;
                cout<<err_char<<endl;
                return MYSQL_FAIL;
            }
        }
        result_vec.push_back(tmp_row);

    }
    return MYSQL_SUCCEED;

}

//���νṹ��ͽṹ����Ϣ
int CMySqlApi::GetAlignResult(void* align_result_ptr,struct ColAttr * attrs_info, const int struct_length)
{
    int i,j,AlignLength,temprecord_offset,MaxDataTypeLength;
    AlignLength = 0;
    temprecord_offset = 0;
    MaxDataTypeLength = 1;

    char err_char[1024];

    MYSQL_ROW my_row;//������
    MYSQL_FIELD *my_field;//������
    int dataLen[MYSQL_MAX_COLS];
    int row, col;

    if(m_colNum<=0  ||m_rowNum<=0)
    {
        sprintf(err_char,"��¼���������Ը����쳣,�������,��¼��:%d ������:%d",m_rowNum,m_colNum);
        m_errMsg = err_char;
        cout<<err_char<<endl;
        return MYSQL_FAIL;
    }

    if(attrs_info==NULL)
    {
        //assert(field_info_ptr==NULL);
        sprintf(err_char,"����Ϣָ��attrs_infoΪ��,�������!!!");
        m_errMsg = err_char;
        cout<<err_char<<endl;
        return MYSQL_FAIL;
    }

    if(align_result_ptr==NULL)
    {
        //assert(align_result_ptr==NULL);
        sprintf(err_char,"������ָ��align_result_ptrΪ��,�������\n");
        m_errMsg = err_char;
        cout<<err_char<<endl;
        return MYSQL_FAIL;
    }

    //��ȡ������  ������
    for(i = 0; i<m_colNum; i++)
    {
        my_field = mysql_fetch_field(m_pRes);
        dataLen[i] = my_field->length;
    }

    for(row = 0; row < m_rowNum; row++)
    {
        temprecord_offset = 0;
        //��ȡһ������
        cout << "m_colNum: " << m_colNum << endl;
        my_row = mysql_fetch_row(m_pRes);
        for(col = 0; col < m_colNum; col++)
        {
             //    cout << "col: " << col << endl;
           //  cout << "data_type: " << attrs_info[col].data_type << " data_size: " << attrs_info[col].data_size << endl;
            switch(attrs_info[col].data_type)
            {
                
                case C_MYSQL_TYPE_STRING:
                {
                    DoAlign(&temprecord_offset, 1);
                    memcpy((char *)align_result_ptr+AlignLength+temprecord_offset,my_row[col],dataLen[col]);
                    break;
                }
                case C_MYSQL_TYPE_UCHAR:
                {
                    unsigned char  tmp = (unsigned char)atoi(my_row[col]);
                    DoAlign(&temprecord_offset, attrs_info[col].data_size);
                    memcpy((char *)align_result_ptr+AlignLength+temprecord_offset,&tmp,attrs_info[col].data_size);
                    break;
                }
                case C_MYSQL_TYPE_SHORT:
                {
                    short  tmp = (short)atoi(my_row[col]);
                    DoAlign(&temprecord_offset, attrs_info[col].data_size);
                    memcpy((char *)align_result_ptr+AlignLength+temprecord_offset,&tmp,attrs_info[col].data_size);
                    break;
                }
                case C_MYSQL_TYPE_INT:
                {
                    int  tmp = atoi(my_row[col]);
                    DoAlign(&temprecord_offset, attrs_info[col].data_size);
                    memcpy((char *)align_result_ptr+AlignLength+temprecord_offset,&tmp,attrs_info[col].data_size);
                    break;
                }
                case C_MYSQL_TYPE_FLOAT:
                {
                    float tmp = atof(my_row[col]);
                    DoAlign(&temprecord_offset, attrs_info[col].data_size);
                    memcpy((char *)align_result_ptr+AlignLength+temprecord_offset,&tmp,attrs_info[col].data_size);
                    break;
                }
                case C_MYSQL_TYPE_DOUBLE:
                {
                    cout << "my_row[col]: " << my_row[col] ;
                    double tmp = strtod(my_row[col],NULL);
                    cout << "tmp: " << tmp << endl;
                    DoAlign(&temprecord_offset, attrs_info[col].data_size);
                    memcpy((char *)align_result_ptr+AlignLength+temprecord_offset,&tmp,attrs_info[col].data_size);
                    break;                    
                }
                case C_MYSQL_TYPE_LONG:
                {
                    long tmp = atol(my_row[col]);
                    DoAlign(&temprecord_offset, attrs_info[col].data_size);
                    memcpy((char *)align_result_ptr+AlignLength+temprecord_offset,&tmp,attrs_info[col].data_size);
                    break;
                }
                case C_MYSQL_TYPE_DATETIME:
                {
                    //cout << "my_row[col]: " << my_row[col] << endl;
                    long tmp_time =  TransStrTimeToLong(my_row[col],"%Y-%m-%d %H:%M:%S");
                    DoAlign(&temprecord_offset, attrs_info[col].data_size);
                    memcpy((char *)align_result_ptr+AlignLength+temprecord_offset,&tmp_time,attrs_info[col].data_size);
                    break;
                }
                case  C_MYSQL_TYPE_KEYID :
                case  C_MYSQL_TYPE_APPKEYID :
                case  C_MYSQL_TYPE_APPID :
                case  C_MYSQL_TYPE_BINARY :
                case  C_MYSQL_TYPE_TEXT :
                case  C_MYSQL_TYPE_UINT:
                case  C_MYSQL_TYPE_IMAGE :
                {
                    sprintf(err_char,"��֧������:%d!",attrs_info[col].data_type);
                    m_errMsg = err_char;
                    cout<<err_char<<endl;
                    return  MYSQL_FAIL;
                }
                default:
                {
                    printf(err_char,"δʶ������:%d",attrs_info[col].data_type);
                    m_errMsg = err_char;
                    cout<<err_char<<endl;
                    return  MYSQL_FAIL;
                }

            }

            GetMaxDataTypeLength(attrs_info[col].data_type,attrs_info[col].data_size,&MaxDataTypeLength);
            temprecord_offset += attrs_info[col].data_size;


        }
       // cout << "temprecord_offset: " << temprecord_offset << endl;

        AlignLength += temprecord_offset;
        DoAlign(&AlignLength, MaxDataTypeLength);
    }

    if(AlignLength!=m_rowNum*struct_length)
    {
        sprintf(err_char,"AlignLength === %d , m_rowNum*struct_length === %d  �������!!! ",AlignLength, m_rowNum*struct_length);
        m_errMsg = err_char;
        cout<<err_char<<endl;
        return MYSQL_FAIL;
    }
    else
    {
        return MYSQL_SUCCEED;
    }



}
//�����ύ
int CMySqlApi::ExecTransCommit(const vector<string> &cmdbuf)
{
	int ret;
    char err_char[1024];
	if(ret = mysql_query(&m_mySql,"begin"))
	{
        sprintf(err_char,"ִ��beginʧ��! Error: %s",mysql_error(&m_mySql));
        m_errMsg = err_char;
        cout<<err_char<<endl;
		return MYSQL_FAIL;
	}
	for(int i = 0; i < cmdbuf.size(); i++)
	{
		cout<<__LINE__<<" sql = "<<cmdbuf.at(i)<<endl;
		if(ret = mysql_query(&m_mySql,cmdbuf.at(i).c_str()))
		{
            sprintf(err_char,"ִ�е�%d��sqlʧ��! Error: %s,  ִ������ȫ����",mysql_error(&m_mySql));
            m_errMsg = err_char;
            cout<<err_char<<endl;
			if( mysql_query(&m_mySql,"rollback"))
			{
                sprintf(err_char,"ִ�е�%d��sqlʧ��! ����ȫ����ʱ,�쳣,Error: %s",mysql_error(&m_mySql));
                m_errMsg +="\n";
                m_errMsg += err_char;
                cout<<err_char<<endl;
			}
			return MYSQL_FAIL;
		}
	}
	
	if(ret = mysql_query(&m_mySql,"commit"))
	{
        sprintf(err_char,"�����ύʧ��! Error: %s",mysql_error(&m_mySql));
        m_errMsg = err_char;
        cout<<err_char<<endl;
        
        if( mysql_query(&m_mySql,"rollback"))
        {
            sprintf(err_char,"�����ύʧ��! ִ������ȫ����ʱ,�쳣,Error: %s",mysql_error(&m_mySql));
            m_errMsg +="\n";
            m_errMsg += err_char;
            cout<<err_char<<endl;
        }
		return MYSQL_FAIL;
	}
	
	return MYSQL_SUCCEED;
}

//��һ�ύ
int CMySqlApi::ExecSingleCommit(const string &sql_str)
{
	int ret;
    char err_char[1024];
	if(ret = mysql_query(&m_mySql,sql_str.c_str()))
	{
        sprintf(err_char,"ִ��sqlʧ��! Error: %s",mysql_error(&m_mySql));
        m_errMsg = err_char;
        cout<<err_char<<endl;
		return MYSQL_FAIL;
	}
	
	return MYSQL_SUCCEED;
}
//��һ�ύ
int CMySqlApi::ExecSingleCommit(const char *sql_char)
{
	int ret;
    char err_char[1024];
	if(ret = mysql_query(&m_mySql,sql_char))
	{
        sprintf(err_char,"ִ��sqlʧ��! Error: %s",mysql_error(&m_mySql));
        m_errMsg = err_char;
        cout<<err_char<<endl;
		return MYSQL_FAIL;
	}
	
	return MYSQL_SUCCEED;
}

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <cstring>
#include <vector>

#include "sql_change.h"
#include "mysql_api.h"



int main(int argc, char **argv)
{
    TLoginInfo  login_info;
    CSqlTool sqlTest;
    vector<string>              sql_vec;

    sprintf(login_info.host,"192.100.1.40");
    sprintf(login_info.user_name,"root");
    sprintf(login_info.user_passwd,"123456");
	sprintf(login_info.dbname,"ems_hyd_ner");


    string   sql1 = "UPDATE disconnector SET name='�Ĵ�.���/220kV.2714��բzxk' WHERE id=114841791840124932";
    string   sql2 = "UPDATE measpoint SET st_id = 113997366322791449,name='�Ĵ�.���/220kV.2714��բzxk/ң��ֵ', record_app = 65535 WHERE id = 121315715717267485";
    string   sql3 = "UPDATE aclineend SET vl_id = 113152940654461037 WHERE id = 116812115351699909";
    string   sql4 = "INSERT disconnector (st_id,record_app,code,name,describe,bay_id,bv_id,vl_id,discr_type,nom_state,run_state,ind,jnd,replicate_flag,id) VALUES (113997366322791449,65535,'test','�Ĵ�.���/220kV.2714��բtest','',0,112871465677750279,113152941392660983,0,0,0,47644072214527,47644072214527,0,114841791840124932)";
    string   sql5 = "INSERT measpoint (id,name,st_id,pnt_id,record_app) VALUES (121315715717267485,'�Ĵ�.���/220kV.2714��բtest/ң��ֵ',113997366322791449,114841920689143812,65535)";

    sql_vec.push_back(sql1);
    sql_vec.push_back(sql2);
    sql_vec.push_back(sql3);
    sql_vec.push_back(sql4);
    sql_vec.push_back(sql5);


    //��¼���ݿ�
    if(!g_db_dci.Connect(login_info.host, login_info.user_name, login_info.user_passwd, login_info.dbname))
    {
        string err;
		err = g_db_dci.ErrMsg();
        printf("���ݿ�����ʧ��, ����: %s\n",err.c_str());
        return  FALSE;
    }
    else
    {
        printf("���ݿ����ӳɹ�!!!!!\n");
    }
//************************************************


//    int val = sqlTest.SqlProcess(sql_vec);



//*************************************************
//�˳����ݿ�
    if (!g_db_dci.DisConnect())
    {
        string err;
		err = g_db_dci.ErrMsg();
        printf("���ݿ�����ʧ��, ����: %s\n",err.c_str());

        return  FALSE;
    }
    else
    {
        printf("���ݿ�ע���ɹ�!!!!\n");
    }

    return 0;
}


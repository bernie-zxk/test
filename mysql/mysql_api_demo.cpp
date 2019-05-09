#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <cstring>
#include <vector>

#include "mysql_api.h"


int main(int argc, char *argv[])
{
    int ret = 0;
    CMySqlApi db_mysql;

    //-------------------begin
    struct  Test_Info
    {
        long     time;
        char      name[67];
        double     tmp;    
    };
    struct  Test_Info * test_ptr;
    ColInfo_Stru col_info[3];



    col_info[0].data_type = C_DATATYPE_DATETIME;
    col_info[0].data_size = sizeof(long);
    col_info[1].data_type = C_DATATYPE_CHAR;
    col_info[1].data_size = 67;
    col_info[2].data_type = C_DATATYPE_DOUBLE;
    col_info[2].data_size = sizeof(double);



    //--------------------------end


    db_mysql.db_connect("192.100.1.40","root","123456","ems_hyd_ner");
    ret = db_mysql.ReadData("select time,name,tmpd from az");
    if(ret <0)
    {
        printf("ReadData failed!\n");
        db_mysql.FreeReadData();
        db_mysql.db_disconnect();


        return 0;

    }
    //-------------------begin
    test_ptr = new Test_Info[db_mysql.m_rowNum];
    db_mysql.GetAlignResult(test_ptr,col_info,sizeof(Test_Info));


    for (int i = 0; i < oo.row_num; i ++)
    {
        cout << "time: " << test_ptr[i].time;
        cout << " tmp: " << test_ptr[i].tmp<< endl;;
        cout << " name: " << test_ptr[i].name<< endl;

    }
    delete [] test_ptr;


    //--------------------------end



   cout << " string: " << sizeof(string)<< endl;;

    // printf("row %d col_num%d\n",oo.row_num,oo.col_num);
    db_mysql.FreeReadData();
    db_mysql.db_disconnect();


    return 0;
}










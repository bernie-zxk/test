#ifndef  _CREATE_SQL_INSERT_H
#define  _CREATE_SQL_INSERT_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
//#include <oci.h>
#include <unistd.h>
//dm
#include <dcidefine.h>
#include <dcisg.h>
//kingbase

#define MAX_FIELD_LEN  128
#define MAX_VALUE_LEN  256

#define DCI_SQL_FAIL        -1 
#define DCI_SQL_SUCCEED     0 


using namespace std;
typedef struct FIELD_S
{
       char   field_name[MAX_FIELD_LEN];
	char   field_value[MAX_VALUE_LEN];
	bool   ifok;
}FIELD_S;




class DCI_SQL_TOOL
{
	protected:
	
        CDci cdv1; 

		
	public:
		DCI_SQL_TOOL();
		~DCI_SQL_TOOL();

			
		bool dbconnect(char *srv_name,char *user_name,char *password);
		bool dbdisconnect();

		int   create_insert(char * tbname, char* outputfile);
};



#endif

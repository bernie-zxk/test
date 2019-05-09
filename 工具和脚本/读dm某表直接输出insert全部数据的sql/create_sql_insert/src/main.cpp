#include <create_sql_insert.h>

void ArgCheck(int a_cnt, char **argv);
void print_usage();




int main(int argc,char *argv[])
{

	ArgCheck(argc,argv);

	DCI_SQL_TOOL *sqltool = new DCI_SQL_TOOL();
	int ret = sqltool->dbconnect(argv[3], argv[1], argv[2]);
	if(ret < 0)
		exit(-1);

	ret = sqltool->create_insert(argv[4], argv[5]);
	if(ret < 0)
		exit(-1);


       exit(0);
}

void ArgCheck(int a_cnt, char **argv)   //未完成
{
	if (a_cnt!=6)          //../create_sql_insert   d5000  d5000  mdb  testtb  testdb.sql
	{
		print_usage();
		exit(1);
	}
	return;
}



void print_usage()   //未完成
{
	cout<<"usage: ./create_sql_insert  username  password  dbservername  tablename  outputfilename "<<endl;
	cout<<"for example:  ./create_sql_insert  d5000  password  mdb  testtable  /home/d5000/testtable.sql "<<endl;

	return;
}
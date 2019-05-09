#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

struct Mstrv
{
	float svalue;
	string smlstr;
};

int cmp(const void *a, const void *B)
{
	//return (*(Mstrv *)a)->svalue < (*(Mstrv *)B)->svalue ? 1:-1;
	return (*(Mstrv *)a).svalue < (*(Mstrv *)B).svalue ? 1:-1;
}

int main()
{
	int i;
	string x="xx";
	cout<<x<<endl;
	Mstrv Ms[5]={
		           {1.2,"a"},
		           {0.2,"b"},
		           {1.0,"c"},
		           {1.9,"d"},
		           {0.1,"e"}
	            };
	cout<<"before qsort"<<endl;
    for(i=0;i<5;i++)
	{
		cout<<Ms[i].smlstr<<"  "<<Ms[i].svalue<<endl;
	}
	cout<<endl;

	qsort(Ms,5,sizeof(Mstrv),cmp);

	cout<<"after qsort"<<endl;
    for(i=0;i<5;i++)
	{
		cout<<Ms[i].smlstr<<"  "<<Ms[i].svalue<<endl;
	}
	cout<<endl;

	exit(0);
}

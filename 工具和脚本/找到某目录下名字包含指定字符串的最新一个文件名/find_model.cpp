/*
    �ҵ�!~/data/cimdata/ �µ����� ����_*.CIME,  scp��ָ���ڵ�Ŀ¼��
 
    �÷��磺
    ./find_model ccs1-pp01:data/cim
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

int main(int argc,char *argv[])
{
    string cmd;

    cmd="ls -lt ~/data/cimdata | grep '����_' | grep '.CIME' | awk '{print($8)}' > cimemodellist.txt";
    system(cmd.c_str());

    cout<<"shell  ��  "<<cmd<<endl;

    fstream infile("cimemodellist.txt");
    if (!infile)
    {
        cout<<"open cimemodellist.txt failed"<<endl;
        return -1;
    }

    string mname;

    getline(infile,mname);

    cmd.clear();
    cmd = "scp -rp ~/data/cimdata/" + mname + " " +argv[1];

    system(cmd.c_str());
    cout<<"ִ��  "<<cmd<<endl;

    exit(0);
}

#include "h_file_dmail.h"
/*
   �������Ĺ���
*/
H_FILE::H_FILE(string fname, string sendaddr, vector<string> recvaddr, string fcontext,string mailtype)
{   
    trans_fname = fname; //Ҫ������ļ���   
    send_addr = sendaddr;  //���͵�ַ
    for (int i=0; i<recvaddr.size(); i++)
    {
        recv_addr.push_back(recvaddr[i]);//���ܵ�ַ
    }
    f_context=fcontext;     //����(�硰����ң����)
    trans_type=mailtype;   //��������

    f_tail = get_tailstr(trans_fname);  
    time_str = get_timestr(trans_fname);   //��ǰʱ��ʱ��
	f_dv = get_dvstr(trans_fname);  //���������
	h_fname = get_hfilename(trans_fname);     //ͷ�ļ���
} 

/*
  �޲�������
*/
H_FILE::H_FILE()
{
	trans_type="�ļ�";   //��������
}
H_FILE::~H_FILE()
{
}
/*
  ���ò��������޲������캯�����ʹ��
*/
int H_FILE::set_info(string fname, string sendaddr, vector<string> recvaddr, string fcontext,string mailtype)
{
	trans_fname = fname; //Ҫ������ļ���   
    send_addr = sendaddr;  //���͵�ַ
    for (int i=0; i<recvaddr.size(); i++)
    {
        recv_addr.push_back(recvaddr[i]);//���ܵ�ַ
    }
    f_context=fcontext;     //����(�硰����ң����)
    trans_type=mailtype;   //��������,�确�ļ���

    f_tail = get_tailstr(trans_fname);  
    time_str = get_timestr(trans_fname);   //��ǰʱ��ʱ��
	f_dv = get_dvstr(trans_fname);  //���������
	h_fname = get_hfilename(trans_fname);     //ͷ�ļ���

	return 1;
}
/*
  ����ͷ�ļ�
*/
int H_FILE::creat_hfile(string &path)
{
	string pathname = path+"/"+h_fname;
	ofstream ofile(pathname.c_str());
	if (!ofile)
	{
		cout<<"can not open file to write "<<pathname<<endl;
		return -1;
	}

	time_t now;
    time(&now);
    char str[512];
    strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    //<�������ƴ�������::����˵�� time='2011-10-08 16:09:17'>
	ofile<<"<�������ƴ�������::����˵�� time='"<<str<<"'>"<<endl;
	//@#˳�� ��Ŀ ����
	ofile<<"@#˳�� ��Ŀ ����"<<endl;
	//#0 ��ʶ �Ĵ�_20111008_094510_220kV_analog.eh
	ofile<<"#0 ��ʶ "<<h_fname<<endl;
	//#1 ���͵�ַ ����1.�Զ���
	ofile<<"#1 ���͵�ַ "<<send_addr<<endl;
	//#2 ���յ�ַ �Ĵ�1.�Զ���
	string addrecv;
	for (vector<string>::size_type i=0; i<recv_addr.size(); i++)
	{
		addrecv+=recv_addr[i]+" ";
		//sstm << recv_addr[i];
		//sstm << " ";
	}
	//string recvss;
	//sstm >> recvss;
	ofile<<"#2 ���յ�ַ "<<addrecv<<endl;
	//#3 �������� �ļ�
	ofile<<"#3 �������� "<<trans_type<<endl;
	//#4 ���� 'ң��ȫ���'
	ofile<<"#4 ���� '"<<f_context<<"'"<<endl;
	//#5 �ļ� '�Ĵ�_20111008_094510_220kV.analog'
	ofile<<"#5 �ļ� '"<<trans_fname<<"'"<<endl;
	//</�������ƴ�������::����˵��>
	ofile<<"</�������ƴ�������::����˵��>"<<endl;

	ofile.close();
	return 1;
}
/*
    �����ļ���������ļ���׺���� CIME  analog��
*/
string H_FILE::get_tailstr(string &tfile)
{
    string str_tail;
    string::size_type pos = tfile.find(".");

    str_tail.assign(tfile,pos+1,tfile.size()-pos-1);

	return str_tail;
}
/*
  ��֯ͷ�ļ���
*/
string H_FILE::get_hfilename(string &tfile)
{
	string hname=tfile;
	string::size_type pos = hname.find(".");
    hname.erase(pos,hname.size()-pos);
	hname += "_" + f_tail + ".eh";

	return hname;
}
/*
    ��֯ʱ��
*/
string H_FILE::get_timestr(string &tfile)
{
	string timestr;
	string::size_type pos = tfile.find("_");
	timestr.assign(tfile,pos+1,tfile.size()-pos-1);

	pos = timestr.find(".");
	timestr.erase(pos,timestr.size()-pos);

	return timestr;
}

/*
    �����ļ��������������
*/
string H_FILE::get_dvstr(string &tfile)
{
	string dvget;
	string::size_type pos = tfile.find("_");
	dvget.assign(tfile,0,pos);

	return dvget;
}

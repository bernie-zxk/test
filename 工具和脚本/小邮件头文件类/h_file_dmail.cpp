#include "h_file_dmail.h"
/*
   传参数的构造
*/
H_FILE::H_FILE(string fname, string sendaddr, vector<string> recvaddr, string fcontext,string mailtype)
{   
    trans_fname = fname; //要传输的文件名   
    send_addr = sendaddr;  //发送地址
    for (int i=0; i<recvaddr.size(); i++)
    {
        recv_addr.push_back(recvaddr[i]);//接受地址
    }
    f_context=fcontext;     //内容(如“增量遥测点表”)
    trans_type=mailtype;   //传输类型

    f_tail = get_tailstr(trans_fname);  
    time_str = get_timestr(trans_fname);   //当前时间时标
	f_dv = get_dvstr(trans_fname);  //获得区域名
	h_fname = get_hfilename(trans_fname);     //头文件名
} 

/*
  无参数构造
*/
H_FILE::H_FILE()
{
	trans_type="文件";   //传输类型
}
H_FILE::~H_FILE()
{
}
/*
  设置参数，与无参数构造函数配合使用
*/
int H_FILE::set_info(string fname, string sendaddr, vector<string> recvaddr, string fcontext,string mailtype)
{
	trans_fname = fname; //要传输的文件名   
    send_addr = sendaddr;  //发送地址
    for (int i=0; i<recvaddr.size(); i++)
    {
        recv_addr.push_back(recvaddr[i]);//接受地址
    }
    f_context=fcontext;     //内容(如“增量遥测点表”)
    trans_type=mailtype;   //传输类型,如‘文件’

    f_tail = get_tailstr(trans_fname);  
    time_str = get_timestr(trans_fname);   //当前时间时标
	f_dv = get_dvstr(trans_fname);  //获得区域名
	h_fname = get_hfilename(trans_fname);     //头文件名

	return 1;
}
/*
  生成头文件
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
    //<生产控制大区互联::传输说明 time='2011-10-08 16:09:17'>
	ofile<<"<生产控制大区互联::传输说明 time='"<<str<<"'>"<<endl;
	//@#顺序 项目 内容
	ofile<<"@#顺序 项目 内容"<<endl;
	//#0 标识 四川_20111008_094510_220kV_analog.eh
	ofile<<"#0 标识 "<<h_fname<<endl;
	//#1 发送地址 华中1.自动化
	ofile<<"#1 发送地址 "<<send_addr<<endl;
	//#2 接收地址 四川1.自动化
	string addrecv;
	for (vector<string>::size_type i=0; i<recv_addr.size(); i++)
	{
		addrecv+=recv_addr[i]+" ";
		//sstm << recv_addr[i];
		//sstm << " ";
	}
	//string recvss;
	//sstm >> recvss;
	ofile<<"#2 接收地址 "<<addrecv<<endl;
	//#3 传输类型 文件
	ofile<<"#3 传输类型 "<<trans_type<<endl;
	//#4 内容 '遥测全点表'
	ofile<<"#4 内容 '"<<f_context<<"'"<<endl;
	//#5 文件 '四川_20111008_094510_220kV.analog'
	ofile<<"#5 文件 '"<<trans_fname<<"'"<<endl;
	//</生产控制大区互联::传输说明>
	ofile<<"</生产控制大区互联::传输说明>"<<endl;

	ofile.close();
	return 1;
}
/*
    根据文件名，获得文件后缀（如 CIME  analog）
*/
string H_FILE::get_tailstr(string &tfile)
{
    string str_tail;
    string::size_type pos = tfile.find(".");

    str_tail.assign(tfile,pos+1,tfile.size()-pos-1);

	return str_tail;
}
/*
  组织头文件名
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
    组织时标
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
    根据文件名，获得区域名
*/
string H_FILE::get_dvstr(string &tfile)
{
	string dvget;
	string::size_type pos = tfile.find("_");
	dvget.assign(tfile,0,pos);

	return dvget;
}

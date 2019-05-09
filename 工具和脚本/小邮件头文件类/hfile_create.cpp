/*
 	./hfile_create -f �ļ��� -saddr ���͵�ַ -raddr ���յ�ַ1+���յ�ַ2+...  [-text �ļ����ͣ��硰��־����]
 	���У�-text ��ʡ�ԡ�
 	���յ�ַ�ɶ������Ҫ�á�+���ָ��������пո�
*/
#include "h_file_dmail.h"

string g_fname;
string g_saddr;
string g_text;
vector<string> g_vraddr;

void print_usage()
{
	cout<<"usage: ./hfile_create -f �ļ��� -saddr ���͵�ַ -raddr ���յ�ַ1+���յ�ַ2+...  [-text �ļ����ͣ��硰��־����]"<<endl;
	cout<<"���У�-text ��ʡ��"<<endl;
	cout<<"      -text ʡ��ʱ��ȱʡΪ'�����ļ�'"<<endl;
	cout<<"      ���յ�ַ�ɶ������Ҫ�á�+���ָ��������пո�"<<endl;
	cout<<"      �ļ�����ʽ  ����Ϊ ����׺ �� 'xxx_1234.txt'"<<endl;
	cout<<"      xxx_1234.txt  ���γ��ļ����� xxx_1234_txt.eh"<<endl;
}
/*
    ��ý��յ�ַ�����ܶ����
*/
void get_raddrs(char *raddrs)
{
	string tmpstr=raddrs;
	string sing_raddr;
	string::size_type pos=0;
	const string::size_type spos=0;
	if ((pos= tmpstr.find("+"))==string::npos)
	{
		g_vraddr.push_back(tmpstr);
	}
	else
	{
		while ((pos= tmpstr.find("+"))!=string::npos)
		{
			sing_raddr.clear();
			sing_raddr.assign(tmpstr,spos,pos-spos);
			g_vraddr.push_back(sing_raddr);
			tmpstr.erase(spos,pos-spos+1);
		}
        g_vraddr.push_back(tmpstr);//���һ��
	}
	return;
}
/*
    ���ͻ�ȡ����
*/
void get_par(int argc, char *argv[])
{
	if (argc!=7 && argc!=9)
	{
		print_usage();
		exit(1);
	}
	if (argc==7)
	{
		if (strcmp(argv[1],"-f")!=0 || strcmp(argv[3],"-saddr")!=0 || strcmp(argv[5],"-raddr")!=0)
		{
			print_usage();
			exit(1);
		}
		g_text = "�ļ�";
	}
	else if (argc==9)
	{
		if (strcmp(argv[1],"-f")!=0 || strcmp(argv[3],"-saddr")!=0 || 
			 strcmp(argv[5],"-raddr")!=0 || strcmp(argv[7],"-text")!=0)
		{
			print_usage();
			exit(1);
		}
		g_text = argv[8];
	}

	string::size_type pos;
	g_fname=argv[2];
	if ((pos = g_fname.find("."))==string::npos)
	{
		print_usage();
		exit(1);
	}
    g_saddr=argv[4];

	get_raddrs(argv[6]);
	
	return;
}

int main(int argc, char *argv[])
{
	//���ͻ�ȡ����
	get_par(argc, argv);

	cout <<"g_fname: "<<g_fname<<endl;
	cout <<"g_saddr: "<<g_saddr<<endl;
	cout <<"g_text: "<<g_text<<endl;
	for (vector<string>::size_type vi = 0; vi < g_vraddr.size(); vi++)
	{
		cout <<"g_vraddr: "<<g_vraddr[vi]<<endl;
	}
	H_FILE Hfile(g_fname,g_saddr,g_vraddr,g_text);
    string pth=".";
	int ret = Hfile.creat_hfile(pth);
	if (ret == -1)
	{
		cout<<"failed to create hfile"<<endl;
	}

	exit(0);
}

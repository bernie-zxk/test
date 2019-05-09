#include "h_file_dmail.h"

int main(int argc, char *argv[])
{
	if (argc!=2)
	{
		printf("input file!\n");
		exit(1);
	}
	//H_FILE::H_FILE(string fname, string sendaddr, vector<string> recvaddr, string fcontext)
	vector<string> recvaddr;
	recvaddr.push_back("湖南");
	recvaddr.push_back("湖南2");

	H_FILE Hfile(argv[1],"华中",recvaddr,"模型");
    
	cout<<"Hfile.f_dv: "<<Hfile.f_dv<<endl;
	cout<<"Hfile.f_tail: "<<Hfile.f_tail<<endl;
	cout<<"Hfile.time_str: "<<Hfile.time_str<<endl;
	cout<<"Hfile.h_fname: "<<Hfile.h_fname<<endl;

	cout<<"Hfile.trans_fname: "<<Hfile.trans_fname<<endl;
	cout<<"Hfile.send_addr: "<<Hfile.send_addr<<endl;

    cout<<"Hfile.f_context: "<<Hfile.f_context<<endl;
	cout<<"Hfile.trans_type: "<<Hfile.trans_type<<endl;

	for (int i=0;i<Hfile.recv_addr.size();i++)
	{
		cout<<"Hfile.recv_addr: "<<Hfile.recv_addr[i]<<endl;
	}

	string pth=".";
	int ret = Hfile.creat_hfile(pth);
	if (ret == -1)
	{
		cout<<"failed to create hfile"<<endl;
	}

	exit(0);
}

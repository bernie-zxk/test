#include "ftp_client.h"

int main(int argc, char *argv[])
{
    string serverIP = "192.168.200.228";
    string userName = "bernie";
    string password = "urge.rover!pass";
    string GetRemoteFile = "kkk.cpp";
    string GetLocalFile = "/home/d5000/kd/src/platform/personal_test/zhaoxiaokai/test/get.cpp";
    string PutRemoteFile = "put.cpp";
    string PutLocalFile = "/home/d5000/kd/src/platform/personal_test/zhaoxiaokai/test/gggg.cpp";
	string dir;
    CFTPManager MYFTP;
    MYFTP.login2Server(serverIP);
    MYFTP.inputUserName(userName);
    MYFTP.inputPassWord(password);
    //MYFTP.CreateDirectory("test2018");
    MYFTP.Put(PutRemoteFile,PutLocalFile);
    
    MYFTP.Get(GetRemoteFile,GetLocalFile);
	dir = MYFTP.Dir("/tmp/zhao/bernie/");
    //cout << "::" << dir.c_str() << endl;
    MYFTP.quitServer();


}


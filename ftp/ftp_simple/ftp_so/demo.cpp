#include "ftp_client.h"
#include "ZPrecTime.h"

int main(int argc, char *argv[])
{
    string serverIP = "192.168.200.228";
    string userName = "bernie";
    string password = "urge.rover!pass";
    string GetRemoteFile = "kkk.cpp";
    string GetLocalFile = "/home/d5000/kd/src/platform/personal_test/zhaoxiaokai/test/get.cpp";
    string PutRemoteFile = "test.txt";
    string PutLocalFile = "/home/d5000/kd/src/platform/personal_test/zhaoxiaokai/test/demo/test.txt";
	string dir;
    CFTPManager MYFTP;
   // MYFTP.type mode = MYFTP.binary;

	
		{
		ZPrecTime aaa("   login ºÄÊ±: ");	
    MYFTP.login2Server(serverIP);
    MYFTP.inputUserName(userName);
    MYFTP.inputPassWord(password);
    	}
	MYFTP.setTransferMode(0x31);
    //MYFTP.CreateDirectory("test2018");
    {
    	ZPrecTime aaa("    MYFTP.Put1 ºÄÊ±: ");
 
		  MYFTP.Put("test1.txt",PutLocalFile); 
		  MYFTP.Put("test2.txt",PutLocalFile);
		  MYFTP.Put("test3.txt",PutLocalFile);
		  MYFTP.Put("test4.txt",PutLocalFile);
		  MYFTP.Put("test5.txt",PutLocalFile);
		  MYFTP.Put("test6.txt",PutLocalFile);
		  MYFTP.Put("test7.txt",PutLocalFile);
		  MYFTP.Put("test8.txt",PutLocalFile);
		  MYFTP.Put("test9.txt",PutLocalFile);
		  MYFTP.Put("test10.txt",PutLocalFile);
		  
		
        
    }
    //MYFTP.Get(GetRemoteFile,GetLocalFile);
	//dir = MYFTP.Dir("/tmp/zhao/bernie/");
    //cout << "::" << dir.c_str() << endl;
    MYFTP.quitServer();
    

}


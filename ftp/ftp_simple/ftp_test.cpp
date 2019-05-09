#include <unistd.h>   
#include <stdio.h>   
#include "ftp_client.h"   
   
#define HOST_ADDRESS "192.168.200.227"   
#define USER_NAME "test"   
#define USER_PWD "verse:jail.equal"   
#define GET_FILE_NAME "/home/d5000/kd/src/platform/personal_test/zhaoxiaokai/test/zhao/get.h"   
#define GET_FILE_PATH "/tmp/test/hello.txt"   
#define PUT_FILE_NAME "/home/d5000/kd/src/platform/personal_test/zhaoxiaokai/test/zhao/hello228.txt"   
#define PUT_FILE_PATH "/tmp/test/put.txt"   
   
int main(void)   
{   
    AFC_FTP afc_ftp;   
    int nRet = 0;   
    nRet = afc_ftp.FtpConnect(HOST_ADDRESS,USER_NAME,USER_PWD);   
    if(nRet == FTP_CONNECT_SUCCESS){   
        printf("Connecting successful!\n");   
    }   
    else if(nRet == FTP_HOST_NAME_ERROR){   
        printf("Host name error!\n");   
        return 0;   
    }   
    else{   
        printf("User name or password error!!\n");   
        return 0;   
    }   
       
    if(afc_ftp.FtpGet(GET_FILE_NAME, GET_FILE_PATH, FTPLIB_BINARY)){   
        printf("Getting file successful!\n");   
    }   
    else{   
        
        printf("Getting file error!\n");   
    }   
   
    if(afc_ftp.FtpPut(PUT_FILE_NAME, PUT_FILE_PATH, FTPLIB_BINARY)){   
        printf("Putting file successful!\n");   
    }   
    else{   
        printf("Putting file error!\n");   
    }   
   
    return 0;   
}   


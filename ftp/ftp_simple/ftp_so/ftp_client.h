//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// [基本信息] 
// 文件名			:afc_ftplib.h 
// 创建日期         :2006-02-08 
// 设计者           :  
// 编码者           :lu lin 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// [描述]	FTP客户端类。提供外部接口，与服务器通讯； 
// [备注] 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// [修改记录]  
//================================================================= 
//  序号          修改人             修改原因 
//------------------+--------------------------+--------------------------- 
 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
 
 
#if !defined(AFC__FTPLIB_H) 
#define FTP_CLIENT_H 
 
/* FtpAccess() type codes */ 
#define FTPLIB_DIR 1 
#define FTPLIB_DIR_VERBOSE 2 
#define FTPLIB_FILE_READ 3 
#define FTPLIB_FILE_WRITE 4 
 
/* FtpAccess() mode codes */ 
#define FTPLIB_ASCII 'A' 
#define FTPLIB_IMAGE 'I' 
#define FTPLIB_TEXT FTPLIB_ASCII 
#define FTPLIB_BINARY FTPLIB_IMAGE 
 
#define FTPLIB_BUFSIZ 8192 
#define FTPLIB_RESPONSE 256 
 
#define FTP_CONNECT_SUCCESS 1  
#define FTP_HOST_NAME_ERROR 2 
#define FTP_USER_PWS_ERROR  3 
 
 
class AFC_FTP 
{ 
	struct NetBuf { 
    char *cput,*cget; 
    int handle; 
    int cavail,cleft; 
    char buf[FTPLIB_BUFSIZ]; 
    int dir; 
    char response[FTPLIB_RESPONSE]; 
	}; 
 
public: 
	AFC_FTP(); 
	~AFC_FTP(); 
public: 
	int FtpConnect(const char *host,const char *user, const char *pass); //连接到远程服务器
	int FtpGet(const char *output, const char *path, char mode); //发出GET命令并将收到的数据写入输出
	int FtpPut(const char *input, const char *path, char mode); //发出PUT命令并从输入发送数据
   
private: 
	void InitNetConnect(NetBuf* p); // init net_control 
	int FtpConnectHost(const char *host); //使用主机名或IP地址连接到远程服务器
	int FtpLoginHost(const char *user, const char *pass); //登录到远程服务器
    void FtpQuit(); //断开远程	
 
	int FtpAccess(const char *path, int typ, int mode, NetBuf *nControl,NetBuf *nData); //返回数据流的句柄
	int FtpRead(void *buf, int max, NetBuf *nData); //从数据连接中读取
	int FtpWrite(void *buf, int len, NetBuf *nData); //写入数据连接
	int FtpClose(NetBuf *nData); //关闭数据连接
	int ReadLine(char *buf,int max,NetBuf *ctl); //读一段文字
	int WriteLine(char *buf, int len, NetBuf *nData); //写一行文本
	int ReadResp(char c, NetBuf *nControl); //从服务器读取响应
	int FtpSendCmd(const char *cmd, char expresp, NetBuf *nControl); //发送一个命令并等待预期响应
	int FtpOpenPort(NetBuf *nControl, NetBuf *nData, int mode, int dir); //建立数据连接
	int FtpXfer(const char *localfile, const char *path, NetBuf *nControl, int typ, int mode); //发出命令和传输数据
 
private: 
	int ftplib_debug; 
 
	NetBuf net_control; 
	char net_buf[FTPLIB_BUFSIZ]; 
}; 
 
 
#endif /* AFC__FTPLIB_H */ 




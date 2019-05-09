//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// [������Ϣ] 
// �ļ���			:afc_ftplib.h 
// ��������         :2006-02-08 
// �����           :  
// ������           :lu lin 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// [����]	FTP�ͻ����ࡣ�ṩ�ⲿ�ӿڣ��������ͨѶ�� 
// [��ע] 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// [�޸ļ�¼]  
//================================================================= 
//  ���          �޸���             �޸�ԭ�� 
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
	int FtpConnect(const char *host,const char *user, const char *pass); //���ӵ�Զ�̷�����
	int FtpGet(const char *output, const char *path, char mode); //����GET������յ�������д�����
	int FtpPut(const char *input, const char *path, char mode); //����PUT��������뷢������
   
private: 
	void InitNetConnect(NetBuf* p); // init net_control 
	int FtpConnectHost(const char *host); //ʹ����������IP��ַ���ӵ�Զ�̷�����
	int FtpLoginHost(const char *user, const char *pass); //��¼��Զ�̷�����
    void FtpQuit(); //�Ͽ�Զ��	
 
	int FtpAccess(const char *path, int typ, int mode, NetBuf *nControl,NetBuf *nData); //�����������ľ��
	int FtpRead(void *buf, int max, NetBuf *nData); //�����������ж�ȡ
	int FtpWrite(void *buf, int len, NetBuf *nData); //д����������
	int FtpClose(NetBuf *nData); //�ر���������
	int ReadLine(char *buf,int max,NetBuf *ctl); //��һ������
	int WriteLine(char *buf, int len, NetBuf *nData); //дһ���ı�
	int ReadResp(char c, NetBuf *nControl); //�ӷ�������ȡ��Ӧ
	int FtpSendCmd(const char *cmd, char expresp, NetBuf *nControl); //����һ������ȴ�Ԥ����Ӧ
	int FtpOpenPort(NetBuf *nControl, NetBuf *nData, int mode, int dir); //������������
	int FtpXfer(const char *localfile, const char *path, NetBuf *nControl, int typ, int mode); //��������ʹ�������
 
private: 
	int ftplib_debug; 
 
	NetBuf net_control; 
	char net_buf[FTPLIB_BUFSIZ]; 
}; 
 
 
#endif /* AFC__FTPLIB_H */ 




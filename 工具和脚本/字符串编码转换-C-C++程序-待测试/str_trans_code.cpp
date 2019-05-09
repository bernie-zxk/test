//iconv_linux���ַ�������ת������ʵ��
//(1) iconv_t iconv_open(const char *tocode, const char *fromcode);
//�˺���˵����Ҫ���������ֱ����ת��,tocode��Ŀ�����,fromcode��ԭ����,�ú�������һ��ת�����,��������������ʹ�á�
]
//(2) size_t iconv(iconv_t cd,char **inbuf,size_t *inbytesleft,char **outbuf,size_t *outbytesleft);
//�˺�����inbuf�ж�ȡ�ַ�,ת���������outbuf��,inbytesleft���Լ�¼��δת�����ַ���,outbytesleft���Լ�¼��������ʣ��ռ䡣

//(3) int iconv_close(iconv_t cd);
//�˺������ڹر�ת�����,�ͷ���Դ��
 
//����1: ��C����ʵ�ֵ�ת��ʾ������
 
/* f.c : ����ת��ʾ��C���� */
#include <iconv.h>
#define OUTLEN 255
main()
{
char *in_utf8 = "正�?��??�?";
char *in_gb2312 = "���ڰ�װ";
char out[OUTLEN];
 
/*unicode��תΪgb2312��*/
rc = u2g(in_utf8,strlen(in_utf8),out,OUTLEN);
printf("unicode-->gb2312 out=%sn",out);

//gb2312��תΪunicode��
rc = g2u(in_gb2312,strlen(in_gb2312),out,OUTLEN);
printf("gb2312-->unicode out=%sn",out);


}
/*����ת��:��һ�ֱ���תΪ��һ�ֱ���*/
int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
iconv_t cd;
int rc;
char **pin = &inbuf;
char **pout = &outbuf;
 
cd = iconv_open(to_charset,from_charset);
if (cd==0) return -1;
memset(outbuf,0,outlen);
if (iconv(cd,pin,&inlen,pout,&outlen)==-1) return -1;
iconv_close(cd);
return 0;
}

/*UNICODE��תΪGB2312��*/
int u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}

/*GB2312��תΪUNICODE��*/
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}
 
//����2: ��C++����ʵ�ֵ�ת��ʾ������
 
/* f.cpp : ����ת��ʾ��C++���� */
#include <iconv.h>
#include <iostream>
 
#define OUTLEN 255
 
using namespace std;
 
// ����ת��������
 
class CodeConverter {
 
private:
iconv_t cd;
public:
 
// ����
CodeConverter(const char *from_charset,const char *to_charset) {
cd = iconv_open(to_charset,from_charset);
}
 
 
// ����
~CodeConverter() {
iconv_close(cd);
}
 
 
// ת�����
int convert(char *inbuf,int inlen,char *outbuf,int outlen) {
char **pin = &inbuf;
char **pout = &outbuf;
 
memset(outbuf,0,outlen);
return iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen);
}
};
 
int main(int argc, char **argv)
{
char *in_utf8 = "正�?��??�?";
char *in_gb2312 = "���ڰ�װ";
char out[OUTLEN];
 
// utf-8-->gb2312
CodeConverter cc = CodeConverter("utf-8","gb2312");
cc.convert(in_utf8,strlen(in_utf8),out,OUTLEN);
cout << "utf-8-->gb2312 in=" << in_utf8 << ",out=" << out << endl;
 
// gb2312-->utf-8
CodeConverter cc2 = CodeConverter("gb2312","utf-8");
cc2.convert(in_gb2312,strlen(in_gb2312),out,OUTLEN);
cout << "gb2312-->utf-8 in=" << in_gb2312 << ",out=" << out << endl;
}

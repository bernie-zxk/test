

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

/*
Ӣ���ַ�,ʹ��ASCII,��һ��byte��ʾ���ɣ�ASCII��һ���涨��128���ַ��ı��룬����ո�SPACE����32��������00100000����
��д����ĸA��65��������01000001������128�����ţ�����32�����ܴ�ӡ�����Ŀ��Ʒ��ţ���ֻռ����һ���ֽڵĺ���7λ����ǰ���1λͳһ�涨Ϊ0��
������������һ��byte����ʾ���Բ������������ĺ��ֱ��룬�ǲ�������byte����ʾ��ͬʱΪ����ԭ��asciiӢ���ַ���������
�ر�涨:
     ��������127���ַ�����һ��ʱ���ͱ�ʾһ�����֣�ǰ���һ���ֽڣ�����֮Ϊ���ֽڣ���0xA1�õ� 0xF7������һ���ֽڣ����ֽڣ���0xA1��0xFE��
	 �������ǾͿ�����ϳ���Լ7000������庺���ˡ�
	 ����Щ��������ǻ�����ѧ���š�����ϣ������ĸ�����ĵļ����Ƕ����ȥ�ˣ�
	 ���� ASCII �ﱾ�����е����֡���㡢��ĸ��ͳͳ���±��������ֽڳ��ı��룬����ǳ�˵��"ȫ��"�ַ�����ԭ����127�����µ���Щ�ͽ�"���"�ַ��ˡ� ��
	 ��������ΪGB2312����.
	 һ���Ϊ�����룮
	 
	 
	 ������˵����
	     ���ַ���ͷ��β����¼�ж�һ���ַ����� ���뺺�֡���һ�ֽڣ��ĸ��������жϸ�����ż������������������һ�����뺺�֡���������ַ��� ���������� 
*/
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#define MAX_STR_LEN 8 
//#define TEST_STR "aһ������" 

#define MAX_STR_LEN 128 
#define TEST_STR "һ�����������߰˾�ʮһ�����������߰˾�ʮһ�����������߰˾�ʮһ�����������߰˾�ʮһ�����������߰˾�ʮһ�����������߰˾�ʮaһ��������" 



//�ж��ַ����� ���ֵ��ֽ�  �� ��������ż��
int chkHalfChinese(char * buf,int len)
{
    int i = 0;
    int cnt = 0;
    int idx;
    unsigned char ckch='0';
    
    for(i=0;i<len;i++)
     {
         unsigned char ckch = (unsigned char)buf[i];
         if(ckch > 0xa1)   
         {
         	if(ckch == 0xa3)
         	    ;//ȫ���ַ�
			else
			{    //���� 
				cnt++;   
           		idx=i;	
			} 
         }
     }
     
     
    if(cnt%2)
     {
     //����idx�Ǽ�¼���һ��ascii�������ĵ��ַ������cnt����������˵�����ַ��Ƕ���ģ����丳ֵΪ���ַ���
        buf[idx] =' ';
        //return(1);
        return(idx);
     }
    else
        return(0);
} 

//����ַ����а뺺�ֵĸ����������������һ���뺺��֮��Ĳ���ȥ�����ý������滻  
int del_last_half_cn(char * inbuf,int inlen, char *outbuf, int outlen)
{
     printf("5 check error str: %s \n\n",inbuf);
	printf("size %d ,sizeinbuf=%d ,sizeofinbuf=%d before deal. output: %s\n",MAX_STR_LEN,strlen(inbuf),sizeof(inbuf),inbuf);
	int pos = chkHalfChinese(inbuf,MAX_STR_LEN);
    // int ret = chkHalfChinese(prtstr,strlen(prtstr));//  ���Է��� strlen(prtstr) =9 
    if(pos!=0)
    {
     	memset(outbuf,0,sizeof(outbuf));
     	strncpy(outbuf,inbuf,pos);
     	//strncpy(outbuf,inbuf,pos-1);
     	return 1;
     	
     }

     return 0;
}



int main(int argc, char** argv) {
	
	char teststr[MAX_STR_LEN*2];
	memset(teststr,0,sizeof(teststr));
	sprintf(teststr,"%s\0",TEST_STR);
	printf("test: %s \n\n",teststr);
	
	
	char *pt = teststr+MAX_STR_LEN;
	memset(pt,0,MAX_STR_LEN);    //��������������� 
	
	printf("create error str: %s \n\n",teststr);
 
     char  errstr[MAX_STR_LEN];   
     strncpy(errstr,teststr,MAX_STR_LEN);   //���һ�����ֱ��ض� 
     
     printf("1 check error str: %s \n\n",errstr);
     
	 char finalstr[MAX_STR_LEN];
//     printf("2 check error str: %s \n\n",errstr);
	 memset(finalstr,0,MAX_STR_LEN);
  //   printf("3 check error str: %s \n\n",errstr);
	 
//	 printf("size %d ,sizeinbuf=%d ,sizeofinbuf=%d before del_last_half_cn. output: %s \n",MAX_STR_LEN,strlen(errstr),sizeof(errstr),errstr);
 //     printf("4 check error str: %s \n\n",errstr);
    int ret = del_last_half_cn(errstr,MAX_STR_LEN,finalstr,MAX_STR_LEN); 
     if(ret == 0)
     {
     	printf("string %s doesn't' have to deal",errstr);
     }
     else if(ret == 1)
     {
     	printf("size %d  after deal. output: %s\n",MAX_STR_LEN,finalstr);
     }

     exit(0);

	
}

/* 
bool isCN(char *str)
{
      bool fret;
	  //һ������ռ�������ֽڣ�ÿ���ֽڵ����λ��Ϊ1��   
      if(strlen(str) <= 1) 
		   fret= false;
	  else
	  {
          if((str[0]&0x80)&&(str[1]&0x80))
		  {
             
	         fret=true;
		  }
	      else
		    fret=false;
	  }
      return fret;
}

int del_last_half_cn(char * buf,int len)
{
	bool bret=false;
    int i=0;
    int cnt=0;
	 
    while(i<len-1)//
    {
    	bret = isCN(&(buf[i]));
    	if(bret)
    	{
    		i=i+2;
    		cnt++;
    	}
    	else
    	{
    		i++;
    	}
    }
     return cnt;
}
*/



/* run this program using the console pauser or add your own getch, system("pause") or input loop */

/*
英文字符,使用ASCII,用一个byte表示即可．ASCII码一共规定了128个字符的编码，比如空格“SPACE”是32（二进制00100000），
大写的字母A是65（二进制01000001）。这128个符号（包括32个不能打印出来的控制符号），只占用了一个字节的后面7位，最前面的1位统一规定为0。
　　　汉字用一个byte来表示明显不够，因此最早的汉字编码，是采用两个byte来表示．同时为了与原来ascii英文字符不混淆．
特别规定:
     两个大于127的字符连在一起时，就表示一个汉字，前面的一个字节（他称之为高字节）从0xA1用到 0xF7，后面一个字节（低字节）从0xA1到0xFE，
	 这样我们就可以组合出大约7000多个简体汉字了。
	 在这些编码里，我们还把数学符号、罗马希腊的字母、日文的假名们都编进去了，
	 连在 ASCII 里本来就有的数字、标点、字母都统统重新编了两个字节长的编码，这就是常说的"全角"字符，而原来在127号以下的那些就叫"半角"字符了。 这
	 个方案称为GB2312编码.
	 一般称为国标码．
	 
	 
	 本程序说明：
	     从字符串头到尾，记录判断一个字符串中 “半汉字”（一字节）的个数，并判断个数奇偶。如果是奇数，则将最后一个“半汉字”及后面的字符用 结束符代替 
*/
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#define MAX_STR_LEN 8 
//#define TEST_STR "a一二三四" 

#define MAX_STR_LEN 128 
#define TEST_STR "一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十a一二三四五" 



//判断字符串中 汉字单字节  的 个数的奇偶。
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
         	    ;//全角字符
			else
			{    //汉字 
				cnt++;   
           		idx=i;	
			} 
         }
     }
     
     
    if(cnt%2)
     {
     //其中idx是记录最后一个ascii码是中文的字符，如果cnt是奇数，则说明该字符是多余的，将其赋值为空字符。
        buf[idx] =' ';
        //return(1);
        return(idx);
     }
    else
        return(0);
} 

//如果字符串中半汉字的个数是奇数，则最后一个半汉字之后的部分去掉，用结束符替换  
int del_last_half_cn(char * inbuf,int inlen, char *outbuf, int outlen)
{
     printf("5 check error str: %s \n\n",inbuf);
	printf("size %d ,sizeinbuf=%d ,sizeofinbuf=%d before deal. output: %s\n",MAX_STR_LEN,strlen(inbuf),sizeof(inbuf),inbuf);
	int pos = chkHalfChinese(inbuf,MAX_STR_LEN);
    // int ret = chkHalfChinese(prtstr,strlen(prtstr));//  测试发现 strlen(prtstr) =9 
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
	memset(pt,0,MAX_STR_LEN);    //产生半个汉字乱码 
	
	printf("create error str: %s \n\n",teststr);
 
     char  errstr[MAX_STR_LEN];   
     strncpy(errstr,teststr,MAX_STR_LEN);   //最后一个汉字被截断 
     
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
	  //一个汉字占用两个字节，每个字节的最高位必为1。   
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

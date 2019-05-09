#include "Similar_str.h"

Similar_str::Similar_str()
{
	v_size.clear();
#ifdef STR_GBK2312
   //string ch_cn="א";  //gb2312  CN size 2 
	CN_LEN=2;
#else
   //string ch_cn="中";  //utf-8  CN size 3"
    CN_LEN=3; 
#endif
//   CN_LEN=ch_cn.size();
}
#if 0
Similar_str::Similar_str(string stra, string strb)
{
   assert(!stra.empty());
   assert(!strb.empty());
   str_a=stra;
   str_b=strb;

   string ch_cn="中";
   CN_LEN=ch_cn.size();

}
#endif

Similar_str::~Similar_str()
{
}


//�������ƶ�

float Similar_str::calc_similar()
{
    int i;
    float max=0.0;
    for (i=0;i<v_size.size();i++)
    {
        max+=v_size[i]*v_size[i];
    }
    
    return (float)max/(float)(str_a.size()*str_b.size());
}

//�ҵ�����ƥ����ַ���������ƥ�䳤�ȣ���������str_b��λ��
int  Similar_str::find_map(string part_a, int& b_pos)
{
	int map_len=0,pos;
	string sb;
    string elmt_a,elmt_b;
	int sa_pos=0,sb_pos=0,lppos=0;
	string tmpsa,tmpsb,lpsa,lpsb;
	bool iscn=false;

    //sb.assign(str_b,str_b.begin()+b_pos,str_b.end()); 
    sb.assign(str_b,b_pos,str_b.size()-b_pos); 
    
	elmt_a.clear();
    //��һ���ַ��Ƿ��Ǻ��֣�����ȡCN_LEN�ֽڣ�����ȥ1�ֽڬ�
    if(isCN(part_a))
    {
		if(b_pos+CN_LEN>str_b.size())
			return 0;
        //elmt_a.assign(part_a,part_a.begin(),part_a.begin()+CN_LEN);
        elmt_a.assign(part_a,0,CN_LEN);
        tmpsa.assign(part_a,CN_LEN,part_a.size()-CN_LEN);
        iscn=true;
    }            
    else
    {
		if(b_pos+1>str_b.size())
			return 0;
        elmt_a.push_back(part_a[0]);
        //tmpsa.assign(part_a,part_a.begin()+1,part_a.end());
        tmpsa.assign(part_a,1,part_a.size()-1);
		iscn=false;
    }
    //elmt_a��str_b�в���
    if((pos=sb.find(elmt_a))==string::npos)//�Ҳ���
    {
        return 0;
    }
    else//�ҵ�
    {
		if(iscn)
		{
			sb_pos=pos+CN_LEN;//pos��sb���ҵ�elmt_a��λ��
			b_pos+=pos+CN_LEN;
			map_len+=CN_LEN;//ƥ�䳤��CN_LEN
		}
		else
		{
			sb_pos=pos+1;
			b_pos+=pos+1;
			map_len++;
		}

		if(sb_pos<sb.size())
		     tmpsb.assign(sb,sb_pos,sb.size()-sb_pos);
		     //tmpsb.assign(sb,sb.begin()+sb_pos,sb.end());
		else
			return map_len;

	    iscn=false;	
		//������
		while(sb_pos<sb.size())
		{
			 if(tmpsa.empty())
				 break;
             if(isCN(tmpsa))
             {
				 if (sb_pos+CN_LEN > sb.size())//��b��ʣ��Ĳ����Ƚ���
					 break;
                 elmt_a.assign(tmpsa,0,CN_LEN);
				 elmt_b.assign(tmpsb,0,CN_LEN);
                 lpsa.assign(tmpsa,CN_LEN,tmpsa.size()-CN_LEN);
                 lpsb.assign(tmpsb,CN_LEN,tmpsb.size()-CN_LEN);
				 tmpsa=lpsa;
				 tmpsb=lpsb;
                 iscn=true;
             }            
             else
             {
				 //if (sb_pos+1 >= sb.size())//��b��ʣ��Ĳ����Ƚ���
				 if (sb_pos+1 > sb.size())//��b��ʣ��Ĳ����Ƚ���
					 break;
                 //elmt_a.push_back(tmpsa[0]);
                 elmt_a=tmpsa[0];
                 //elmt_b.push_back(tmpsb[0]);
                 elmt_b=tmpsb[0];
                 lpsa.assign(tmpsa,1,tmpsa.size()-1);
                 lpsb.assign(tmpsb,1,tmpsb.size()-1);
				 tmpsa=lpsa;
				 tmpsb=lpsb;
		         iscn=false;
             }
    
			 if(elmt_a!=elmt_b)//�Ҳ���
             {
				 return map_len;
			 }
			 else
			 {
                 if(iscn)
				 {
					 map_len+=CN_LEN;
					 sb_pos+=CN_LEN;
					 b_pos+=CN_LEN;
				 }
				 else
				 {
					 map_len++;
					 sb_pos++;
					 b_pos++;
				 }
			 }
		}
    }

	return map_len;
}
//�Ƚ��ַ�����ͳ�Ƹ�ƥ�䲿�ֳ��ȣ��������ƶ�
float Similar_str::get_similar(string stra,string strb)
{
   v_size.clear();//��ֹ��������ʱ��v_size��׷��
   assert(!stra.empty());
   assert(!strb.empty());
   str_a=stra;//str_a  ���Ա����
   str_b=strb;
   //ȫ��ת���  ֻ������ ���� ռ2�ֽ�
   ToDBC_gb(const_cast<char *>(str_a.c_str()),str_a.size());
   ToDBC_gb(const_cast<char *>(str_b.c_str()),str_b.size());

    int i=0,j=0,k=0;
    int max_cnt=0,ret_cnt=0;
    int p_cnt=0;
    int b_pos;
    string part_a;
    string sun_part;
    string rec_map;
    string elmt_a,elmt_b;
  

    while(i<str_a.size())
    {
        part_a.clear();
        //��i��ʼ����str_a������part_a
        part_a.assign(str_a,i,str_a.size()-i); 

		b_pos=0;
        max_cnt=0;
        while(b_pos<str_b.size())
		{
		    //��str_b��b_posλ�ÿ�ʼ����patr_a������ƥ����ַ�����������ƥ��ľͷ���ƥ�䳤�ȣ�
			//��������str_b��λ�á�
            ret_cnt = find_map(part_a,b_pos);//b_pos����Ϊƥ����str_b��λ��
			if(ret_cnt == -1)
			{
				cout<<"find_map error ,part_a: "<<part_a<<endl;
				return -1;
			}
			else if(ret_cnt==0)
			{//û��ƥ���ϵ�
				break;
			}
			else
			{
			    if(max_cnt<ret_cnt)
				    max_cnt=ret_cnt;
			}
		}
	
		if (max_cnt>0)
		{
            v_size.push_back(max_cnt);//part_a�˴αȽϵ��ƥ�䳤�ȴ���
		    //part_a��i��ʼ�����ƥ�䳤��max_cnt�����´δ�part_a��i+max_cnt����ʼ�Ƚ�
		    i+=max_cnt;
		}
		else//û�ҵ�ƥ�䣬��������αȽϵ�part_a��һ���ַ���һ�����֣���������ıȽ�
		{
            if(isCN(part_a))
				i+=CN_LEN;
			else
				i++;
		}
       

     }
	float sml_value=calc_similar();
	return sml_value;
}

//������ַ��Ƿ�������
bool Similar_str::isCN(string str)
{
    bool fret;
	  //һ������ռ��2���ֽڣ�ÿ���ֽڵ����Ϊ��Ϊ1
//    if(strlen(str) <= 1) 
    if(str.empty()) 
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


//only for gb2312  (CN size 2)
//ȫ��ת��ǣ������ں���ռ2�ֽڵ��ַ���
int Similar_str::ToDBC_gb(char *input,int len) 
{ 
	int m=0,i=0;
	unsigned char c1,c2;
	char c,tmp[2];
    char putput[300];
    bzero(putput,300);

	while ( i < strlen(input) )
	{
		if(isCN(&input[i])) //ȫ��
		{
			c1=input[i];
			c2=input[i+1];
			if(c1 == 163)
			{
		        c=(char)(c2-128);
				sprintf(tmp,"%c\0",c);
				strcat(putput,tmp);
				m++;
			}
			else if(c1 == 161 && c2 == 161)
			{
			    c = ' ';
				sprintf(tmp,"%c\0",c);
				strcat(putput,tmp);
				m++;
			}
			else
			{
			    strncat(putput,&input[i],2);
			}
			i=i+2;
		}
		else
		{
		    strncat(putput,&input[i],1);
			i++;
		}
		       
	}
	if(m > 0)
	{
		sprintf(input,"%s\0",putput);
	}
	return m;	 
}





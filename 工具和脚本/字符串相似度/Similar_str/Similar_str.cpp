#include "Similar_str.h"

Similar_str::Similar_str()
{
	v_size.clear();
#ifdef STR_GBK2312
   //string ch_cn="×";  //gb2312  CN size 2 
	CN_LEN=2;
#else
   //string ch_cn="ä¸­";  //utf-8  CN size 3"
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

   string ch_cn="ä¸­";
   CN_LEN=ch_cn.size();

}
#endif

Similar_str::~Similar_str()
{
}


//¼ÆËãÏàËÆ¶È

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

//ÕÒµ½Á¬ĞøÆ¥ÅäµÄ×Ö·û´®£¬·µ»ØÆ¥Åä³¤¶È£¬±äÁ¿·µ»Østr_bµÄÎ»ÖÃ
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
    //µÚÒ»¸ö×Ö·ûÊÇ·ñÊÇºº×Ö£¬ÊÇÔòÈ¡CN_LEN×Ö½Ú£¬·ñÔòÈ¥1×Ö½Ú¬¦
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
    //elmt_aµ½str_bÖĞ²éÕÒ
    if((pos=sb.find(elmt_a))==string::npos)//ÕÒ²»µ½
    {
        return 0;
    }
    else//ÕÒµ½
    {
		if(iscn)
		{
			sb_pos=pos+CN_LEN;//posÊÇsbÖĞÕÒµ½elmt_aµÄÎ»ÖÃ
			b_pos+=pos+CN_LEN;
			map_len+=CN_LEN;//Æ¥Åä³¤¶ÈCN_LEN
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
		//Á¬ĞøÕÒ
		while(sb_pos<sb.size())
		{
			 if(tmpsa.empty())
				 break;
             if(isCN(tmpsa))
             {
				 if (sb_pos+CN_LEN > sb.size())//Èôb´®Ê£ÓàµÄ²»¹»±È½ÏÁË
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
				 //if (sb_pos+1 >= sb.size())//Èôb´®Ê£ÓàµÄ²»¹»±È½ÏÁË
				 if (sb_pos+1 > sb.size())//Èôb´®Ê£ÓàµÄ²»¹»±È½ÏÁË
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
    
			 if(elmt_a!=elmt_b)//ÕÒ²»µ½
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
//±È½Ï×Ö·û´®£¬Í³¼Æ¸÷Æ¥Åä²¿·Ö³¤¶È£¬¼ÆËãÏàËÆ¶È
float Similar_str::get_similar(string stra,string strb)
{
   v_size.clear();//·ÀÖ¹·´¸´µ÷ÓÃÊ±£¬v_sizeµÄ×·¼Ó
   assert(!stra.empty());
   assert(!strb.empty());
   str_a=stra;//str_a  Àà³ÉÔ±±äÁ¿
   str_b=strb;
   //È«½Ç×ª°ë½Ç  Ö»ÊÊÓÃÓÚ ºº×Ö Õ¼2×Ö½Ú
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
        //´Ói¿ªÊ¼£¬½«str_a¿½±´¸øpart_a
        part_a.assign(str_a,i,str_a.size()-i); 

		b_pos=0;
        max_cnt=0;
        while(b_pos<str_b.size())
		{
		    //´Óstr_bµÄb_posÎ»ÖÃ¿ªÊ¼£¬ÓÃpatr_aÕÒÁ¬ĞøÆ¥ÅäµÄ×Ö·û´®£¬Óöµ½²»Æ¥ÅäµÄ¾Í·µ»ØÆ¥Åä³¤¶È£¬
			//±äÁ¿·µ»Østr_bµÄÎ»ÖÃ¡£
            ret_cnt = find_map(part_a,b_pos);//b_pos¸üĞÂÎªÆ¥ÅäºóµÄstr_bµÄÎ»ÖÃ
			if(ret_cnt == -1)
			{
				cout<<"find_map error ,part_a: "<<part_a<<endl;
				return -1;
			}
			else if(ret_cnt==0)
			{//Ã»ÄÜÆ¥ÅäÉÏµÄ
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
            v_size.push_back(max_cnt);//part_a´Ë´Î±È½ÏµÄ×î³¤Æ¥Åä³¤¶È´¢´æ
		    //part_a´Ói¿ªÊ¼£¬×î´óÆ¥Åä³¤¶Èmax_cnt£¬ÔòÏÂ´Î´Ópart_aµÄi+max_cnt³ö¿ªÊ¼±È½Ï
		    i+=max_cnt;
		}
		else//Ã»ÕÒµ½Æ¥Åä£¬ÔòÌø¹ıÕâ´Î±È½ÏµÄpart_aµÄÒ»¸ö×Ö·û»òÒ»¸öºº×Ö£¬¼ÌĞøºóÃæµÄ±È½Ï
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

//¼ì²éÊ××Ö·ûÊÇ·ñÊÇÖĞÎÄ
bool Similar_str::isCN(string str)
{
    bool fret;
	  //Ò»¸öºº×ÖÕ¼ÓÃ2¸ö×Ö½Ú£¬Ã¿¸ö×Ö½ÚµÄ×î¸ßÎª±ÈÎª1
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
//È«½Ç×ª°ë½Ç£¬ÊÊÓÃÓÚºº×ÖÕ¼2×Ö½ÚµÄ×Ö·û¼¯
int Similar_str::ToDBC_gb(char *input,int len) 
{ 
	int m=0,i=0;
	unsigned char c1,c2;
	char c,tmp[2];
    char putput[300];
    bzero(putput,300);

	while ( i < strlen(input) )
	{
		if(isCN(&input[i])) //È«½Ç
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





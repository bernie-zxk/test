
#include "Similar_str.h"

main()
{
	int ret,i; 


    string stra,strb;
    cout<<"input the first string: "<<endl;
    cin>>stra;

    cout<<"input the second string: "<<endl;
    cin>>strb;

    Similar_str *Sms = new Similar_str();

    float svalue = Sms->get_similar(stra,strb);

	cout<<"比较的字符串:  "<<endl;
	cout<<stra<<"  length: "<<stra.size()<<endl;
	cout<<strb<<"  length: "<<strb.size()<<endl;


	cout<<" 匹配字符串 "<<Sms->v_size.size()<<" 部分，各部分长度分别是："<<endl;
	for(i=0;i<Sms->v_size.size();i++)
	{
		cout<<Sms->v_size[i]<<" ";
	}
	cout<<endl;

    cout<<"相似度值:  "<<svalue<<endl;
   

    exit(0);
    
}

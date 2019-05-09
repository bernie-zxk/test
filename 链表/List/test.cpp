#include <stdio.h>
#include <stdlib.h>
#include "List.h"
int main(int argc,char **argv)
{
    DoubleList list;
    int num = 0;
    linklist *Head = NULL;
    Head = list.create();
    list.insert(Head,50);
    list.insert(Head,40);
    list.insert(Head,30);
    list.insert(Head,20);
    list.insert(Head,10);
    list.insert(Head,5);
   // num = list.getlength(Head);
  //  printf("num = %d\n",num);
    list.show(Head);
    list.mydelete(Head,2);
    list.mydelete(Head,2);
    
   // num = list.getlength(Head);
   // printf("num = %d\n",num);
    list.show(Head);

    return 0;

}

#include <stdio.h>
#include <stdlib.h>
#include "List.h"
int main(int argc,char **argv)
{
    DoubleList list;
    int num = 0;
    list.create();
    list.insert(50);
    list.insert(40);
    list.insert(30);
    list.insert(20);
    list.insert(10);
    list.insert(5);
    num = list.getlength();
    printf("num = %d\n",num);
    list.show();
    list.mydelete(2);
    list.mydelete(2);
    
    num = list.getlength();
    printf("num = %d\n",num);
    list.show();

    return 0;

}

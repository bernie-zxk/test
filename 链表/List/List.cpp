#include "List.h"


linklist* SilgleList::create()
{
    linklist * H = NULL;
    if((H =(linklist *)malloc(sizeof(struct node))) == NULL)
    {
        printf("malloc failed\n");
        return NULL;
    }
    H->next = NULL;

    return H;
}

int SilgleList::insert(linklist* H,datatype value)
{
    linklist * p = NULL;
    if((p = (linklist*)malloc(sizeof(linklist))) == NULL)
    {
        printf("malloc node failed\n");
        return -1;
    }

    p->data = value;

    p->next = H->next;
    H->next = p;

    return 0;
}

int SilgleList::mydelete(linklist * H,int pos)
{
    int i = -1;
    linklist* p;
    while(i < pos)
    {
        if(H->next == NULL)
        {
            printf("list is empty!\n");
            return -1;
        }
        i++;
        H = H->next;
    }
    p = H->next;
    H->next = p->next;

    free(p);

    return 0;
}


int SilgleList::getlength (linklist *H)
{
    int n=0;
    linklist *q = H;
    while (q->next!=NULL)
    {
        n++;
        q=q->next;
    }
    return(n);
}

void SilgleList::show(linklist *H)
{
    while(H->next != NULL)
    {
        printf("%d ",H->next->data);
        H = H->next;
    }
    putchar(10);
}


//DoubleList
linklist* DoubleList::create()
{
    linklist * H = NULL;
    if((H =(linklist *)malloc(sizeof(linklist))) == NULL)
    {
        printf("malloc failed\n");
        return NULL;
    }

    H->data = 0;
    H->next = NULL;
    H->prior = NULL;

    return H;
}

int DoubleList::insert(linklist *H,datatype value)
{
    linklist *q;
    while(H->next != NULL)
    {
        H = H->next;
    }
    q = (linklist*)malloc(sizeof(linklist));
    if(q == NULL)
    {
        printf("malloc p failed\n");
        return -1;
    }
    q->data = value;
    q->next = H->next;
    q->prior = H;
    H->next = q;
    //q->next->prior = q;


    return 0;
}

int DoubleList::mydelete(linklist * H,int pos)
{
    int i = 0;
    linklist* q;
    q = H->next;
    while(i < pos && q->next != NULL )
    {
        q = q->next;
        i++;
    }

    q->prior->next = q->next;
    q->next->prior = q->prior;
    free(q);

    return 0;
}

int DoubleList::getlength (linklist *H)
{
    int length = 0;
    linklist *q;
    q = H;

    while (q->next != NULL)
    {
        length++;
        q = q->next;
    }

    return  length;
}

void DoubleList::show(linklist *H)
{
    //linklist * p = NULL;
    printf("Ë«Á´±íÊý¾Ý£º\n");
    H = H->next;
    while(H != NULL)
    {
        printf("%d\n",H->data);
        H = H->next;
    }

}


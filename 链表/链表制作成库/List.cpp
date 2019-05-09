#include "List.h"

SingleList::~SingleList()
{
    linklist* q;
    while(Head)
    {
        q = Head;
        Head = Head->next;
        free(q);
    }
}

void SingleList::create()
{
	if((Head =(linklist *)malloc(sizeof(struct node))) == NULL)
	{
		printf("malloc failed\n");
		return;
	}
	Head->next = NULL;
	
}

int SingleList::insert(datatype value)
{
	linklist *p = NULL,*q = NULL;
    q = Head;

    while(q->next != NULL)
    {
        q = q->next;
    }
	if((p = (linklist*)malloc(sizeof(linklist))) == NULL)
	{
		printf("malloc node failed\n");
		return -1;
	}

	p->data = value;

	p->next = q->next;
	q->next = p;

	return 0;
}

int SingleList::mydelete(int pos)
{
    int i = 1;
    linklist *q,*p;
    q = Head;
    while(i < pos)
    {
        if(Head->next == NULL)
        {
            printf("list is empty!\n");
            return -1;
        }
        i++;
        Head = Head->next;
    }
    
    p = Head->next;
	Head->next = p->next;

	free(p);
	Head = q;
	return 0;
}


int SingleList::getlength ()
{
	int n=0;
 	linklist *q = Head;
 	while (q->next!=NULL)
 	{
 		n++;
  		q=q->next;
 	}
 	return(n);
}

void SingleList::show()
{
    linklist* temp;
    temp = Head;
	while(Head->next != NULL)
	{
		printf("%d ",Head->next->data);
		Head = Head->next;
	}
    Head = temp;
	putchar(10);
}


//DoubleList
DoubleList::~DoubleList()
{
    linklist1* q;
    while(Head)
    {
        q = Head;
        Head = Head->next;
        free(q);
    }
}

void DoubleList::create()
{
	if((Head =(linklist1 *)malloc(sizeof(linklist1))) == NULL)
	{
		printf("malloc failed\n");
		return;
	}

	Head->data = 0;
	Head->next = NULL;
    Head->prior = NULL;
	
	return;
}

int DoubleList::insert(datatype value)
{
	linklist1 *p,*q;
    p = Head;
    while(p->next != NULL)
    {
        p = p->next;
    }
    q = (linklist1*)malloc(sizeof(linklist1));
    if(q == NULL)
	{
		printf("malloc q failed\n");
		return -1;
	}
    q->data = value;
    q->next = p->next;
    q->prior = p;
    p->next = q;
    
	return 0;
}

int DoubleList::mydelete(int pos)
{
    int i = 1;
    linklist1* q;
    q = Head->next;
    while(i < pos && q->next != NULL )
    {
        if(q->next == NULL)
        {
            printf("list is empty!");
            return -1;
        }
        q = q->next;
        i++;
    }
 
    q->prior->next = q->next;
    q->next->prior = q->prior;
	free(q);

	return 0;
}

int DoubleList::getlength ()
{
	int length = 0;
 	linklist1 *q;
    q = Head;
    
 	while (q->next != NULL)
 	{
        length++;
  		q = q->next;
 	}

 	return  length;
}

void DoubleList::show()
{
    linklist1* p;
    p = Head;
    printf("Ë«Á´±íÊý¾Ý£º\n");
    p = p->next;
    while(p != NULL)
    {
        printf("%d\n",p->data);
        p = p->next;
    }

}


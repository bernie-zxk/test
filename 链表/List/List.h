#ifndef _LIST_H_
#define _LIST_H_

#include <stdio.h>
#include <stdlib.h>

typedef int datatype;

struct node{
	datatype data;
	struct node *prior;
	struct node *next;
};
typedef struct node linklist;

class List 
{
    public:
		List(){};
		virtual ~List(){};
		virtual linklist *create() = 0;
		virtual int insert(linklist *H,int value) = 0;
		virtual int mydelete(linklist *H,int pos) = 0;
		virtual int getlength(linklist *H) = 0;
        virtual void show(linklist *H) = 0;
};

class SilgleList:public List
{
	public:
		SilgleList(){};
		~SilgleList(){};
		virtual linklist *create();
		virtual int insert(linklist *H,int value);
		virtual int mydelete(linklist *H,int pos);
		virtual int getlength(linklist *H);
        virtual void show(linklist *H);

};

class DoubleList:public List
{

	public:
		DoubleList(){};
		~DoubleList(){};
		virtual linklist *create();
		virtual int insert(linklist *H,int value);
		virtual int mydelete(linklist *H,int pos);
		virtual int getlength(linklist *H);
        virtual void show(linklist *H);

};

#endif

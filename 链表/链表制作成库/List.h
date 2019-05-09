#ifndef _LIST_H_
#define _LIST_H_

#include <stdio.h>
#include <stdlib.h>

typedef int datatype;

struct node{
	datatype data;
	struct node *next;
};
typedef struct node linklist;

struct node1{
	datatype data;
	struct node1 *prior;
	struct node1 *next;
};
typedef struct node1 linklist1;

class List 
{
    public:
		List(){};
		virtual ~List(){};
		virtual void create() = 0;
		virtual int insert(int value) = 0;
		virtual int mydelete(int pos) = 0;
		virtual int getlength() = 0;
        virtual void show() = 0;
};




class SingleList:public List
{
	public:
		SingleList(){};
		~SingleList();
		virtual void create();
		virtual int insert(int value);
		virtual int mydelete(int pos);
		virtual int getlength();
        virtual void show();
    private:
		linklist* Head;

};

class DoubleList: List
{

	public:
		DoubleList(){};
		~DoubleList();
		virtual void create();
		virtual int insert(int value);
		virtual int mydelete(int pos);
		virtual int getlength();
        virtual void show();
   private:
   	    linklist1* Head;
   	
       

};

#endif



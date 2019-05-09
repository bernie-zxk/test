#ifndef dmlst_h
#define dmlst_h

/***********************************************************************
This macro expands to the unnamed type definition of a struct which acts
as the two-way list base node. The base node contains pointers
to both ends of the list and a count of nodes in the list (excluding
the base node from the count). TYPE should be the list node type name. */

#define DM_LIST_BASE_NODE_T(TYPE)\
struct {\
	int	count;	/* count of nodes in list */\
	TYPE *	start;	/* pointer to list start, NULL if empty */\
	TYPE *	end;	/* pointer to list end, NULL if empty */\
}\

/***********************************************************************
This macro expands to the unnamed type definition of a struct which
should be embedded in the nodes of the list, the node type must be a struct.
This struct contains the pointers to next and previous nodes in the list.
The name of the field in the node struct should be the name given
to the list. TYPE should be the list node type name. Example of usage:

typedef struct LRU_node_struct	LRU_node_t;
struct LRU_node_struct {
	DM_LIST_NODE_T(LRU_node_t)	LRU_list;
	...
}
The example implements an LRU list of name LRU_list. Its nodes are of type
LRU_node_t.
*/

#define DM_LIST_NODE_T(TYPE)\
struct {\
	TYPE *	prev;	/* pointer to the previous node,\
			NULL if start of list */\
	TYPE *	next;	/* pointer to next node, NULL if end of list */\
}\

/***********************************************************************
Initializes the base node of a two-way list. */

#define DM_LIST_INIT(BASE)\
do{\
	(BASE).count = 0;\
	(BASE).start = NULL;\
	(BASE).end   = NULL;\
}while(0)

/***********************************************************************
Initializes the node of a two-way list. */
#define DM_LIST_NODE_INIT(NAME, N)\
do{\
	((N)->NAME).prev = NULL;\
	((N)->NAME).next = NULL;\
}while(0)

/***********************************************************************
Adds the node as the first element in a two-way linked list.
BASE has to be the base node (not a pointer to it). N has to be
the pointer to the node to be added to the list. NAME is the list name. */

#define DM_LIST_ADD_FIRST(NAME, BASE, N)\
do{\
	((BASE).count)++;\
	((N)->NAME).next = (BASE).start;\
	((N)->NAME).prev = NULL;\
	if ((BASE).start != NULL) {\
		(((BASE).start)->NAME).prev = (N);\
	}\
	(BASE).start = (N);\
	if ((BASE).end == NULL) {\
		(BASE).end = (N);\
	}\
}while(0)

/***********************************************************************
Adds the node as the last element in a two-way linked list.
BASE has to be the base node (not a pointer to it). N has to be
the pointer to the node to be added to the list. NAME is the list name. */

#define DM_LIST_ADD_LAST(NAME, BASE, N)\
do{\
	((BASE).count)++;\
	((N)->NAME).prev = (BASE).end;\
	((N)->NAME).next = NULL;\
	if ((BASE).end != NULL) {\
		(((BASE).end)->NAME).next = (N);\
	}\
	(BASE).end = (N);\
	if ((BASE).start == NULL) {\
		(BASE).start = (N);\
	}\
}while(0)


/***********************************************************************
Adds the node as the last element in a two-way linked list.
BASE has to be the base node (not a pointer to it). N has to be
the pointer to the node to be added to the list. NAME is the list name. */

#define DM_LIST_CON_LAST(NAME, BASE, BASE2)\
do{\
    if ((BASE).count == 0)\
    {\
        (BASE).count = (BASE2).count;\
        (BASE).start = (BASE2).start;\
        (BASE).end   = (BASE2).end;\
    }\
    else if ((BASE2).count == 0)\
    {}\
    else\
    {\
        ((BASE).count) += ((BASE2).count);\
        (((BASE).end)->NAME).next = (BASE2).start;\
        (((BASE2).start)->NAME).prev = (BASE).end;\
        (BASE).end = (BASE2).end;\
    }\
}while(0)

/***********************************************************************
Inserts a NODE2 before NODE1 in a list.
BASE has to be the base node (not a pointer to it). NAME is the list
name, NODE1 and NODE2 are pointers to nodes. */

#define DM_LIST_INSERT_BEFORE(NAME, BASE, NODE1, NODE2)\
do{\
	((BASE).count)++;\
	((NODE2)->NAME).next = (NODE1);\
	((NODE2)->NAME).prev = ((NODE1)->NAME).prev;\
	if (((NODE1)->NAME).prev != NULL) {\
		((((NODE1)->NAME).prev)->NAME).next = (NODE2);\
	}\
	((NODE1)->NAME).prev = (NODE2);\
	if ((BASE).start == (NODE1)) {\
		(BASE).start = (NODE2);\
	}\
}while(0)

/***********************************************************************
Inserts a NODE2 after NODE1 in a list.
BASE has to be the base node (not a pointer to it). NAME is the list
name, NODE1 and NODE2 are pointers to nodes. */

#define DM_LIST_INSERT_AFTER(NAME, BASE, NODE1, NODE2)\
do{\
	((BASE).count)++;\
	((NODE2)->NAME).prev = (NODE1);\
	((NODE2)->NAME).next = ((NODE1)->NAME).next;\
	if (((NODE1)->NAME).next != NULL) {\
		((((NODE1)->NAME).next)->NAME).prev = (NODE2);\
	}\
	((NODE1)->NAME).next = (NODE2);\
	if ((BASE).end == (NODE1)) {\
		(BASE).end = (NODE2);\
	}\
}while(0)

/***********************************************************************
Removes a node from a two-way linked list. BASE has to be the base node
(not a pointer to it). N has to be the pointer to the node to be removed
from the list. NAME is the list name. */

#define DM_LIST_REMOVE(NAME, BASE, N)\
do{\
	((BASE).count)--;\
	if (((N)->NAME).next != NULL) {\
		((((N)->NAME).next)->NAME).prev = ((N)->NAME).prev;\
	} else {\
		(BASE).end = ((N)->NAME).prev;\
	}\
	if (((N)->NAME).prev != NULL) {\
		((((N)->NAME).prev)->NAME).next = ((N)->NAME).next;\
	} else {\
		(BASE).start = ((N)->NAME).next;\
	}\
	((N)->NAME).next = NULL;\
	((N)->NAME).prev = NULL;\
}while(0)

/************************************************************************
Gets the next node in a two-way list. NAME is the name of the list
and N is pointer to a node. */

#define DM_LIST_GET_NEXT(NAME, N)\
	(((N)->NAME).next)

/************************************************************************
Gets the previous node in a two-way list. NAME is the name of the list
and N is pointer to a node. */

#define DM_LIST_GET_PREV(NAME, N)\
	(((N)->NAME).prev)

/************************************************************************
Alternative macro to get the number of nodes in a two-way list, i.e.,
its length. BASE is the base node (not a pointer to it). */

#define DM_LIST_GET_LEN(BASE)\
	(BASE).count

/************************************************************************
Gets the first node in a two-way list, or returns NULL,
if the list is empty. BASE is the base node (not a pointer to it). */

#define DM_LIST_GET_FIRST(BASE)\
	(BASE).start

/************************************************************************
Gets the last node in a two-way list, or returns NULL,
if the list is empty. BASE is the base node (not a pointer to it). */

#define DM_LIST_GET_LAST(BASE)\
	(BASE).end

/************************************************************************
Checks the consistency of a two-way list. NAME is the name of the list,
TYPE is the node type, and BASE is the base node (not a pointer to it). */

#ifdef _DEBUG
#define DM_LIST_VALIDATE(NAME, TYPE, BASE)\
do{\
	int	ut_list_i_313;\
	TYPE *	ut_list_node_313;\
\
	ut_list_node_313 = (BASE).start;\
\
	for (ut_list_i_313 = 0; ut_list_i_313 < (BASE).count;\
	 					ut_list_i_313++) {\
	 	ut_list_node_313 = (ut_list_node_313->NAME).next;\
	}\
\
\
	ut_list_node_313 = (BASE).end;\
\
	for (ut_list_i_313 = 0; ut_list_i_313 < (BASE).count;\
	 					ut_list_i_313++) {\
	 	ut_list_node_313 = (ut_list_node_313->NAME).prev;\
	}\
\
}while(0)
	
#else
#define DM_LIST_VALIDATE(NAME, TYPE, BASE)
#endif

#endif


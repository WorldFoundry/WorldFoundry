//============================================================================
// _List.h: double linked list handling for HAL
//============================================================================
// use only once insurance

#ifndef __lIST_H
#define __lIST_H

//============================================================================
// Documentation:
//
//
// 	Abstract:
// 			Intrusive doubly-linked list library
// 	History:
// 			Created	10-24-94 06:26pm Kevin T. Seghetti
//
// 	Class Hierarchy:
// 			none
//
// 	Dependancies:
// 			none
//
// 	Restrictions:
// 		Nodes cannot be in more than one list at a time
// 	Example:
//
//
//============================================================================
// dependencies

#include <hal/halbase.h>

//============================================================================

#define NODEINLIST(node) (node->_prev)

//============================================================================
// debugging macros

#if DO_ASSERTIONS
#define VALIDATENODE(node) \
 {\
	VALIDATEPTR(node); \
 }

// done this way so that head & tail nodes pass the test
#define VALIDATENODEINLIST(node) \
 {\
	VALIDATENODE(node); \
	if(!(node)->_next) assert((node)->_prev); \
	if(!(node)->_prev) assert((node)->_next); \
 }

#define VALIDATENODENOTINLIST(node) \
 {\
	VALIDATENODE(node); \
	assert((node)->_prev == NULL); \
	assert((node)->_next == NULL); \
 }
#else
#define VALIDATENODE(node)
#define VALIDATENODEINLIST(node)
#define VALIDATENODENOTINLIST(node)
#endif

//============================================================================

typedef struct _SNode					// base class, not usefull unless derived from
{
	struct _SNode* _next;
	struct _SNode* _prev;
	short _priority;					// node priority, used by prioritized list functions
} SNode;

//-----------------------------------------------------------------------------
// node function prototypes

void
NodeConstruct(SNode* self);

// kts remove
SNode*
NodeNew(void);

// if in list, removes self from list, and generates a warning
void
NodeDestruct(SNode* self);

SNode*
NodeDelete(SNode* self);					// returns NULL

SNode*
NodePrev(SNode* self);						// returns NULL if no previous nodes

SNode*
NodeNext(SNode* self);						// returns NULL if no more nodes

void
NodeInsert(SNode* self,SNode* prev);		// insert this node after previous node

SNode*
NodeRemove(SNode* self);					// remove this node from list(note: MUST be in a list

short
NodeGetPriority(SNode* self);

// set priority of node in list.  WARNING: doesn't work if node not in list
// only works if list is already in priority order
void
NodeSetPriority(SNode* self,short priority);

//=============================================================================

typedef struct List
{
	SNode* _head;			// special overlapping head & tail Nodes
	SNode* _zero;			// we should discuss, should we do this
	SNode* _tail;			// to save 4 bytes per list instance?
} SList;

//-----------------------------------------------------------------------------

#if DO_ASSERTIONS
#define VALIDATELIST(list) \
	VALIDATEPTR(list); \
	VALIDATEPTR((list)->_head); \
	VALIDATEPTR((list)->_tail); \
	assert((list)->_zero == NULL)
#else
#define VALIDATELIST(list)
#endif

//----------------------------------------------------------------------------
// list function prototypes

void
ListConstruct(SList* self);

SList*
ListNew(void);

void
ListDestruct(SList* self);

SList*								// this returns NULL so you can say foo = ListDelete(foo);
ListDelete(SList* self);

void
ListAddHead(SList* self, SNode* newNode);

void
ListAddTail(SList* self, SNode* newNode);

// gets return node pointer, but do not remove node from list
SNode*
ListGetHead(SList* self);				// note: returns NULL if list empty

SNode*
ListGetTail(SList* self);                // note: returns NULL if list empty

SNode*
ListRemoveHead(SList* self);				// note: returns NULL if list empty

SNode*
ListRemoveTail(SList* self);                // note: returns NULL if list empty

void
ListEnqueue(SList* self, SNode* newNode, short pri);	// prioritized insertion

bool
ListEmpty(SList* self);			// if list empty, returns true

SList*
NodeGetList(SNode* self);					// get address of list this node is in

//=============================================================================

#if TEST_LIST
void
ListTest(void);
#endif

//=============================================================================
#endif
//=============================================================================

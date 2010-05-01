//=============================================================================
// _List.c: double linked list handling
//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//			Intrusive doubly-linked list library
//	History:
//			Created	10-24-94 06:26pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:

//	Restrictions:
//		Nodes cannot be in more than one list at a time
//	Example:
//=============================================================================
// dependencies

#define __lIST_C
#include <hal/_list.h>
#include <cpplib/stdstrm.hp>
//#include <cstdio>

//-----------------------------------------------------------------------------
// node funcions

void
NodeConstruct(SNode* self)
{
	VALIDATEPTR(self);
	self->_next = NULL;				// must always do this, so that NodeDestruct can detect whether the node is in a list or not
	self->_prev = NULL;
	self->_priority = 0;
}

//=============================================================================

#if TEST_LIST
SNode*
NodeNew(void)
{
	SNode* self;
	self = new (HALLmalloc) SNode;
	assert( self );
	AssertMemoryAllocation(self);
	NodeConstruct(self);
	return(self);
}
#endif

//=============================================================================
// if in list, removes self from list, and generates a warning

void
NodeDestruct(SNode* self)
{
	VALIDATENODE(self);
	if(NODEINLIST(self))
	 {
		DBSTREAM2(cprogress << "WARNING: node in list destructed\n");
		NodeRemove(self);
	 }
}

//=============================================================================

#if TEST_LIST
SNode*
NodeDelete(SNode* self)
{
	VALIDATENODE(self);
	NodeDestruct(self);
	HALLmalloc.Free(self);
	return(NULL);
}
#endif

//=============================================================================

SNode*
NodePrev(SNode* self)
{
	VALIDATENODEINLIST(self);
	if(self->_prev && self->_prev->_prev)
		return(self->_prev);
	else
		return(NULL);
}

//=============================================================================

SNode*
NodeNext(SNode* self)
{
	VALIDATENODEINLIST(self);
	if(self->_next && self->_next->_next)
		return(self->_next);
	else
		return(NULL);
}

//=============================================================================

void
NodeInsert(SNode* self,SNode* newNode)		// insert newNode after this
{
	VALIDATENODENOTINLIST(newNode);
	VALIDATENODEINLIST(self);
	assert(newNode != self);				// insure not adding to self

	newNode->_prev = self;
	newNode->_next = self->_next;

	newNode->_next->_prev = newNode;
	self->_next = newNode;
}

//=============================================================================

SNode*
NodeRemove(SNode* self)
{
	VALIDATENODE(self);

	self->_prev->_next = self->_next;			// unlink from list
	self->_next->_prev = self->_prev;
	self->_next = NULL;
	self->_prev = NULL;

	return(self);
}

//=============================================================================

short
NodeGetPriority(SNode* self)
{
	VALIDATEPTR(self);					// could be in list or not
	return(self->_priority);
}

//=============================================================================
// set priority of node in list.  WARNING: doesn't work if not in list

void
NodeSetPriority(SNode* self,short priority)
{
	SList* parent;
	VALIDATENODEINLIST(self);
	if(priority != self->_priority)
	 {
//		self->_priority = priority;
		parent = NodeGetList(self);
		VALIDATELIST(parent);

		NodeRemove(self);
		ListEnqueue(parent,self,priority);				// place at correct priority
	 }
}

//=============================================================================

SList*
NodeGetList(SNode* self)
{
	SNode* temp;
	VALIDATENODE(self);

	temp = self;
	while(temp->_prev)
	 {
		VALIDATENODEINLIST(temp);
		temp = temp->_prev;
	 }

	VALIDATELIST((SList*)temp);
	// temp now points to head, which coincides with the address of the list
	return((SList*)temp);
}

//=============================================================================
// list code
//=============================================================================
// does tail point to head?
// if list empty, returns true

#define LISTEMPTY(list) \
	(list->_tail == (SNode*)list)

//=============================================================================

void
ListConstruct(SList* self)
{
	VALIDATEPTR(self);
	self->_head = (SNode*)&self->_zero;
	self->_zero = NULL;
	self->_tail = (SNode*)&self->_head;
}

//=============================================================================

SList*
ListNew(void)
{
	SList* self;
	self = new (HALLmalloc) SList;
	assert( self );
	AssertMemoryAllocation(self);
	ListConstruct(self);
	return(self);
}

//=============================================================================

void
ListDestruct(SList* self)
{
	(void)self;
	VALIDATELIST(self);
	assert(ListEmpty(self));
#if DO_ASSERTIONS
	self->_head = NULL;
	self->_zero = NULL;
	self->_tail = NULL;
#endif
}

//=============================================================================

SList*
ListDelete(SList* self)
{
	VALIDATELIST(self);
	ListDestruct(self);
	HALLmalloc.Free(self);
	return(NULL);
}

//=============================================================================

void
ListAddHead(SList* self, SNode* newNode)
{
	VALIDATELIST(self);
	NodeInsert((SNode*)&self->_head,newNode);
}

//=============================================================================

void
ListAddTail(SList* self, SNode* newNode)
{
	VALIDATELIST(self);
	NodeInsert(self->_tail,newNode);
}

//=============================================================================

SNode*
ListRemoveHead(SList* self)				// note: returns NULL if list empty
{
	VALIDATELIST(self);

	if(LISTEMPTY(self))					// check if list is empty
		return(NULL);
	return(NodeRemove(self->_head));
}

//=============================================================================

SNode*
ListGetHead(SList* self)				// note: returns NULL if list empty
{
	VALIDATELIST(self);

	if(LISTEMPTY(self))					// check if list is empty
		return(NULL);
	return(self->_head);
}

//=============================================================================

void
ListEnqueue(SList* self, SNode* newNode,short priority)	// prioritized insertion
{
	SNode* temp;

	VALIDATELIST(self);

	temp = self->_head;
	newNode->_priority = priority;

	while((temp != (SNode*)&self->_zero) && temp->_priority >= priority)
		temp = temp->_next;

	// temp = node to insert before, so pass in prev since NodeInsert inserts after
	NodeInsert(temp->_prev,newNode);
}

//=============================================================================

SNode*
ListRemoveTail(SList* self)
{
	VALIDATELIST(self);
	if(LISTEMPTY(self))
		return(NULL);
	return(NodeRemove(self->_tail));
}

//=============================================================================

SNode*
ListGetTail(SList* self)
{
	VALIDATELIST(self);
	if(LISTEMPTY(self))
		return(NULL);
	return(self->_tail);
}

//=============================================================================

bool
ListEmpty(SList* list)
{
	return(LISTEMPTY(list));
}

//=============================================================================
// test suite

#if TEST_LIST

void
ListTest(void)
{
	SList *l1;
	SNode *n1,*n2,*n3,*n4,*tempNode;

	n1 = NodeNew();
	VALIDATENODENOTINLIST(n1);
	n2 = NodeNew();
	n3 = NodeNew();
	n4 = NodeNew();
	l1 = ListNew();
	VALIDATELIST(l1);

	ListAddHead(l1,n1);
	assert(!ListEmpty(l1));			// insure ListEmpty works when list not empty
	ListAddHead(l1,n2);
	ListAddHead(l1,n3);
	assert(!ListEmpty(l1));
	tempNode = ListGetHead(l1);		// test ListGetHead & NodeNext
	assert(tempNode == n3);
	tempNode = NodeNext(tempNode);
	assert(tempNode == n2);
	tempNode = NodeNext(tempNode);
	assert(tempNode == n1);
	tempNode = NodeNext(tempNode);
	assert(tempNode == NULL);

	tempNode = ListGetTail(l1);		// test ListGetTail & NodePrev
	assert(tempNode == n1);
	tempNode = NodePrev(tempNode);
	assert(tempNode == n2);
	tempNode = NodePrev(tempNode);
	assert(tempNode == n3);
	tempNode = NodePrev(tempNode);
	assert(tempNode == NULL);

	assert(NodeGetList(n2) == l1);

	tempNode = ListRemoveHead(l1);
	assert(tempNode == n3);
	tempNode = ListRemoveHead(l1);
	assert(tempNode == n2);
	tempNode = ListRemoveHead(l1);
	assert(tempNode == n1);
	assert(ListEmpty(l1));

	ListAddHead(l1,n1);
	ListAddHead(l1,n2);
	ListAddHead(l1,n3);
	tempNode = ListRemoveTail(l1);
	assert(tempNode == n1);
	tempNode = ListRemoveTail(l1);
	assert(tempNode == n2);
	tempNode = ListRemoveTail(l1);
	assert(tempNode == n3);
	assert(ListEmpty(l1));

	ListAddTail(l1,n1);
	ListAddTail(l1,n2);
	ListAddTail(l1,n3);
	tempNode = ListRemoveTail(l1);
	assert(tempNode == n3);
	tempNode = ListRemoveTail(l1);
	assert(tempNode == n2);
	tempNode = ListRemoveTail(l1);
	assert(tempNode == n1);
	assert(ListEmpty(l1));

	ListAddHead(l1,n1);
	ListAddHead(l1,n2);
	ListAddHead(l1,n3);
	NodeInsert(n3,n4);
	NodeRemove(n1);
	tempNode = ListRemoveHead(l1);
	assert(tempNode == n3);
	tempNode = ListRemoveHead(l1);
	assert(tempNode == n4);
	tempNode = ListRemoveHead(l1);
	assert(tempNode == n2);
	assert(ListEmpty(l1));

	// prioritized list tests
	ListEnqueue(l1,n1,10);
	ListEnqueue(l1,n3,30);
	ListEnqueue(l1,n4,40);
	ListEnqueue(l1,n2,20);
	assert(NodeGetPriority(n2) == 20);
	tempNode = ListRemoveHead(l1);
	assert(tempNode == n4);
	tempNode = ListRemoveHead(l1);
	assert(tempNode == n3);
	tempNode = ListRemoveHead(l1);
	assert(tempNode == n2);
	tempNode = ListRemoveHead(l1);
	assert(tempNode == n1);
	assert(ListEmpty(l1));

	ListEnqueue(l1,n1,10);
	ListEnqueue(l1,n3,30);
	ListEnqueue(l1,n4,40);
	ListEnqueue(l1,n2,20);
	NodeSetPriority(n3,100);
	tempNode = ListRemoveHead(l1);
	assert(tempNode == n3);
	tempNode = ListRemoveHead(l1);
	assert(tempNode == n4);
	tempNode = ListRemoveHead(l1);
	assert(tempNode == n2);
	tempNode = ListRemoveHead(l1);
	assert(tempNode == n1);
	assert(ListEmpty(l1));

	// clean up
	NodeDelete(n1);
	NodeDelete(n2);
	NodeDelete(n3);
	NodeDelete(n4);
	ListDelete(l1);
}

#endif

//=============================================================================

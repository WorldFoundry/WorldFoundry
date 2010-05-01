//=============================================================================
// _message.h: private messaging system header file for PIGS
//=============================================================================
// use only once insurance

#ifndef __MESSAGE_H
#define __MESSAGE_H

//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//			Messaging system for PIGS tasker
//	History:
//			Created	12-02-94 11:53am Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:
//			SList,SNode,Signal

//	Restrictions:
//			Message can only be in one queue at a time
//			Attaching a MessagePort to a Task allocates a signal,
//				therefore, there can only be 32 MessagePorts attached to
//				any particular task
//	Example:

//=============================================================================

#include <hal/message.h>
#include <hal/_signal.h>

//=============================================================================
// debugging macros
// it turns out all the calls that deal with messages don't expect it to be
// in a list(MessagePort) at the time, so we use VALIDATENODENOTINLIST)

#if DO_ASSERTIONS
#define VALIDATEMESSAGEPORT(iSelf) \
 { \
	SMessagePort* self; \
	VALIDATEITEM(iSelf); \
	self = ITEMRETRIEVE(iSelf,SMessagePort); \
	VALIDATEMESSAGEPORTPTR(self); \
 }

#define VALIDATEMESSAGEPORTPTR(port) \
 {\
	VALIDATEPTR(port); \
	VALIDATELIST(&(port)->_messages);\
	if(port->_behavior == EMP_NOTHING) \
	 { \
		assert(port->_task == 0); \
		assert(port->_signal == 0); \
	 } \
	else \
	 { \
		VALIDATEITEM(port->_task); \
		VALIDATESIGNAL(port->_signal); \
	 } \
	assert((port->_behavior == EMP_NOTHING) || (port->_behavior == EMP_SIGNAL));\
 }
#else
#define VALIDATEMESSAGEPORT(iSelf)
#define VALIDATEMESSAGEPORTPTR(self)
#endif

//-----------------------------------------------------------------------------

struct _MessagePort
{
	SNode _link;					// link into public message port list
	char _name[PORTNAMELEN];		// for finding by name
	portBehavior _behavior;			// what to do when message arrives
	ITask _task;					// if behavior is PORT_SIGNAL, this is the task to signal
	halSignal _signal;		 		// if behavior is PORT_SIGNAL, this is the signal to send
	SList _messages;
};

//=============================================================================
// since SMessagePortList is just a _list

struct _MessagePortList
{
	SList _messagePortList;
};

//-----------------------------------------------------------------------------

#if DO_ASSERTIONS
#define VALIDATEMESSAGEPORTLIST(iSelf) \
 { \
	SMessagePortList* self; \
	VALIDATEITEM(iSelf); \
	self = ITEMRETRIEVE(iSelf,SMessagePortList); \
	VALIDATEMESSAGEPORTLISTPTR((&self->_messagePortList)); \
 }
#define VALIDATEMESSAGEPORTLISTPTR(self) VALIDATELIST(self);
#else
#define VALIDATEMESSAGEPORTLIST(self)
#define VALIDATEMESSAGEPORTLISTPTR(self)
#endif

//=============================================================================
// private functions

void
_PublicMessagePortListConstruct(void);		// don't call, tasker will upon startup

void
_PublicMessagePortListDestruct(void);		// don't call, tasker will upon cleanup

void
_MessageSystemConstruct(int maxMessages,int maxPorts);		// sets up memPools

void
_MessageSystemDestruct(void);

//=============================================================================

#if TEST_MESSAGE
void
MessageTest(void);
#endif

//=============================================================================
#endif
//=============================================================================

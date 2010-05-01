//=============================================================================
// message.h: messaging system for PIGS
//=============================================================================
// use only once insurance

#ifndef _MESSAGE_H
#define _MESSAGE_H

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
// dependencies

#include <hal/halbase.h>
#include <hal/tasker.h>
#include <hal/item.h>

//=============================================================================
// controls message port behavior, whether to signal a task or not when message arrives

typedef enum 			// PortBehavior
{
	EMP_NOTHING = 0,		// do nothing when message arrives, just queue
	EMP_SIGNAL				// send signal to task when message arrives
} portBehavior;

//-----------------------------------------------------------------------------
#define PORTNAMELEN 16

struct _MessagePort;
typedef struct _MessagePort SMessagePort;

ITEMTYPECREATE(IMessagePort,SMessagePort);

//=============================================================================

IMessagePort
MessagePortNew(char* name);

IMessagePort
MessagePortDelete(IMessagePort iSelf);

void
MessagePortAttachTask(IMessagePort iSelf, ITask task);	// must do this to allow waiting on a port

ITask
MessagePortGetTask(IMessagePort iSelf);					// get the ITask for a message port.	Beware, a message port may not have its own task

const char*
MessagePortGetName(IMessagePort iSelf);					// get const ptr to message port name

void
MessagePortSetName(IMessagePort iSelf,char* name);		// set message port name

void
MessagePortPut(IMessagePort iSelf,int16 type, int32 message);	// send message to this port

void
MessagePortPutInterrupt(IMessagePort iSelf,int16 type, int32 message); // send message to this port from an interrupt

void
MessagePortWait(IMessagePort iSelf);					// wait until message arrives on this port

bool
MessagePortIsWaiting(IMessagePort iSelf);				// is message waiting?

// kts I hate passing a ptr to an int32 to get the message, there has to be a
// better way

int16
MessagePortGet(IMessagePort iSelf,int32* pMessage);      		// if no message waiting, asserts

//=============================================================================
// list function prototypes

struct _MessagePortList;
typedef struct _MessagePortList SMessagePortList;

ITEMTYPECREATE(IMessagePortList,SMessagePortList);

//=============================================================================

IMessagePortList
MessagePortListNew(void);

IMessagePortList
MessagePortListDelete(IMessagePortList iSelf);

void
MessagePortListAdd(IMessagePortList iSelf, IMessagePort port);

void
MessagePortListRemove(IMessagePortList iSelf, IMessagePort port);

IMessagePort
MessagePortListFind(IMessagePortList iSelf,char* name);		// search list of message ports for port with name matching name

//=============================================================================
// public port list

void
PublicMessagePortListAdd(IMessagePort port);

void
PublicMessagePortListRemove(IMessagePort port);

IMessagePort
PublicMessagePortFind(char* name);				// returns NULL ptr if not found

//=============================================================================
#endif
//=============================================================================

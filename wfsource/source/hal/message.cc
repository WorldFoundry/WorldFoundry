//=============================================================================
// message.c: messaging system for PIGS
//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:

//	History:
//			Created	12-13-94 10:58am Kevin T. Seghetti
//			(in progress)

//	Class Hierarchy:
//			none

//	Dependancies:
//			SList,SNode,STask,signal

//	Restrictions:

//	Example:

//=============================================================================
// dependencies

#define _mESSAGE_C

#include <hal/halbase.h>
#include <hal/_message.h>
#include <hal/message.h>
#include <hal/_signal.h>
#include <hal/general.h>
#include <hal/_mempool.h>

//=============================================================================

#if DO_ASSERTIONS
#define VALIDATEMESSAGE(msg) \
 {\
	VALIDATEPTR(msg); \
	VALIDATENODENOTINLIST(&(msg)->_link); \
 }
#else
#define VALIDATEMESSAGE(node)
#endif

//=============================================================================
// messages only convey two pieces of information, a 16-bit type, and a 32
// number, the user can use either of these fields for whatever they want

typedef struct _Message
{
	SNode _link;
	int16 _type;			// message type
	int32 _message;			// actual message
} SMessage;

//-----------------------------------------------------------------------------

SMessage*
MessageNew(int16 type,int32 message);

SMessage*
MessageDelete(SMessage* self);


//=============================================================================
// regional data

SMessagePortList publicMessagePortList;
IMessagePortList iPublicMessagePortList;

static SMemPool *memPoolMessages;
static SMemPool *memPoolMessagePorts;

//=============================================================================

SMessage*
MessageNew(int16 type, int32 message)
{
	SMessage* self;
	self = (SMessage*)MemPoolAllocate(memPoolMessages,sizeof(SMessage));
	NodeConstruct(&self->_link);
	self->_type = type;
	self->_message = message;
	return(self);
}

//-----------------------------------------------------------------------------

SMessage*
MessageDelete(SMessage* self)
{
	VALIDATEMESSAGE(self);
	VALIDATENODENOTINLIST(&self->_link);			// insure not in list
	NodeDestruct(&self->_link);
	MemPoolFree(memPoolMessages,self);
	return(NULL);
}

//=============================================================================
// MessagePorts
//-----------------------------------------------------------------------------

IMessagePort
MessagePortNew(char* name)
{
	IMessagePort iSelf;
	SMessagePort* self;

#if DO_ASSERTIONS
	if(name)
		VALIDATESTRING(name);
#endif
	self = (SMessagePort*)MemPoolAllocate(memPoolMessagePorts,sizeof(SMessagePort));
	iSelf = ITEMCREATE(self,SMessagePort);

	// MessagePortConstruct
	NodeConstruct(&self->_link);

	self->_behavior = EMP_NOTHING;
	self->_task = 0;					// clear item
	self->_signal = SIGNAL_INVALID;
	ListConstruct(&self->_messages);
	self->_name[0] = 0;					// in case no name given
	if(name)
		MessagePortSetName(iSelf,name);
	// end MessagePortConstruct
	return(iSelf);
}

//=============================================================================

IMessagePort
MessagePortDelete(IMessagePort iSelf)
{
	SMessagePort* self;
	VALIDATEMESSAGEPORT(iSelf);
	self = ITEMRETRIEVE(iSelf,SMessagePort);
	ITEMDESTROY(iSelf,SMessagePort);

	assert(ListEmpty(&self->_messages));			// there should not be any messages waiting
	// MessagePortDestruct
	NodeDestruct(&self->_link);
	ListDestruct(&self->_messages);
#if defined( DO_MULTITASKING )
	if(self->_task)
	 {
		SignalFree(self->_task,self->_signal);
	 }
#endif
	// end MessagePortDestruct
	MemPoolFree(memPoolMessagePorts,self);
	return(NULLITEM);
}

//=============================================================================

#if defined( DO_MULTITASKING )
void
MessagePortAttachTask(IMessagePort iSelf, ITask iTask)
{
	SMessagePort* self;
	VALIDATEMESSAGEPORT(iSelf);
	self = ITEMRETRIEVE(iSelf,SMessagePort);
	assert(self->_behavior == EMP_NOTHING);		// cannot attach if already attached
	self->_behavior = EMP_SIGNAL;				// what to do when message arrives
	self->_task = iTask;						// if behavior is PORT_SIGNAL, this is the task to signal
	self->_signal = SignalAlloc(self->_task);	// if behavior is PORT_SIGNAL, this is the signal to send
}
#endif

//=============================================================================

ITask
MessagePortGetTask(IMessagePort iSelf)
{
	SMessagePort* self;
	VALIDATEMESSAGEPORT(iSelf);
	self = ITEMRETRIEVE(iSelf,SMessagePort);
	return(self->_task);
}

//=============================================================================

const char*
MessagePortGetName(IMessagePort iSelf)
{
	SMessagePort* self;
	VALIDATEMESSAGEPORT(iSelf);
	self = ITEMRETRIEVE(iSelf,SMessagePort);
//	return((const char*)&self->_name);
	return((const char*)self->_name);			// kts removed & 07-08-96 03:27pm
}


//=============================================================================

void
MessagePortSetName(IMessagePort iSelf,char* name)
{
	SMessagePort* self;
	VALIDATEMESSAGEPORT(iSelf);
	self = ITEMRETRIEVE(iSelf,SMessagePort);
	VALIDATESTRING(name);
	assert(strlen(name) < PORTNAMELEN);
	StringCopyCount(self->_name,name,PORTNAMELEN);
}

//=============================================================================

void
MessagePortPut(IMessagePort iSelf,int16 type, int32 message)	// send message to this port
{
	SMessage* pMessage;
	SMessagePort* self;
	VALIDATEMESSAGEPORT(iSelf);
	self = ITEMRETRIEVE(iSelf,SMessagePort);
	pMessage = MessageNew(type,message);
	VALIDATEMESSAGE(pMessage);
	ListAddHead(&self->_messages,(SNode*)pMessage);
#if defined( DO_MULTITASKING )
	if(self->_behavior == EMP_SIGNAL)
	 {
		SignalSend(self->_task,self->_signal);
	 }
#endif
}

//=============================================================================

void
MessagePortPutInterrupt(IMessagePort iSelf,int16 type, int32 message)	// send message to this port from an interrupt
{
	SMessage* pMessage;
	SMessagePort* self;
	VALIDATEMESSAGEPORT(iSelf);
	self = ITEMRETRIEVE(iSelf,SMessagePort);
	pMessage = MessageNew(type,message);
	VALIDATEMESSAGE(pMessage);
	ListAddHead(&self->_messages,(SNode*)pMessage);
#if defined( DO_MULTITASKING )
	if(self->_behavior == EMP_SIGNAL)
	 {
		// Note: here we call SignalSendInterrupt, which does not call task switch
		SignalSendInterrupt(self->_task,self->_signal);
	 }
#endif
}

//=============================================================================

#if defined( DO_MULTITASKING )
void
MessagePortWait(IMessagePort iSelf)					// wait until message arrives on this port
{
	SMessagePort* self;
	VALIDATEMESSAGEPORT(iSelf);
	self = ITEMRETRIEVE(iSelf,SMessagePort);
	assert(self->_behavior == EMP_SIGNAL);			// only if signal port will this function work
	while(ListEmpty(&self->_messages))				// insure that recieving a signal leads to having a message
	 {												// note: this prevents more than one port from sharing a signal
		SignalWait(self->_signal);
	 }
}
#endif

//=============================================================================
// if message waiting, returns true

bool
MessagePortIsWaiting(IMessagePort iSelf)			// is message waiting?
{
	SMessagePort* self;
	VALIDATEMESSAGEPORT(iSelf);
	self = ITEMRETRIEVE(iSelf,SMessagePort);
	return(!ListEmpty(&self->_messages));
}

//=============================================================================

int16
MessagePortGet(IMessagePort iSelf, int32* pMessageData)          		// if no message waiting, asserts
{
	SMessage* pMessage;
	int16 type;
	SMessagePort* self;
	VALIDATEMESSAGEPORT(iSelf);
	self = ITEMRETRIEVE(iSelf,SMessagePort);
	assert(MessagePortIsWaiting(iSelf));
	pMessage = (SMessage*)ListRemoveTail(&self->_messages);
	if(pMessageData)
		*pMessageData = pMessage->_message;		    // kts I hate this, there has to be a better way
	type = pMessage->_type;
	MessageDelete(pMessage);
	return(type);
}

//=============================================================================
// these could all become macros in the final release, since its only here
// to prevent callers from having to cast

IMessagePortList
MessagePortListNew(void)
{
	IMessagePortList iSelf;
	SMessagePortList* self;
	self = (SMessagePortList*)ListNew();
	iSelf = ITEMCREATE(self,SMessagePortList);
	return(iSelf);
}

//=============================================================================

IMessagePortList
MessagePortListDelete(IMessagePortList iSelf)
{
	SMessagePort* self;
	VALIDATEMESSAGEPORTLIST(iSelf);
	self = ITEMRETRIEVE(iSelf,SMessagePort);
	ITEMDESTROY(iSelf,SMessagePort);
	ListDelete((SList*)self);
	return(NULLITEM);
}

//=============================================================================

void
MessagePortListAdd(IMessagePortList iSelf, IMessagePort iPort)
{
	SMessagePortList* self;
	SMessagePort* port;
	VALIDATEMESSAGEPORTLIST(iSelf);
	VALIDATEMESSAGEPORT(iPort);
	self = ITEMRETRIEVE(iSelf,SMessagePortList);
	port = ITEMRETRIEVE(iPort,SMessagePort);
	ListAddHead((SList*)self,(SNode*)port);
}

//=============================================================================

void
MessagePortListRemove(IMessagePortList iSelf, IMessagePort iPort)
{
	(void)iSelf;
	SMessagePort* port;
	VALIDATEMESSAGEPORTLIST(iSelf);
	VALIDATEMESSAGEPORT(iPort);
	port = ITEMRETRIEVE(iPort,SMessagePort);
	NodeRemove((SNode*)port);
}

//=============================================================================

IMessagePort
MessagePortListFind(IMessagePortList iSelf,char* name)
{
	SNode* tempNode;
	SMessagePortList* self;
	IMessagePort iPort;
	VALIDATEMESSAGEPORTLIST(iSelf);
	VALIDATESTRING(name);
	assert(strlen(name) <= PORTNAMELEN);

	self = ITEMRETRIEVE(iSelf,SMessagePortList);

	tempNode = ListGetHead((SList*)self);

	while(tempNode)
	 {
		if(!strcmp(name,((SMessagePort*)tempNode)->_name))
		 {
			iPort = ITEMLOOKUP((SMessagePort*)tempNode,SMessagePort);
			return(iPort);
		 }
		tempNode = NodeNext(tempNode);
	 }
	return(NULLITEM);
}

//=============================================================================
// public message port list interface

void
_PublicMessagePortListConstruct(void)
{
	ListConstruct((SList*)&publicMessagePortList);			// initialize public message ports
	iPublicMessagePortList = ITEMCREATE(&publicMessagePortList,SMessagePortList);
}

//=============================================================================

void
_PublicMessagePortListDestruct(void)
{
	ITEMDESTROY(iPublicMessagePortList,SMessagePortList);
	ListDestruct((SList*)&publicMessagePortList);			// initialize public message ports
}

//=============================================================================

void
PublicMessagePortListAdd(IMessagePort iPort)
{
	MessagePortListAdd(iPublicMessagePortList, iPort);
}

//=============================================================================

void
PublicMessagePortListRemove(IMessagePort iPort)
{
	MessagePortListRemove(iPublicMessagePortList, iPort);
}

//=============================================================================

IMessagePort
PublicMessagePortFind(char* name)
{
	VALIDATESTRING(name);
	return(MessagePortListFind(iPublicMessagePortList,name));
}

//=============================================================================
// message system construct/destruct: this is needed for the memory pool tracking

void
_MessageSystemConstruct(int maxMessages,int maxPorts)
{
	memPoolMessages = MemPoolConstruct(sizeof(SMessage),maxMessages,HALLmalloc);
	memPoolMessagePorts = MemPoolConstruct(sizeof(SMessagePort),maxPorts, HALLmalloc);
}

//=============================================================================

void
_MessageSystemDestruct(void)
{
	MemPoolDestruct(memPoolMessages);
	MemPoolDestruct(memPoolMessagePorts);
}

//=============================================================================
// test suite

#if TEST_MESSAGE

void
MessageTest(void)
{
}

#endif

//=============================================================================

//=============================================================================
// timer.c: timer/interrupt system for PIGS/HAL
//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//			Sends messages to subscribing tasks at a regular interval

//	History:
//			Created 01-11-95 01:11pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:
//		Tasker(since each timer dispatcher is a task)
//	Restrictions:

//	Example:
//=============================================================================
// dependencies

#define _tIMER_C

#include <hal/halbase.h>
#include <hal/timer.h>
#include <hal/general.h>
#include <hal/tasker.h>
#include <hal/_signal.h>
#include <hal/_message.h>
#include <hal/message.h>
#include <hal/_mempool.h>

//=============================================================================
// this should be higher than any normal task, since it only runs in response
// to an interrupt

#define TIMER_PRI PRIORITY_MEDIUM
// was 50

//=============================================================================

typedef struct _TimerUserData
{
	// protected, this data needs to be filled in by anyone spawning a timer task
	char name[PORTNAMELEN];
	// private, this data doesn't need to be filled in, its used internally
	SList _clientList;
	int32 _tickCount;
	bool _cont;					// should we continue executing?
} STimerUserData;

//=============================================================================

typedef struct _TimerClientData
{
	SNode _link;

	int32 _rate;						// rate to send messages, 1 = every tick, 2 = every other tick, etc
	int32 _currentRateCounter;			// decrements each tick until 0, then reset to rate
	int32 _count;						// decrements each tick until 0, then deletes client from list

	IMessagePort _clientPort;
	int16 _clientMessageType;

} STimerClientData;

//=============================================================================

static SMemPool* memPoolTimerSubscribers;

//=============================================================================

static void
TimerParseMessage(STimerUserData* pUserData,int32 message, int16 messageType)
{
	// for now, assume there is only one message type
	STimerMessage* pMessage;
	STimerClientData* pClientData;
	pMessage = (STimerMessage*)message;


	assert(messageType < TMT_MAX);
	switch(messageType)
	 {
		case TMT_SUBSCRIBE:
			pClientData = (STimerClientData*)malloc(sizeof(STimerClientData));
			assert( pClientData );
			AssertMemoryAllocation(pClientData);
			NodeConstruct(&pClientData->_link);
			pClientData->_rate = pMessage->rate;
			pClientData->_currentRateCounter = pMessage->rate;
			pClientData->_count = pMessage->count;
			pClientData->_clientPort = pMessage->clientPort;
			pClientData->_clientMessageType = pMessage->clientMessageType;
			ListAddHead(&pUserData->_clientList,(SNode*)pClientData);
			break;
		case TMT_UNSUBSCRIBE:
			pClientData = (STimerClientData*)ListGetHead(&pUserData->_clientList);
			while(
					pClientData
					&& (
						pClientData->_clientPort != pMessage->clientPort
						|| pClientData->_clientMessageType != pMessage->clientMessageType
						|| pClientData->_rate != pMessage->rate
					   )
				 )
				pClientData = (STimerClientData*)NodeNext((SNode*)pClientData);
				assert(pClientData);			// if not found, client not in list

				NodeRemove((SNode*)pClientData);
				free(pClientData);
			break;
		case TMT_SHUTDOWN:
			pUserData->_cont = false;
			break;
	 }
}

//=============================================================================
// sends a timer message to everyone in the client list

static void
TimerSendTick(STimerUserData* pUserData)
{
	STimerClientData* current;
	STimerClientData* next;

	current = (STimerClientData*)ListGetHead(&pUserData->_clientList);
	while(current)
	 {
		next = (STimerClientData*)NodeNext((SNode*)current);

		if(current->_count)
		 {
			current->_count--;
			if(!current->_count)
			 {
				NodeRemove((SNode*)current);

//				NodeDelete((SNode*)current);
				NodeDestruct((SNode*)current);
				free((void*)current);

			 }
		 }
		current->_currentRateCounter--;
		if(!current->_currentRateCounter)
		 {
			current->_currentRateCounter = current->_rate;
			MessagePortPut(current->_clientPort, current->_clientMessageType, pUserData->_tickCount);
		 }
		current = next;
	 }
}

//=============================================================================

static void
TimerTask(void)
{
	char privateName[PORTNAMELEN];
	STimerUserData* pUserData;
	int32 message;
	int16 messageType;
	IMessagePort publicPort;			// public port, users send requests here
	IMessagePort interruptPort;			// private port, only interrupt handler sends tickCount here
	SMessagePort* port;
	halSignalMask signals;
	// init code

	pUserData = (STimerUserData*)TaskGetUserData(GetCurrentTask());

	publicPort = MessagePortNew(pUserData->name);
	VALIDATEITEM(publicPort);
	PublicMessagePortListAdd(publicPort);
	MessagePortAttachTask(publicPort, GetCurrentTask());

	assert(strlen(pUserData->name) < (PORTNAMELEN-1));
	privateName[0] = '_';
	StringCopyCount(&privateName[1],pUserData->name,PORTNAMELEN-1);

	interruptPort = MessagePortNew(privateName);				// who cares what its called
	VALIDATEITEM(interruptPort);
	PublicMessagePortListAdd(interruptPort);
	MessagePortAttachTask(interruptPort, GetCurrentTask());

	// init pUserData Variables
	ListConstruct(&pUserData->_clientList);
	pUserData->_tickCount = 0;
	pUserData->_cont = true;

	// build signal array for both ports, KTS we need an easy way to do this
	port = ITEMRETRIEVE(publicPort,SMessagePort);
	signals = port->_signal;
	port = ITEMRETRIEVE(interruptPort,SMessagePort);
	signals |= port->_signal;

	while(pUserData->_cont)
	 {
		// waits for messages, either from users or from interrupt timer
		SignalWait(signals);				//	MessagePortWait(publicPort);

		if(MessagePortIsWaiting(interruptPort))
		 {
			MessagePortGet(interruptPort,NULL);			// ignore message contents, the arrival is all we need
			pUserData->_tickCount++;
			TimerSendTick(pUserData);
			assert(pUserData->_tickCount);			// if 0, timer overflow
		 }
		if(MessagePortIsWaiting(publicPort))
		 {
			messageType = MessagePortGet(publicPort,&message);
			TimerParseMessage(pUserData,message,messageType);
		 }
	 }
	// ok, time to clean up
	assert(ListEmpty(&pUserData->_clientList));
	PublicMessagePortListRemove(publicPort);
	MessagePortDelete(publicPort);
	PublicMessagePortListRemove(interruptPort);
	MessagePortDelete(interruptPort);
	free(pUserData);
	TaskKill(GetCurrentTask());
	assert(0);
}

//=============================================================================

typedef struct
{
	// protected, this data needs to be filled in by anyone spawning a timer task
	IMessagePort tickPort;
	// private, this data doesn't need to be filled in, its used internally
} STickUserData;

//=============================================================================
// low priority task which triggers timer whenever it gets a chance

static void
TickTask(void)
{
	STickUserData* pUserData;
	pUserData = (STickUserData*)TaskGetUserData(GetCurrentTask());

	while(1)
	 {
		MessagePortPut(pUserData->tickPort, 0, 0);		// sending this message will cause the timer to run, which will cause all of its clients to run
	 }
}

//=============================================================================
// called once by PIGS startup, creates all the timer tasks

ITask iTickTask;

void
_TimerStart(int maxSubscribers)
{
	STimerUserData* userData;
	STickUserData* tickData;

//	 construct the memory pool
	memPoolTimerSubscribers = MemPoolConstruct(sizeof(STimerClientData),maxSubscribers,HALLmalloc);
	assert( ValidPtr( memPoolTimerSubscribers ) );

//	 create the dispatchers
	userData = (STimerUserData*)malloc(sizeof(STimerUserData));
	assert( ValidPtr(userData) );
	AssertMemoryAllocation(userData);
	StringCopyCount(userData->name,"TimerVBlank",PORTNAMELEN);
	CreateTask(&TimerTask,TIMER_PRI,userData);

	userData = (STimerUserData*)malloc(sizeof(STimerUserData));
	assert(ValidPtr(userData));
	AssertMemoryAllocation(userData);
	StringCopyCount(userData->name,"TimerSysClock",PORTNAMELEN);
	CreateTask(&TimerTask,TIMER_PRI,userData);


//	 now create the interrupt handlers
//		 until we have pre-emption, we will just have a very low priority task
//		which fires each time all other tasks are waiting
	TaskSwitch();
	tickData = (STickUserData*)malloc(sizeof(STickUserData));
	assert( ValidPtr(tickData) );
	AssertMemoryAllocation(userData);
	tickData->tickPort = PublicMessagePortFind("_TimerVBlank");
	VALIDATEITEM( tickData->tickPort );
	iTickTask = CreateTask(&TickTask,-100,tickData);
	VALIDATEITEM( iTickTask );
}

//=============================================================================
// called once by PIGS cleanup, destroys all the timer tasks

void
_TimerStop(void)
{
	IMessagePort timerPort;
	void* pUserData;

//	 first shut down interrupt handlers
	pUserData = TaskGetUserData(iTickTask);
	TaskKill(iTickTask);
	free(pUserData);

//	 now shut down dispatchers
	timerPort = PublicMessagePortFind(TIMER_VBLANK);
	assert(timerPort);
	MessagePortPut(timerPort,TMT_SHUTDOWN,0);

	timerPort = PublicMessagePortFind(TIMER_SYSCLOCK);
	assert(timerPort);
	MessagePortPut(timerPort,TMT_SHUTDOWN,0);

//	 assume timer is a higher priority than we are
	assert(TaskGetPriority(GetCurrentTask()) < TIMER_PRI);
	MemPoolDestruct(memPoolTimerSubscribers);
}

//=============================================================================
// convenience calls, not very fast or efficient

void
TimerSubscribe(IMessagePort clientPort,int16 type, int32 rate, int32 count,char* timerName)
{
	IMessagePort timerPort;
	STimerMessage timerMessage;

	// create a timerMessage struct
	timerMessage.clientPort = clientPort;
	timerMessage.clientMessageType =type;
	timerMessage.rate = rate;
	timerMessage.count = count;

	// find vblank timer, and send timer message to it
	timerPort = PublicMessagePortFind(timerName);
	assert(timerPort);
	MessagePortPut(timerPort,TMT_SUBSCRIBE,(int32)&timerMessage);
}

//=============================================================================

void
TimerUnsubscribe(IMessagePort clientPort,int16 type, int32 rate, int32 count,char* timerName)
{
	IMessagePort timerPort;
	STimerMessage timerMessage;

//	 create a timerMessage struct
	timerMessage.clientPort = clientPort;
	timerMessage.clientMessageType =type;
	timerMessage.rate = rate;
	timerMessage.count = count;

//	 find vblank timer, and send timer message to it
	timerPort = PublicMessagePortFind(timerName);
	assert(timerPort);
	MessagePortPut(timerPort,TMT_UNSUBSCRIBE,(int32)&timerMessage);
}

//=============================================================================

int32
TimerGetTime(char* timerName)
{
	IMessagePort timerPort;
	timerPort = PublicMessagePortFind(timerName);
	assert(timerPort);

	assert(0);					// not implemented yet
	// kts write code here
	return(0);
}

//=============================================================================

#if TEST_TIMER

void
TimerTest(void)
{
}
#endif

//=============================================================================

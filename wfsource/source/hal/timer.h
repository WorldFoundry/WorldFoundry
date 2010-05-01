//=============================================================================
// timer.h: PIGS timer interface
//=============================================================================
// use only once insurance

#ifndef _tIMER_H
#define _tIMER_H

//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//			Sends messages to subscribing tasks at a regular interval

//	Sync device, interval, count

//	History:
//			Created	01-10-95 04:16pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:

//	Restrictions:

//	Example:

//=============================================================================
// dependencies

#include <hal/halbase.h>
#include <hal/message.h>
#include <hal/item.h>

//=============================================================================

#define TIMER_VBLANK "TimerVBlank"
#define TIMER_SYSCLOCK "TimerSysClock"

//=============================================================================
// the timer communicates through messages to its clients

typedef enum 		// timerSource, device to derive sync from
{
	ETS_INVALID = 0,
	ETS_VBLANK,  	// ticks at vertical refresh rate, don't assume this is 60hz
	ETS_SYSCLOCK,	// system dependent, use equates to calc real-time
	ETS_MAX			// for assertions
} timerSource;

//=============================================================================

typedef enum 		// timerMessageType
{
	TMT_INVALID = 0,
	TMT_SUBSCRIBE,  	// subscribe to timer, timer will send messages to task
	TMT_UNSUBSCRIBE,	// cancel subscription to timer, timer will no longer send messages to task
	TMT_SHUTDOWN,		// used by system to terminate task, don't use
	TMT_MAX				// for assertions
} timerMessageType;

//=============================================================================
// this is the structure of the message you send to the timer device
// all of the fields must be filled in

typedef struct _TimerMessage
{
	IMessagePort clientPort;			// port you wish to recieve timer messagss on
	int16 clientMessageType;			// message type you wish to recive
	int32 rate;							// rate to send messages, 1 = every tick, 2 = every other tick, etc
	int32 count;						// # of messages to recieve, after this any, timer will forget all about you
										// set to 0 for forever
} STimerMessage;

//=============================================================================
// called once by PIGS startup, creates all the timer tasks

void
_TimerStart(int maxSubscribers);

//=============================================================================
// called once by PIGS cleanup, destroys all the timer tasks

void
_TimerStop(void);

//=============================================================================
// convienece calls

void
TimerSubscribe(IMessagePort clientPort,int16 type, int32 rate, int32 count, char* timerName);

void
TimerUnsubscribe(IMessagePort clientPort,int16 type, int32 rate, int32 count,char* timerName);

int32
TimerGetTime(char* timerName);



//=============================================================================
#endif
//=============================================================================

//============================================================================
// msgport.cc: Message Port class for Actor objects
//============================================================================
// Documentation:
//
//
//	Abstract:
//			Messaging system for communication between Actor objects.  This
//			supliments the existing mailbox system rather than replacing it.
//			(Mailboxes provide one-to-many broadcasts, while MsgPorts provide
//			one-to-one message passing.)
//
//	History:
//			Created	12:44 2/6/96 by Phil Torre
//
//	Class Hierarchy:
//			none
//
//	Dependancies:
//
//	Restrictions:
//
//	Example:
//
//
//============================================================================

#define _MSGPORT_CC
#include "msgport.hp"

//==============================================================================

SMemPool* MsgPort::_msgPortMemPool = NULL;
int MsgPort::_portCount = 0;

//==============================================================================

MsgPort::MsgPort(SMemPool* msgPortMemPool)
{
   assert(msgPortMemPool == NULL || ValidPtr(msgPortMemPool));
   //msgPortMemPool->Validate();
   if(msgPortMemPool && _msgPortMemPool)
      AssertMsg(msgPortMemPool == _msgPortMemPool, "msgPortMemPool = " << msgPortMemPool << ", _msgPortMemPool = " << _msgPortMemPool);     // if this fires then more than one mempool was passed in
   _portCount++;
   _msgPortMemPool = msgPortMemPool;
	ListConstruct(&_messages);		// Initialize list header
}

//============================================================================

MsgPort::~MsgPort()
{
	while ( ListRemoveTail( &_messages ) )
		;		// Make sure message queue is empty
	ListDestruct( &_messages );				// Null out the list header
   _portCount--;
   if(_portCount == 0)
      _msgPortMemPool = NULL;

}

//============================================================================
// Returns true if successful, false if error.

bool MsgPort::PutMsg(int16 msgType, int32 msgData)
{

#pragma message ("KTS " __FILE__ ": this needs to occur somewhere which knows about actors")
   // this assert was to make sure no messages were sent to a statplat (since they can't do anything about collisions anyway)
	//assert( this != &( Actor::_statPlatData._msgPort ) );

   // kts note: if this fires it is most likely a message was sent to a static object, which has a NULL pointer here
   assert(ValidPtr(_msgPortMemPool));

	SMsg* thisMsg = (SMsg*)MemPoolAllocate(_msgPortMemPool, sizeof(SMsg));

	thisMsg->_type = msgType;
	thisMsg->data._message = msgData;

	NodeConstruct(&thisMsg->_link);
	ListAddTail(&_messages, &thisMsg->_link);
	VALIDATELIST(&_messages);
	return true;
}

//============================================================================
// Returns true if successful, false if error.

bool MsgPort::PutMsg(int16 msgType, const void* data, uint32 size)		// Send a message
{

#pragma message ("KTS " __FILE__ ": this needs to occur somewhere which knows about actors"
   // this assert was to make sure no messages were sent to a statplat (since they can't do anything about collisions anyway)
	//assert( this != &( Actor::_statPlatData._msgPort ) );

   // kts note: if this fires it is most likely a message was sent to a static object, which has a NULL pointer here
   assert(ValidPtr(_msgPortMemPool));

   if(size > sizeof(SMsg::_data))
      return false;

	SMsg* thisMsg = (SMsg*)MemPoolAllocate(_msgPortMemPool, sizeof(SMsg));

	thisMsg->_type = msgType;

   memcpy(thisMsg->data.binary,data,size);

	NodeConstruct(&thisMsg->_link);
	ListAddTail(&_messages, &thisMsg->_link);
	VALIDATELIST(&_messages);
	return true;
}

//============================================================================
// Returns true if a message was waiting, false if not.
//bool MsgPort::GetMsg(int16& msgType, int32& msgData)
// note that no size checking is done on the message, it is the callers
// responsibility to provide enough space for the largest message they send

bool 
MsgPort::GetMsg(int16& msgType, void* data, uint32 maxsize)	// Receive a message
{
	SMsg* thisMsg;

	if ( ( thisMsg = (SMsg*)ListRemoveHead(&_messages)) )
	{
		msgType = thisMsg->_type;
      int size = maxsize;
      if(maxsize > sizeof(SMsg::_data))
         size  = sizeof(SMsg::_data);

      memcpy(data,thisMsg->data.binary,size);
		NodeDestruct(&thisMsg->_link);
      // kts note: if this fires it is most likely a message was sent to a static object, which has a NULL pointer here
      assert(ValidPtr(_msgPortMemPool));

		MemPoolFree(_msgPortMemPool, thisMsg);
		return true;
	}
	return false;
}

//============================================================================
// Returns true if a message of the type that was passed in was waiting,
// false if not.
bool MsgPort::GetMsgByType(const int16& msgType, void* data, uint32 maxsize)
{
	SMsg* thisMsg;

	if ( ( thisMsg = (SMsg*)ListRemoveHeadByType(&_messages, msgType)) )
	{
		assert(ValidPtr(thisMsg));

      int size = maxsize;
      if(maxsize > sizeof(SMsg::_data))
         size  = sizeof(SMsg::_data);

      memcpy(data,thisMsg->data.binary,size);
		NodeDestruct(&thisMsg->_link);
      // kts note: if this fires it is most likely a message was sent to a static object, which has a NULL pointer here
      assert(ValidPtr(_msgPortMemPool));

		MemPoolFree(_msgPortMemPool, thisMsg);
		return true;
	}

	return false;
}


//============================================================================

#if SW_DBSTREAM > 0

std::ostream& operator<<(std::ostream& s, const MsgPort &mp)
{
	SNode* nodePtr = mp._messages._head;
	SMsg* message;
	VALIDATELIST(&mp._messages);

	s << "MsgPort Dump:" << std::endl;
	while ((nodePtr->_next != NULL))
	 {
		message = (SMsg*)nodePtr;
		s << "Type: " << message->_type << ", Data: " << std::hex << message->data._message << std::endl;
		nodePtr = nodePtr->_next;
	 }

	return s;
}


std::ostream& operator<<(std::ostream& s, MsgPort::MSG_TYPE theType)
{
	switch (theType)
	{
		case MsgPort::NOTHING:
			s << "NOTHING";
			break;
		case MsgPort::DIE:
			s << "DIE";
			break;
		case MsgPort::DELTA_POWER:
			s << "DELTA_POWER";
			break;
		case MsgPort::DELTA_HEALTH:
			s << "DELTA_HEALTH";
			break;
		case MsgPort::DELTA_SHIELD:
			s << "DELTA_SHIELD";
			break;
		case MsgPort::PLAY_SOUND_EFFECT:
			s << "PLAY_SOUND_EFFECT";
			break;
		case MsgPort::PLAY_DIALOG:
			s << "PLAY_DIALOG";
			break;
		case MsgPort::PLAY_SONG:
			s << "PLAY_SONG";
			break;
		case MsgPort::MOVEMENT_CUE:
			s << "MOVEMENT_CUE";
			break;
		case MsgPort::MOVEMENT_FORCE_X:
			s << "MOVEMENT_FORCE_X";
			break;
		case MsgPort::MOVEMENT_FORCE_Y:
			s << "MOVEMENT_FORCE_Y";
			break;
		case MsgPort::MOVEMENT_FORCE_Z:
			s << "MOVEMENT_FORCE_Z";
			break;
		case MsgPort::COLLISION:
			s << "COLLISION";
			break;
		case MsgPort::SET_MAILBOX:
			s << "SET_MAILBOX";
			break;
		case MsgPort::SET_CAMERA_SHOT:
			s << "SET_CAMERA_SHOT";
			break;
		case MsgPort::SET_ANIM_CYCLE:
			s << "SET_ANIM_CYCLE";
			break;
		case MsgPort::SPECIAL_COLLISION:
			s << "SPECIAL_COLLISION";
			break;
		case MsgPort::OVERRIDE_ANIM_CYCLE:
			s << "OVERRIDE_ANIM_CYCLE";
			break;

		default:
			AssertMsg(0, "Tried to std::ostream << an unknown message type");
	}

	return s;
}


#endif

//============================================================================

// halconv.cc
//
// Convenience calls for message ports and tasks
//

#include <hal/_message.h>

#ifdef __cplusplus
extern "C" {
#endif

IMessagePort
MessagePortNewTask( const char * name )
	{
	ITask iThis = GetCurrentTask();
	IMessagePort port;

	assert( name );
//	ERR_DEBUG_LOG( LOG_VER, ( "%s task name\n", name ) );

	// Message Port for the new task
	port = MessagePortNew( (char *)name );
	VALIDATEMESSAGEPORT(port);

	// Place the message port in the public message port list
	PublicMessagePortListAdd( port );

	// Attach the task to the message port
	MessagePortAttachTask( port, iThis );

//	ERR_DEBUG_LOG( LOG_VER, ( "%s made port\n", name ) );

	return port;
	}


//=============================================================================
// Contructs a message port attached to the current task

IMessagePort
GetNamedPort()
{
	ITask iThis = GetCurrentTask();

	// Because of a bug in the tasker, we have to perform
	// a taskswitch here for the rest of CreateNamedTask to run.
	TaskSwitch();

	return MessagePortNewTask( (const char *)TaskGetName(iThis) );
}

//=============================================================================

IMessagePort
CreateNamedTask( voidFunction* startRoutine, short priority, const char * name )
{
	IMessagePort port;

	// Construct the task with a low priority
	ITask iThis = CreateTask( startRoutine, PRIORITY_LOWEST, NULL );

	// Name the task
	TaskSetName( iThis, (char *)name );

	// Now set the task to its real priority
	TaskSetPriority( iThis, priority );

	// Permit the task and other tasks and chance to run.
	TaskSwitch();

	// The task should have run by now and called MessagePortNewTask,
	// so it should have a public message port
	port = PublicMessagePortFind( (char *)name );

//	ERR_DEBUG_LOG( LOG_VER, ( "%s got port\n", name ) );

	return port;
}

//=============================================================================
#ifdef __cplusplus
};
#endif
//=============================================================================

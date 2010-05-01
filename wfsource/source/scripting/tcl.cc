//==============================================================================
// tcl.cc: tcl interface 
//==============================================================================

#include <pigsys/pigsys.hp>
#include <cpplib/libstrm.hp>
#include "tcl.hp"
#include <tcl.h>

//==============================================================================

IntArrayEntry mailboxIndexArray[] = 
{
#define Comment(val)
#define MAILBOXENTRY(name,value)  { "INDEXOF_" #name, value },
#include <mailbox/mailbox.inc>
    {
         NULL,0
    }
};
#undef MAILBOXENTRY

#pragma message("KTS: these should be in an include file so that all scripting languages can use it")
IntArrayEntry joystickArray[] = 
{
	{  "JOYSTICK_BUTTON_UP", 2048 },
	{  "JOYSTICK_BUTTON_DOWN", 4096 },
	{  "JOYSTICK_BUTTON_RIGHT", 8192 },
	{  "JOYSTICK_BUTTON_LEFT", 16384 },
	{  "JOYSTICK_BUTTON_A", 1 },
	{  "JOYSTICK_BUTTON_B", 2 },
	{  "JOYSTICK_BUTTON_C", 4 },
	{  "JOYSTICK_BUTTON_D", 8 },
	{  "JOYSTICK_BUTTON_E", 16 },
	{  "JOYSTICK_BUTTON_F", 32 },
	{  "JOYSTICK_BUTTON_G", 64 },
	{  "JOYSTICK_BUTTON_H", 128 },
	{  "JOYSTICK_BUTTON_I", 256 },
	{  "JOYSTICK_BUTTON_J", 512 },
	{  "JOYSTICK_BUTTON_K", 1024 },
    {
         NULL,0
    }
};

//==============================================================================
                         
#define check_argc( _min, _max ) \
if ( (argc < (_min)) || (argc > (_max)) ) { interp->result = "wrong # args"; return TCL_ERROR; } 


int 
tcl_read_mailbox( ClientData clientData, Tcl_Interp* interp, int argc, const char* argv[] )
{
    check_argc( 2, 3 );

    TCLInterpreter* THIS = (TCLInterpreter*)clientData;
    THIS->Validate();

    long mailbox;
    // kts these casts are for backward compatibility with tcl 8.3 (8.4 has the proper const parameter signatures)
    if(Tcl_ExprLong(interp, const_cast<char*>(argv[1]), &mailbox) != TCL_OK)
    {
        Tcl_AppendResult(interp, "read mailbox:\"",argv[1],"\" is not valid, perhaps you forgot a $? ", (char*) NULL);
        return TCL_ERROR;
    }

    Scalar valMailbox;

    DBSTREAM2( cscripting << "read-mailbox[" << mailbox << "]"; ) 

    long actorIndex=THIS->_currentObjectIndex;

    if(argc==3)
    {
        if(Tcl_ExprLong(interp, const_cast<char*>(argv[2]), &actorIndex) != TCL_OK)
        {
            Tcl_AppendResult(interp, "read mailbox:\"",argv[2],"\" is not valid, perhaps you forgot a $? ", (char*) NULL);
            return TCL_ERROR;
        }
    }

   DBSTREAM1(cscripting << "tclreadmailbox: argc = " << argc << ", actorIndex = " << actorIndex << ", mailbox = " << mailbox << std::endl; )
   Mailboxes& mailboxes = THIS->_mailboxesManager.LookupMailboxes(actorIndex);
   valMailbox =  mailboxes.ReadMailbox( mailbox);

   valMailbox.AsText(interp->result,25);        // kts I don't actually know how long it is

//    if(valMailbox.AsUnsignedFraction())
//       sprintf( interp->result, "%f", valMailbox.AsLong() / 65536.0 );
//    else
//       sprintf( interp->result, "%d", valMailbox.AsLong() / 65536);
    DBSTREAM2( cscripting << " = " << interp->result << std::endl; )
    return TCL_OK;
}


// kts this is here for backward compatibility with tcl 8.3
int 
tcl_read_mailbox( ClientData clientData, Tcl_Interp* interp, int argc, char* argv[] )
{
    const char** temp = const_cast<const char**>(argv);
    int retVal = tcl_read_mailbox( clientData, interp, argc, temp );
    return retVal;
}


//==============================================================================

int 
tcl_write_mailbox( ClientData clientData, Tcl_Interp* interp, int argc, const char* argv[] )
{
    check_argc( 3, 4 );
    long nMailbox;
    double input;

    TCLInterpreter* THIS = (TCLInterpreter*) clientData;
    THIS->Validate();

    if(Tcl_ExprLong(interp, const_cast<char*>(argv[1]), &nMailbox) != TCL_OK)
    {
        Tcl_AppendResult(interp, "write mailbox:\"",argv[1],"\" is not valid, perhaps you forgot a $? ", (char*) NULL);
        return TCL_ERROR;
    }

    if(Tcl_ExprDouble(interp, const_cast<char*>(argv[2]), &input) != TCL_OK)
    {
        Tcl_AppendResult(interp, "write mailbox:\"",argv[1],"\" is not valid, perhaps you forgot a $? ", (char*) NULL);
        return TCL_ERROR;
    }

    Scalar value( Scalar::FromFloat(input));

    DBSTREAM2( cscripting << "write-mailbox : box " << nMailbox << " now set to " << value << std::endl; ) 

    long actorIndex=THIS->_currentObjectIndex;
    if(argc==4)
    {
        if(Tcl_ExprLong(interp, const_cast<char*>(argv[3]), &actorIndex) != TCL_OK)
        {
            Tcl_AppendResult(interp, "write mailbox:\"",argv[3],"\" is not valid, perhaps you forgot a $? ", (char*) NULL);
            return TCL_ERROR;
        }
    }

    Mailboxes& mailboxes = THIS->_mailboxesManager.LookupMailboxes(actorIndex);
    mailboxes.WriteMailbox(nMailbox, value);
    return TCL_OK;
}


// kts this is here for backward compatibility with tcl 8.3
int 
tcl_write_mailbox( ClientData clientData, Tcl_Interp* interp, int argc, char* argv[] )
{
    const char** temp = const_cast<const char**>(argv);
    int retVal = tcl_write_mailbox( clientData, interp, argc, temp);
    return retVal;
}

//==============================================================================

TCLInterpreter::TCLInterpreter(MailboxesManager& mailboxesManager) : 
  ScriptInterpreter(mailboxesManager),
  _mailboxesManager(mailboxesManager)
{
    _interp = Tcl_CreateInterp();
    assert(ValidPtr(_interp->result));
	Tcl_CreateCommand( _interp, "read-mailbox", tcl_read_mailbox, (ClientData)this, (Tcl_CmdDeleteProc*)NULL );
	Tcl_CreateCommand( _interp, "write-mailbox", tcl_write_mailbox, (ClientData)this, (Tcl_CmdDeleteProc*)NULL );
    AddConstantArray(mailboxIndexArray);
    AddConstantArray(joystickArray);

    // now add mailbox variables with callback
//     IntArrayEntry* entry = mailboxArray;
//     while(entry->name)
//     {
//         Tcl_LinkVar(_interp,(char*)entry->name,(char*)&entry->value,TCL_LINK_INT);
//         entryList++;
//     }

}

//==============================================================================

TCLInterpreter::~TCLInterpreter()
{
     Tcl_DeleteInterp(_interp);
}

//==============================================================================

void
TCLInterpreter::Validate()
{
   // kts need to actually write some validation
}

//==============================================================================

void
TCLInterpreter::AddConstantArray(IntArrayEntry* entryList)
{
    while(entryList->name)
    {
		DBSTREAM3( cscripting << "adding " << entryList->name << " to tcl variables" << std::endl; )
        Tcl_LinkVar(_interp,(char*)entryList->name,(char*)&entryList->value,TCL_LINK_INT|TCL_LINK_READ_ONLY);
        entryList++;
    }
}

//==============================================================================

void
TCLInterpreter::DeleteConstantArray(IntArrayEntry* entryList)
{
    while(entryList->name)
    {
		DBSTREAM3( cscripting << "deleting " << entryList->name << " from tcl variables" << std::endl; )
        Tcl_UnlinkVar(_interp,(char*)entryList->name);
        entryList++;
    }
}

//==============================================================================

Scalar
TCLInterpreter::RunScript(const void* script, int currentObjectIndex)
{
    DBSTREAM2( cscripting << "TCLInterpreter::RunScript:" << std::endl; ) 
    DBSTREAM3( cscripting << "script = " << script << std::endl; ) 
    _currentObjectIndex = currentObjectIndex;
    int code = Tcl_Eval( _interp, (char*)script );

   AssertMsg( code == TCL_OK, "tcl error: " << _interp->result << "on line " << _interp->errorLine << std::endl << script << std::endl );
  float f = atof( _interp->result );
  return Scalar::FromFloat(f);
}

//==============================================================================


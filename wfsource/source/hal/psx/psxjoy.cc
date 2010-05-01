//=============================================================================
// psxjoy.c: psx joystick interface
// Kevin T. Seghetti @ Cave Logic Studios            Feb 22 1995
//=============================================================================

#define _PSXJOY_C
#include <hal/halbase.h>

#include <hal/sjoystic.h>
#include <hal/_input.h>

#include <libetc.h>

//=============================================================================

void
_InitJoystickInterface()
{
//	PadInit(0);	 					    // kts moved to PlatformSpecificInit 1/15/96 1:34PM
										// because must be setup before timers
}

//=============================================================================

void
_TermJoystickInterface()
{
	PadStop();
}

//=============================================================================

int
_JoystickUserAbort()
{
	joystickButtonsF result;
	result = PadRead(0);
//	if((result & (PADstart|PADselect)) != (PADstart|PADselect) )
	if((result & (PADstart)) != (PADstart) )
		return(false);
	return(true);
}

//=============================================================================

joystickButtonsF
_JoystickButtonsF(IJoystick joystick)
{
	joystickButtonsF result;
	SJoystick *self;
	VALIDATEITEM(joystick);
	self = ITEMRETRIEVE(joystick,SJoystick);
	assert(self->_stickNum == EJW_JOYSTICK1 || self->_stickNum == EJW_JOYSTICK2);
	result = PadRead(0);
	if(self->_stickNum == EJW_JOYSTICK2)
		result >>= 16;
	result &= 0xffff;
	return(result);
}

//=============================================================================

# if 0
void
TestJoystick(void)
{
}
#endif

//=============================================================================

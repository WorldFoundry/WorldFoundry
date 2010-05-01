//=============================================================================
// input.c: platform specific input code
//=============================================================================
// Documentation:
//=============================================================================

//	Abstract:

//	History:
//			Created	03-24-95 06:00pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:

//	Restrictions:
//	Example:

//=============================================================================
// select input handlers
//=============================================================================

static whichDevice;

//=============================================================================

#include <hal/hal.h>
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <windowsx.h>
#include "mmsystem.h"

//=============================================================================

//#include "c:/dxsdk/sdk/inc/

//=============================================================================

enum
{
	INPUT_KEYBOARD,
	INPUT_PCJOYSTICK,
	INPUT_DAVESTICK
};

//==============================================================================

// kts for now the gfx library sets the buttons based on windows keyboard
// messages

static joystickButtonsF _buttons = 0;

//=============================================================================
// called from gfx/gl/wgl.c

void
_HALSetJoystickButtons(joystickButtonsF joystickButtons)
{
    _buttons = joystickButtons;
}

//=============================================================================

void
_InitJoystickInterface()       // save standard kbd handlers, install new one
{
}

//=============================================================================

void
_TermJoystickInterface()       // save standard kbd handlers, install new one
{
}

//=============================================================================

int
_JoystickUserAbort()
{
#pragma message( __FILE__ ": I wanted to use the standard defines, but need to call _JoystickBUttonsF -- had to hack Windows-specific values instead [continuing the tradition]" )
//#define	EJ_BUTTONB_A 7
//#define	EJ_BUTTONB_B 4
//#define	EJ_BUTTONB_C 5
//#define	EJ_BUTTONB_D 6

	JOYINFO ji;
	if ( joyGetPos( 0, &ji ) != JOYERR_UNPLUGGED )
	{
//		if(ji.wButtons & (1<<8))
		if( (ji.wButtons & 0x0F) == 0x0F )
			return 1;
	}
	return 0;
}

//=============================================================================

const BORDER = 384;

// all joystick bits use psx mappings, this table maps windows bits to psx bits

int joystickRemapTable[][2] =
{
	{1<<0,EJ_BUTTONF_A },
	{1<<1,EJ_BUTTONF_B },
	{1<<2,EJ_BUTTONF_C },
	{1<<3,EJ_BUTTONF_D },
	{1<<4,EJ_BUTTONF_E },
	{1<<5,EJ_BUTTONF_F },
	{1<<6,EJ_BUTTONF_G },
	{1<<7,EJ_BUTTONF_H },
	{1<<8,EJ_BUTTONF_I },
	{0,0 }
};


joystickButtonsF
_JoystickButtonsF(IJoystick joystick)
{
	static JOYINFO ji;
	SJoystick *self = ITEMRETRIEVE(joystick,SJoystick);
	joystickButtonsF buttons = 0;

	assert ( self->_stickNum == EJW_JOYSTICK1 || self->_stickNum == EJW_JOYSTICK2 );

	//printf( "wXpos = %d\txYpos = %d\n", ji.wXpos, ji.wYpos );

	if ( self->_stickNum == EJW_JOYSTICK1 )
	{
		buttons = _buttons;  // kts start with buttons from keyboard for stick 1, just or them together for now					   

		if ( joyGetPos( 0, &ji ) != JOYERR_UNPLUGGED )
		{
			for(int index=0;joystickRemapTable[index][0];index++)
				if(ji.wButtons & joystickRemapTable[index][0])
					buttons |= joystickRemapTable[index][1];
//			buttons = ji.wButtons & 0x3FF;
			if ( ji.wXpos < BORDER )
				buttons |= EJ_BUTTONF_LEFT;
			if ( ji.wXpos > 65535-BORDER )
				buttons |= EJ_BUTTONF_RIGHT;
			if ( ji.wYpos < BORDER )
				buttons |= EJ_BUTTONF_UP;
			if ( ji.wYpos > 65535-BORDER )
				buttons |= EJ_BUTTONF_DOWN;
		}
	}

	if ( self->_stickNum == EJW_JOYSTICK2 )
	{
		if ( joyGetPos( 1, &ji ) != JOYERR_UNPLUGGED )
		{
			for(int index=0;joystickRemapTable[index][0];index++)
				if(ji.wButtons & joystickRemapTable[index][0])
					buttons |= joystickRemapTable[index][1];
//			buttons = ji.wButtons & 0x3FF;
			if ( ji.wXpos < BORDER )
				buttons |= EJ_BUTTONF_LEFT;
			if ( ji.wXpos > 65535-BORDER )
				buttons |= EJ_BUTTONF_RIGHT;
			if ( ji.wYpos < BORDER )
				buttons |= EJ_BUTTONF_UP;
			if ( ji.wYpos > 65535-BORDER )
				buttons |= EJ_BUTTONF_DOWN;
		}
	}

	return buttons;
}

//=============================================================================

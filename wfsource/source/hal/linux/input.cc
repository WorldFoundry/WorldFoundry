//=============================================================================
// linux/input.c: platform specific input code
// Copyright ( c ) 1999 World Foundry Group.
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org

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

//=============================================================================

#include <hal/hal.h>

//=============================================================================

enum
{
	INPUT_KEYBOARD,
	INPUT_PCJOYSTICK
};

//==============================================================================

// kts for now the gfx library sets the buttons based on X keyboard
// messages

static joystickButtonsF _buttons = 0;

//=============================================================================
// called from gfx/mesa.c

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
    if(_buttons & 0x8000000)
        return 1;
	return 0;
}

//=============================================================================

const int BORDER = 384;

// all joystick bits use psx mappings, this table maps linuix bits to psx bits

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
//	static JOYINFO ji;
	SJoystick *self = ITEMRETRIEVE(joystick,SJoystick);
	joystickButtonsF buttons = 0;

	assert ( self->_stickNum == EJW_JOYSTICK1 || self->_stickNum == EJW_JOYSTICK2 );

	//printf( "wXpos = %d\txYpos = %d\n", ji.wXpos, ji.wYpos );

	if ( self->_stickNum == EJW_JOYSTICK1 )
	{
        buttons = _buttons;
#if 0
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
#endif
	}

	if ( self->_stickNum == EJW_JOYSTICK2 )
	{
#if 0
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
#endif
	}

	return buttons;
}

//=============================================================================

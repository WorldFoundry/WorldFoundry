//=============================================================================
// int9.h: Replacement keyboard handler for PC
//=============================================================================
// use only once insurance

#ifndef __INPUT_H
#define __INPUT_H

//=============================================================================
// Documentation:
//=============================================================================

//	Abstract:

//	History:
//			Created Joseph Boyle @ Cave Logic Studios  Jan 11 1995
//			Updated 03-24-95 06:35pm Kevin T. Seghetti, renamed to _input.h from int9.h, added pc joystick interface
//	Class Hierarchy:
//			none

//	Dependancies:

//	Restrictions:

//	Example:
//=============================================================================

#include <hal/sjoystic.h>

void _InitJoystickInterface(void);
void _TermJoystickInterface(void);
joystickButtonsF _JoystickButtonsF(IJoystick joystick);
int  _JoystickUserAbort(void);

#ifdef TEST_JOYSTICK
void _TestJoystickInterface(void);
#endif

//=============================================================================
#endif
//=============================================================================

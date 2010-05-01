//=============================================================================
// SJoystick.h: HAL interface to joystick and other input devices
//=============================================================================
// use only once insurance

#ifndef	_sjOYSTICK_H
#define	_sjOYSTICK_H

//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//		this library implements a platform independent joystick interface

//	History:
//			Created	10-21-94 03:24pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:
//			halbase.h

//	Restrictions:

//	Example:

//=============================================================================
// dependencies

#include <hal/halbase.h>
#include <hal/item.h>

//=============================================================================
// debugging macros

#if DO_ASSERTIONS
#define VALIDATEJOYSTICK(iSelf) \
 { \
	SJoystick* self; \
	VALIDATEITEM(iSelf); \
	self = ITEMRETRIEVE(iSelf,SJoystick); \
	VALIDATEJOYSTICKPTR(self); \
 }

#define VALIDATEJOYSTICKPTR(self) \
 {\
	VALIDATEPTR(self); \
	assert(self->_stickNum > EJW_INVALID && self->_stickNum < EJW_JOYSTICK_MAX); \
 }
#else
#define VALIDATEJOYSTICK(iSelf)
#define VALIDATEJOYSTICKPTR(self)
#endif

//=============================================================================
// globals

// joystickButtons

                       
// #define EJ_BUTTONB_UP 12
// #define  EJ_BUTTONB_DOWN 14
// #define  EJ_BUTTONB_RIGHT 13
// #define  EJ_BUTTONB_LEFT 15
// #define  EJ_BUTTONB_A 7
// #define  EJ_BUTTONB_B 4
// #define  EJ_BUTTONB_C 5
// #define  EJ_BUTTONB_D 6
// #define  EJ_BUTTONB_E 0
// #define  EJ_BUTTONB_F 2
// #define  EJ_BUTTONB_G 1
// #define  EJ_BUTTONB_H 3
// #define  EJ_BUTTONB_I 8
                       
                       
#if 1
#define EJ_BUTTONB_UP 11
#define	EJ_BUTTONB_DOWN 12
#define	EJ_BUTTONB_RIGHT 13
#define	EJ_BUTTONB_LEFT 14
#define	EJ_BUTTONB_A 0
#define	EJ_BUTTONB_B 1
#define	EJ_BUTTONB_C 2
#define	EJ_BUTTONB_D 3
#define	EJ_BUTTONB_E 4
#define	EJ_BUTTONB_F 5
#define	EJ_BUTTONB_G 6
#define	EJ_BUTTONB_H 7
#define	EJ_BUTTONB_I 8
#define	EJ_BUTTONB_J 9
#define	EJ_BUTTONB_K 10
#define	EJ_BUTTONB_L 31
#define	EJ_BUTTONB_M 31
#define	EJ_BUTTONB_N 31
#define	EJ_BUTTONB_O 31
#define	EJ_BUTTONB_P 31
#define	EJ_BUTTONB_Q 31
#define	EJ_BUTTONB_R 31
#define	EJ_BUTTONB_S 31
#define	EJ_BUTTONB_T 31
#define	EJ_BUTTONB_U 31
#define	EJ_BUTTONB_V 31
#define	EJ_BUTTONB_W 31
#define	EJ_BUTTONB_X 31
#define	EJ_BUTTONB_Y 31
#define	EJ_BUTTONB_Z 31
#define	EJ_BUTTONB_1 31
#define	EJ_BUTTONB_2 31
#else
#if defined( __PSX__ )
#define EJ_BUTTONB_UP 12
#define	EJ_BUTTONB_DOWN 14
#define	EJ_BUTTONB_RIGHT 13
#define	EJ_BUTTONB_LEFT 15
#define	EJ_BUTTONB_A 7
#define	EJ_BUTTONB_B 4
#define	EJ_BUTTONB_C 5
#define	EJ_BUTTONB_D 6
#define	EJ_BUTTONB_E 0
#define	EJ_BUTTONB_F 2
#define	EJ_BUTTONB_G 1
#define	EJ_BUTTONB_H 3
#define	EJ_BUTTONB_I 8
#define	EJ_BUTTONB_J 11
#define	EJ_BUTTONB_K 31
#define	EJ_BUTTONB_L 31
#define	EJ_BUTTONB_M 31
#define	EJ_BUTTONB_N 31
#define	EJ_BUTTONB_O 31
#define	EJ_BUTTONB_P 31
#define	EJ_BUTTONB_Q 31
#define	EJ_BUTTONB_R 31
#define	EJ_BUTTONB_S 31
#define	EJ_BUTTONB_T 31
#define	EJ_BUTTONB_U 31
#define	EJ_BUTTONB_V 31
#define	EJ_BUTTONB_W 31
#define	EJ_BUTTONB_X 31
#define	EJ_BUTTONB_Y 31
#define	EJ_BUTTONB_Z 31
#define	EJ_BUTTONB_1 31
#define	EJ_BUTTONB_2 31
#elif defined( __WIN__ )
#define EJ_BUTTONB_UP 13
#define	EJ_BUTTONB_DOWN 12
#define	EJ_BUTTONB_RIGHT 10
#define	EJ_BUTTONB_LEFT 11
#define	EJ_BUTTONB_A 0		// a on sidewinder
#define	EJ_BUTTONB_B 1      // b on sidewinder
#define	EJ_BUTTONB_C 2      // c on sidewinder
#define	EJ_BUTTONB_D 3		// X on DaveStick, x on sidewinder
#define	EJ_BUTTONB_E 4      // Y on DaveStick, y on sidewinder
#define	EJ_BUTTONB_F 5      // Z on DaveStick, z on sidewinder
#define	EJ_BUTTONB_G 6      // P on DaveStick     , left trigger on sidewinder
#define	EJ_BUTTONB_H 7      // Q on      DaveStick, right trigger on sidewinder
#define	EJ_BUTTONB_I 8      // select on DaveStick, start on sidewinder
#define	EJ_BUTTONB_J 9      // play on   DaveStick, M on sidewinder
#define	EJ_BUTTONB_K 31
#define	EJ_BUTTONB_L 31
#define	EJ_BUTTONB_M 31
#define	EJ_BUTTONB_N 31
#define	EJ_BUTTONB_O 31
#define	EJ_BUTTONB_P 31
#define	EJ_BUTTONB_Q 31
#define	EJ_BUTTONB_R 31
#define	EJ_BUTTONB_S 31
#define	EJ_BUTTONB_T 31
#define	EJ_BUTTONB_U 31
#define	EJ_BUTTONB_V 31
#define	EJ_BUTTONB_W 31
#define	EJ_BUTTONB_X 31
#define	EJ_BUTTONB_Y 31
#define	EJ_BUTTONB_Z 31
#define	EJ_BUTTONB_1 31
#define	EJ_BUTTONB_2 31
#else
#	error Unknown platform -- how are the joystick buttons mapped?
#endif
#endif

typedef long joystickButtons;

// joystickButtonsF
#define	EJ_BUTTONF_NONE 0
#define	EJ_BUTTONF_UP (1<<EJ_BUTTONB_UP)
#define	EJ_BUTTONF_DOWN (1<<EJ_BUTTONB_DOWN)
#define	EJ_BUTTONF_RIGHT (1<<EJ_BUTTONB_RIGHT)
#define	EJ_BUTTONF_LEFT (1<<EJ_BUTTONB_LEFT)
#define	EJ_BUTTONF_A (1<<EJ_BUTTONB_A)
#define	EJ_BUTTONF_B (1<<EJ_BUTTONB_B)
#define	EJ_BUTTONF_C (1<<EJ_BUTTONB_C)
#define	EJ_BUTTONF_D (1<<EJ_BUTTONB_D)
#define	EJ_BUTTONF_E (1<<EJ_BUTTONB_E)
#define	EJ_BUTTONF_F (1<<EJ_BUTTONB_F)
#define	EJ_BUTTONF_G (1<<EJ_BUTTONB_G)
#define	EJ_BUTTONF_H (1<<EJ_BUTTONB_H)
#define	EJ_BUTTONF_I (1<<EJ_BUTTONB_I)
#define	EJ_BUTTONF_J (1<<EJ_BUTTONB_J)
#define	EJ_BUTTONF_K (1<<EJ_BUTTONB_K)
#define	EJ_BUTTONF_L (1<<EJ_BUTTONB_L)
#define	EJ_BUTTONF_M (1<<EJ_BUTTONB_M)
#define	EJ_BUTTONF_N (1<<EJ_BUTTONB_N)
#define	EJ_BUTTONF_O (1<<EJ_BUTTONB_O)
#define	EJ_BUTTONF_P (1<<EJ_BUTTONB_P)
#define	EJ_BUTTONF_Q (1<<EJ_BUTTONB_Q)
#define	EJ_BUTTONF_R (1<<EJ_BUTTONB_R)
#define	EJ_BUTTONF_S (1<<EJ_BUTTONB_S)
#define	EJ_BUTTONF_T (1<<EJ_BUTTONB_T)
#define	EJ_BUTTONF_U (1<<EJ_BUTTONB_U)
#define	EJ_BUTTONF_V (1<<EJ_BUTTONB_V)
#define	EJ_BUTTONF_W (1<<EJ_BUTTONB_W)
#define	EJ_BUTTONF_X (1<<EJ_BUTTONB_X)
#define	EJ_BUTTONF_Y (1<<EJ_BUTTONB_Y)
#define	EJ_BUTTONF_Z (1<<EJ_BUTTONB_Z)
#define	EJ_BUTTONF_1 (1<<EJ_BUTTONB_1)
#define	EJ_BUTTONF_2 (1<<EJ_BUTTONB_2)

typedef long joystickButtonsF;

typedef enum
{
	EJW_INVALID = 0,					// 0 is invalid
	EJW_JOYSTICK1,						// note: always use these, since the
	EJW_JOYSTICK2,						// actual values may change for each
	EJW_JOYSTICK_MAX					// platform
} joystickWhich;

typedef struct _Joystick
{
	int _stickNum;
} SJoystick;

//=============================================================================

ITEMTYPECREATE(IJoystick,SJoystick);

//=============================================================================
// global functions

IJoystick
JoystickNew(joystickWhich which);

IJoystick
JoystickDelete(IJoystick iSelf);

joystickButtonsF
JoystickGetButtonsF(IJoystick iSelf);

int _JoystickUserAbort(void);

#if TEST_JOYSTICK
void
JoystickButtonsFPrint(joystickButtonsF buttons);
#endif

#if TEST_JOYSTICK
void
JoystickTest(void);
#endif

//=============================================================================
#endif	// _sjOYSTICK_H
//=============================================================================

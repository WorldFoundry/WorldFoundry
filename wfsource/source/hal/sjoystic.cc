//=============================================================================
// SJoystick.c:
//=============================================================================

#define _sjOYSTICK_C
#include <hal/sjoystic.h>
#include <hal/_input.h>

//=============================================================================

IJoystick
JoystickNew(joystickWhich which)
{
	SJoystick* self;
	IJoystick iSelf;
	self = new (HALLmalloc) SJoystick;
	assert( self );
	AssertMemoryAllocation(self);
	assert(which == EJW_JOYSTICK1 || which == EJW_JOYSTICK2);

	// init code goes here


	self->_stickNum = which;
	iSelf = ITEMCREATE(self,SJoystick);
	return(iSelf);
}

//=============================================================================

IJoystick
JoystickDelete(IJoystick iSelf)
{
	SJoystick* self;
	VALIDATEJOYSTICK(iSelf);
	self = ITEMRETRIEVE(iSelf,SJoystick);
	ITEMDESTROY(iSelf,SJoystick);

	// shut-down code goes here

	HALLmalloc.Free(self);
	return(NULLITEM);
}

//=============================================================================

joystickButtonsF
JoystickGetButtonsF(IJoystick iSelf)
{
	VALIDATEJOYSTICK(iSelf);
	return(_JoystickButtonsF(iSelf));
}

//=============================================================================

#if		TEST_JOYSTICK

void
JoystickButtonsFPrint(joystickButtonsF buttons)
{
	if (buttons == ~0)
          {  printf("ERROR on INPUT");
             return;
          }
   	if (buttons & EJ_BUTTONF_UP) 	printf("Up ");
	if (buttons & EJ_BUTTONF_DOWN) 	printf("Down ");
	if (buttons & EJ_BUTTONF_RIGHT) printf("Right ");
	if (buttons & EJ_BUTTONF_LEFT) 	printf("Left ");
	printf(" ");
	if (buttons & EJ_BUTTONF_A) printf(" A");
	if (buttons & EJ_BUTTONF_B) printf(" B");
	if (buttons & EJ_BUTTONF_C) printf(" C");
 	if (buttons & EJ_BUTTONF_D) printf(" D");
 	if (buttons & EJ_BUTTONF_E) printf(" E");
 	if (buttons & EJ_BUTTONF_F) printf(" F");
 	if (buttons & EJ_BUTTONF_G) printf(" G");
 	if (buttons & EJ_BUTTONF_H) printf(" H");
 	if (buttons & EJ_BUTTONF_I) printf(" I");
 	if (buttons & EJ_BUTTONF_J) printf(" J");
 	if (buttons & EJ_BUTTONF_K) printf(" K");
 	if (buttons & EJ_BUTTONF_L) printf(" L");
 	if (buttons & EJ_BUTTONF_M) printf(" M");
 	if (buttons & EJ_BUTTONF_N) printf(" N");
 	if (buttons & EJ_BUTTONF_O) printf(" O");
 	if (buttons & EJ_BUTTONF_P) printf(" P");
 	if (buttons & EJ_BUTTONF_Q) printf(" Q");
 	if (buttons & EJ_BUTTONF_R) printf(" R");
 	if (buttons & EJ_BUTTONF_S) printf(" S");
 	if (buttons & EJ_BUTTONF_T) printf(" T");
 	if (buttons & EJ_BUTTONF_U) printf(" U");
 	if (buttons & EJ_BUTTONF_V) printf(" V");
 	if (buttons & EJ_BUTTONF_W) printf(" W");
 	if (buttons & EJ_BUTTONF_X) printf(" X");
 	if (buttons & EJ_BUTTONF_Y) printf(" Y");
 	if (buttons & EJ_BUTTONF_Z) printf(" Z");
 	if (buttons & EJ_BUTTONF_1) printf(" 1");
 	if (buttons & EJ_BUTTONF_2) printf(" 2");
 	printf("	");
}

void
JoystickTest(void)
	{
	IJoystick			joy1, joy2;
	joystickButtonsF	b, b1 = -1, b2 = -1;

	printf("Testing joystick/kbd\n");
	joy1 = JoystickNew(EJW_JOYSTICK1);
	joy2 = JoystickNew(EJW_JOYSTICK2);

	while (!_JoystickUserAbort())
		{
		b = JoystickGetButtonsF(joy1);
		if ( b != b1 )
			{
			b1 = b;
			printf("Joystick1: ");
	   		JoystickButtonsFPrint(b1);
			printf("\n");
	   		}
		if ( b & EJ_BUTTONF_I )
			break;
	   	b = JoystickGetButtonsF(joy2);
		if ( b != b2 )
			{
			b2 = b;
			printf("Joystick2: ");
			JoystickButtonsFPrint(b2);
			printf("\n");
	   		}
		}
	printf("JoystickTest: User abort\n");
	JoystickDelete(joy1);
	JoystickDelete(joy2);
	}

#endif	//TEST_JOYSTICK

//=============================================================================

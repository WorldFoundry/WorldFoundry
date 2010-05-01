//=============================================================================
// int9.cc: Replacement keyboard handler for PC
// Joseph Boyle @ World Foundry Group            Jan 11 1995
//=============================================================================

#ifdef __WATCOMC__
#include <dos.h>
#define D32RealSeg(P)   ((((DWORD) (P)) >> 4) & 0xFFFF)
#define D32RealOff(P)   (((DWORD) (P)) & 0xF)
#endif

#define _INT9_C
#include "halbase.h"

#define WATCOM

//=============================================================================

#include "sjoystic.h"
#include "_input.h"
#include "pcscanco.h"

//=============================================================================
// assembly routines

int  Int9GetChar(void);
void Int9Release(void);

//=============================================================================

volatile unsigned char KbdTable[256];

void __interrupt
Int9Handler(void)
	{
	int c;
	static bool bPaused = false;

	do
		{
		c = Int9GetChar();
#if DEBUG || defined( ALLOW_USER_PAUSE )
		if ( (c&0x7F) == PC_SCANCODE_SCROLLLOCK /*PC_SCANCODE_PAUSE*/ )	// key up or down
			bPaused = true;
		else
#endif
			{
			if ( bPaused )
				bPaused = false;
			else
				{
				if (c&0x80)
					KbdTable[c&0x7F] = 0;
				else
					KbdTable[c&0x7F] = 0x80;
				}
			}
		Int9Release();
		}
	until ( !bPaused );


#ifdef PHARLAP
	if (KbdTable[0x1d] && KbdTable[0x2e]) _TermJoystickInterface(); // Control-C
		// Currently not working under Rational / Watcom DOS extender
#endif // PharLap
	}


#ifdef PHARLAP
FARPTR  old_prot_handler;
REALPTR old_real_handler;
#endif // PHARLAP
#ifdef WATCOM
typedef unsigned int WORD;
typedef unsigned long DWORD;
WORD   orig_pm_sel;
WORD   orig_pm_sel_buffer;
DWORD  orig_pm_off;
DWORD  orig_pm_off_buffer;
#endif // WATCOM

//=============================================================================

static bool		gJoyInited	= false;

static void
_atExitTermJoystick(int code)
{
	(void)code;					// suppress unused warnings
	_TermJoystickInterface();
}


void
_InitJoystickInterfaceKeyboard()       // save standard kbd handlers, install new one
{
	if ( gJoyInited == false ) {
#if defined(PHARLAP)
		_dx_pmiv_get(9, &old_prot_handler);		//      Get Prot Mode Int Vect
		_dx_rmiv_get(9, &old_real_handler);		//      Get Real Mode Int Vect
		_dx_apmiv_set(9, (FARPTR)Int9Handler);	// Set Real&Prot Mode Int Vect
		// PHARLAP
#elif defined(WATCOM)
		union REGS      r;
		struct SREGS    sr;
		void far       *fh;

		// Save the starting protected-mode handler address
		r.x.eax = 0x3509;   // DOS get vector (INT 09h)
		sr.ds = sr.es = 0;
		int386x (0x21, &r, &r, &sr);
		orig_pm_sel = (WORD) sr.es;
		orig_pm_off = r.x.ebx;
		//Install the new protected-mode vector.  Because INT 09h
		//is in the auto-passup range, its normal "passdown"
		//behavior will change as soon as we install a
		//protected-mode handler.  After this next call, when a
		//real mode INT 0Ch is generated, it will be resignalled
		//in protected mode and handled by pmhandler.
		r.x.eax = 0x2509;   // DOS set vector (INT 09h)
		fh = (void far *)Int9Handler;
		r.x.edx = FP_OFF (fh);
		// DS:EDX == &handler
		sr.ds = FP_SEG (fh);
		sr.es = 0;
		int386x (0x21, &r, &r, &sr);
		 // WATCOM
#else
#error Must define a DOS extender for the keyboard handler
#endif


		gJoyInited = true;
		sys_atexit(&_atExitTermJoystick);	// Make sure it gets called on error exit
	}
}

//-----------------------------------------------------------------------------

void
_TermJoystickInterfaceKeyboard()       // restore old kbd interrupt handlers
{
	if ( gJoyInited ) {
#ifdef PHARLAP
		_dx_rmiv_set(9, old_real_handler);      // Set Real Mode Int Vect
		_dx_pmiv_set(9, old_prot_handler);      // Set Prot Mode Int Vect
#endif
#ifdef WATCOM
		union REGS      r;
		struct SREGS    sr;
		r.x.eax = 0x2509;   // DOS set vector (INT 09h)
		r.x.edx = orig_pm_off;
		sr.ds = orig_pm_sel;    // DS:EDX == &handler
		sr.es = 0;
		int386x (0x21, &r, &r, &sr);
#endif // WATCOM
		assert( gJoyInited );
		gJoyInited = false;
	}
}

//=============================================================================

int
_JoystickUserAbortKeyboard()
{
	return(KbdTable[PC_SCANCODE_KEYPAD_ESC]);	 // Esc key
}

//=============================================================================

#if 0
void
TestKbd()
{       int i;
	_InitJoystickInterface();
	while (!_JoystickUserAbortKeyboard()) {
		printf("Scancodes: ");
		for (i=0; i<128; i++)
			if (KbdTable[i]) printf("%x ",i);

		printf("\n");
		}
	_TermJoystickInterface();
}

#endif

//=============================================================================

joystickButtonsF
IBMButtons1Keyboard() 		// right hand keypad
{
joystickButtonsF result;
result = EJ_BUTTONF_NONE;
if (KbdTable[PC_SCANCODE_HOME]) result |= EJ_BUTTONF_UP | EJ_BUTTONF_LEFT;
if (KbdTable[PC_SCANCODE_PAGEUP]) result |= EJ_BUTTONF_UP | EJ_BUTTONF_RIGHT;
if (KbdTable[PC_SCANCODE_END]) 	result |= EJ_BUTTONF_DOWN | EJ_BUTTONF_LEFT;
if (KbdTable[PC_SCANCODE_PAGEDOWN]) result |= EJ_BUTTONF_DOWN | EJ_BUTTONF_RIGHT;

if (KbdTable[PC_SCANCODE_UPARROW])	result |= EJ_BUTTONF_UP;
if (KbdTable[PC_SCANCODE_DOWNARROW]) result |= EJ_BUTTONF_DOWN;
if (KbdTable[PC_SCANCODE_LEFTARROW]) result |= EJ_BUTTONF_LEFT;
if (KbdTable[PC_SCANCODE_RIGHTARROW]) result |= EJ_BUTTONF_RIGHT;

if (KbdTable[PC_SCANCODE_KEYPAD_PERIOD]) result |= EJ_BUTTONF_A;
if (KbdTable[PC_SCANCODE_KEYPAD_ENTER]) result |= EJ_BUTTONF_B;
if (KbdTable[PC_SCANCODE_KEYPAD_PLUS]) result |= EJ_BUTTONF_C;
if (KbdTable[PC_SCANCODE_KEYPAD_MINUS]) result |= EJ_BUTTONF_D;
if (KbdTable[PC_SCANCODE_KEYPAD_STAR]) result |= EJ_BUTTONF_E;
if (KbdTable[PC_SCANCODE_KEYPAD_SLASH]) result |= EJ_BUTTONF_F;
//if (KbdTable[PC_SCANCODE_KEYPAD_PERIOD]) result |= EJ_BUTTONF_G;
//if (KbdTable[PC_SCANCODE_KEYPAD_0]) result |= EJ_BUTTONF_H;

return(result);
}

//-----------------------------------------------------------------------------

joystickButtonsF
IBMButtons2Keyboard()					// keys 1-8
{
joystickButtonsF result;

result = EJ_BUTTONF_NONE;
if (KbdTable[PC_SCANCODE_Q]) result |= EJ_BUTTONF_UP | EJ_BUTTONF_LEFT;
if (KbdTable[PC_SCANCODE_E]) result |= EJ_BUTTONF_UP | EJ_BUTTONF_RIGHT;
if (KbdTable[PC_SCANCODE_Z]) result |= EJ_BUTTONF_DOWN | EJ_BUTTONF_LEFT;
if (KbdTable[PC_SCANCODE_C]) result |= EJ_BUTTONF_DOWN | EJ_BUTTONF_RIGHT;

if (KbdTable[PC_SCANCODE_W]) result |= EJ_BUTTONF_UP;		// block QWE ASD ZXC
if (KbdTable[PC_SCANCODE_X]) result |= EJ_BUTTONF_DOWN;
if (KbdTable[PC_SCANCODE_A]) result |= EJ_BUTTONF_LEFT;
if (KbdTable[PC_SCANCODE_D]) result |= EJ_BUTTONF_RIGHT;

if (KbdTable[PC_SCANCODE_V]) result |= EJ_BUTTONF_A;
if (KbdTable[PC_SCANCODE_B]) result |= EJ_BUTTONF_B;
if (KbdTable[PC_SCANCODE_N]) result |= EJ_BUTTONF_C;
if (KbdTable[PC_SCANCODE_M]) result |= EJ_BUTTONF_D;
if (KbdTable[PC_SCANCODE_COMMA]) result |= EJ_BUTTONF_E;
//if (KbdTable[PC_SCANCODE_PERIOD]) result |= EJ_BUTTONF_F;
//if (KbdTable[PC_SCANCODE_7]) result |= EJ_BUTTONF_G;
//if (KbdTable[PC_SCANCODE_8]) result |= EJ_BUTTONF_H;

return(result);
}

//=============================================================================

//=============================================================================
// pcscanco.h: keyboard scancodes for PC-AT and above
//=============================================================================
// use only once insurance

#ifndef PCSCANCO_H
#define PCSCANCO_H

//=============================================================================
// Documentation:
//=============================================================================

//	Abstract:

//	History:
//			Created Joseph Boyle @ Cave Logic Studios  Feb 09 1995
//			From Programmer's PC Sourcebook 7.014
//	Class Hierarchy:
//			none

//	Dependencies:	Hahaha

//	Restrictions:	E0 prefixing not supported so far

//	Example:

//=============================================================================

#define PC_SCANCODE_BACKQUOTE	0x29
#define PC_SCANCODE_1	0x02
#define PC_SCANCODE_2	0x03
#define PC_SCANCODE_3	0x04
#define PC_SCANCODE_4	0x05
#define PC_SCANCODE_5	0x06
#define PC_SCANCODE_6	0x07
#define PC_SCANCODE_7	0x08
#define PC_SCANCODE_8	0x09
#define PC_SCANCODE_9	0x0a
#define PC_SCANCODE_0	0x0b
#define PC_SCANCODE_MINUS	0x0C
#define PC_SCANCODE_EQUALS	0x0D
#define PC_SCANCODE_BS	0x0E
#define PC_SCANCODE_TAB	0x0F
#define PC_SCANCODE_Q	0x10
#define PC_SCANCODE_W	0x11
#define PC_SCANCODE_E	0x12
#define PC_SCANCODE_R	0x13
#define PC_SCANCODE_T	0x14
#define PC_SCANCODE_Y	0x15
#define PC_SCANCODE_U	0x16
#define PC_SCANCODE_I	0x17
#define PC_SCANCODE_O	0x18
#define PC_SCANCODE_P	0x19
#define PC_SCANCODE_LEFTBRACKET	0x1A
#define PC_SCANCODE_RIGHTBRACKET	0x1B
#define PC_SCANCODE_BACKSLASH	0x2B
#define PC_SCANCODE_CAPSLOCK	0x3A
#define PC_SCANCODE_A	0x1E
#define PC_SCANCODE_S	0x1F
#define PC_SCANCODE_D	0x20
#define PC_SCANCODE_F	0x21
#define PC_SCANCODE_G	0x22
#define PC_SCANCODE_H	0x23
#define PC_SCANCODE_J	0x24
#define PC_SCANCODE_K	0x25
#define PC_SCANCODE_L	0x26
#define PC_SCANCODE_SEMICOLON	0x27
#define PC_SCANCODE_QUOTE	0x28
#define PC_SCANCODE_ENTER	0x1C
#define PC_SCANCODE_Z	0x2C
#define PC_SCANCODE_X	0x2D
#define PC_SCANCODE_C	0x2E
#define PC_SCANCODE_V	0x2F
#define PC_SCANCODE_B	0x30
#define PC_SCANCODE_N	0x31
#define PC_SCANCODE_M	0x32
#define PC_SCANCODE_COMMA	0x33
#define PC_SCANCODE_PERIOD	0x34
#define PC_SCANCODE_SLASH	0x35
#define PC_SCANCODE_RIGHTSHIFT	0x36
#define PC_SCANCODE_LEFTCTRL	0x1D
#define PC_SCANCODE_LEFTALT	0x38
#define PC_SCANCODE_SPACE	0x39

// Following are allegedly preceded by E0 byte which I didn't detect
// (Therefore they overlap with keypad-generated codes)

#define PC_SCANCODE_RIGHTCTRL	0x1D
#define PC_SCANCODE_RIGHTALT	0x38
#define PC_SCANCODE_INSERT	0x52
#define PC_SCANCODE_DELETE	0x53
#define PC_SCANCODE_LEFTARROW	0x4B
#define PC_SCANCODE_HOME	0x47
#define PC_SCANCODE_END	0x4F
#define PC_SCANCODE_UPARROW	0x48
#define PC_SCANCODE_DOWNARROW	0x50
#define PC_SCANCODE_PAGEUP	0x49
#define PC_SCANCODE_PAGEDOWN	0x51
#define PC_SCANCODE_RIGHTARROW	0x4D

#define PC_SCANCODE_KEYPAD_SLASH	0x35
#define PC_SCANCODE_KEYPAD_STAR	0x37
#define PC_SCANCODE_KEYPAD_ENTER	0x1C

// end of E0 prefixed codes

#define PC_SCANCODE_KEYPAD_PERIOD	0x53
#define PC_SCANCODE_KEYPAD_MINUS	0x4A
#define PC_SCANCODE_KEYPAD_PLUS	0x4E

#define PC_SCANCODE_KEYPAD_1	0x4F
#define PC_SCANCODE_KEYPAD_2	0x50
#define PC_SCANCODE_KEYPAD_3	0x51
#define PC_SCANCODE_KEYPAD_4	0x4B
#define PC_SCANCODE_KEYPAD_5	0x4C
#define PC_SCANCODE_KEYPAD_6	0x4D
#define PC_SCANCODE_KEYPAD_7	0x47
#define PC_SCANCODE_KEYPAD_8	0x48
#define PC_SCANCODE_KEYPAD_9	0x49
#define PC_SCANCODE_KEYPAD_0	0x52

#define PC_SCANCODE_KEYPAD_ESC	0x01


#define PC_SCANCODE_F1	0x3B
#define PC_SCANCODE_F2	0x3C
#define PC_SCANCODE_F3	0x3D
#define PC_SCANCODE_F4	0x3E
#define PC_SCANCODE_F5	0x3F
#define PC_SCANCODE_F6	0x40
#define PC_SCANCODE_F7	0x41
#define PC_SCANCODE_F8	0x42
#define PC_SCANCODE_F9	0x43
#define PC_SCANCODE_F10	0x44
#define PC_SCANCODE_F11	0xD9
#define PC_SCANCODE_F12	0xDA


// Weird ones; USE AT OWN RISK

#define PC_SCANCODE_NUMLOCK	0x45		// 45 C5
#define PC_SCANCODE_PRINTSCREEN	0x37	// 2A 37 NOT TESTED
#define PC_SCANCODE_SCROLLLOCK	0x46
#define PC_SCANCODE_PAUSE	0x9D		// 1D E0 45 E0 C5 9D NOT TESTED

//=============================================================================
#endif
//=============================================================================

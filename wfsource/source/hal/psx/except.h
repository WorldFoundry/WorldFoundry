//=============================================================================
// except.h
//=============================================================================

// Exception handler for PSX.(Header File)

// Ver		Date		Author			Desc
//----------------------------------------------------------------------------
// 0.1		09/08/95	Brian Marshall	Initial Version.
// 0.2		04-29-97 03:02pm Kevin Seghetti modified to work in PIGS
//=============================================================================

#ifndef _VSDIE_H_
#define _VSDIE_H_

//=============================================================================

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;

typedef signed char		SBYTE;
typedef signed short	SWORD;
typedef signed long		SDWORD;

#undef NULL
#undef FALSE
#undef TRUE

#define NULL			(0)
#define FALSE			(0)
#define TRUE			(!FALSE)

extern void
InstallExceptionHandler(void);

void
_EX_Init(void);

void
_EX_Quit(void);

//=============================================================================
#endif
//=============================================================================

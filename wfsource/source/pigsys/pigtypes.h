#ifndef PIGSYS_PIGTYPES_H
#define PIGSYS_PIGTYPES_H

// Declare all base typedef's the system knows about

#ifndef	_STDDEF_H
#include <cstddef>
#undef	_STDDEF_H
#define	_STDDEF_H
#endif	//defined(_STDDEF_H)

#ifndef	SYS_INT8
#define	SYS_INT8		signed char
#endif	//!defined(SYS_INT8)

#ifndef	SYS_UINT8
#define	SYS_UINT8		unsigned char
#endif	//!defined(SYS_UINT8)

#ifndef	SYS_INT16
#define	SYS_INT16		signed short
#endif	//!defined(SYS_INT16)

#ifndef	SYS_UINT16
#define	SYS_UINT16		unsigned short
#endif	//!defined(SYS_UINT16)

#ifndef	SYS_INT32
#define	SYS_INT32		signed long
#endif	//!defined(SYS_INT32)

#ifndef	SYS_UINT32
#define	SYS_UINT32		unsigned long
#endif	//!defined(SYS_UINT32)

#ifndef	SYS_UCHAR
#define	SYS_UCHAR		unsigned char
#endif	//!defined(SYS_UCHAR)

#ifndef	SYS_USHORT
#define	SYS_USHORT		unsigned short
#endif	//!defined(SYS_USHORT)

#ifndef	SYS_UINT
#define	SYS_UINT		unsigned int
#endif	//!defined(SYS_UINT)

#ifndef	SYS_ULONG
#define	SYS_ULONG		unsigned long
#endif	//!defined(SYS_ULONG)


#ifndef	SYS_SMALLINT
#define	SYS_SMALLINT	int
#endif	//!defined(SYS_SMALLINT)

#ifndef	SYS_LARGEINT
#define	SYS_LARGEINT	long
#endif	//!defined(SYS_LARGEINT)

#if		!SYS_SUPPRESS_BASE_TYPEDEFS
typedef	SYS_INT8		int8;
typedef	SYS_UINT8		uint8;
typedef	SYS_INT16		int16;
typedef	SYS_UINT16		uint16;
typedef	SYS_INT32		int32;
typedef	SYS_UINT32		uint32;
typedef	SYS_UCHAR		uchar;
#endif	//!SYS_SUPPRESS_BASE_TYPEDEFS

#if		!SYS_SUPPRESS_BSD_TYPEDEFS
#if !defined(__PSX__)
typedef SYS_USHORT		ushort;
#endif
typedef SYS_UINT		uint;
typedef SYS_ULONG		ulong;
#endif	//!SYS_SUPPRESS_BSD_TYPEDEFS

typedef	SYS_SMALLINT	smallint;
typedef	SYS_LARGEINT	largeint;

//-----------------02-08-96 04:58pm-----------------
// ANSI standard(looking) Boolean notation added by XINA
//--------------------------------------------------
#if !_BOOL_IS_DEFINED_
#	if defined( __WIN__ ) && defined( __cplusplus )
#		error Use the bool built-in to C++
#	endif
#undef bool
// definition of bool changed in accordance with the ANSI C++ Standard
//#define bool char
// kts so that no clipping occurs
#define bool int
#undef false
#define false 0
#undef true
#define true 1
#endif

#ifndef	SYS_BOOL
#define	SYS_BOOL		bool
#endif	//!defined(SYS_BOOL)

#endif


// pigtool.h: PIGS types compatibility file

#include <limits.h>

#ifndef	_STDDEF_H
#include <stddef.h>
#undef	_STDDEF_H
#define	_STDDEF_H
#endif	/*defined(_STDDEF_H)*/

#ifndef	SYS_INT8
#define	SYS_INT8		signed char
#endif	/*!defined(SYS_INT8)*/

#ifndef	SYS_UINT8
#define	SYS_UINT8		unsigned char
#endif	/*!defined(SYS_UINT8)*/

#ifndef	SYS_INT16
#define	SYS_INT16		signed short
#endif	/*!defined(SYS_INT16)*/

#ifndef	SYS_UINT16
#define	SYS_UINT16		unsigned short
#endif	/*!defined(SYS_UINT16)*/

#ifndef	SYS_INT32
#define	SYS_INT32		signed long
#endif	/*!defined(SYS_INT32)*/

#ifndef	SYS_UINT32
#define	SYS_UINT32		unsigned long
#endif	/*!defined(SYS_UINT32)*/

#ifndef	SYS_UCHAR
#define	SYS_UCHAR		unsigned char
#endif	/*!defined(SYS_UCHAR)*/

#ifndef	SYS_USHORT
#define	SYS_USHORT		unsigned short
#endif	/*!defined(SYS_USHORT)*/

#ifndef	SYS_UINT
#define	SYS_UINT		unsigned int
#endif	/*!defined(SYS_UINT)*/

#ifndef	SYS_ULONG
#define	SYS_ULONG		unsigned long
#endif	/*!defined(SYS_ULONG)*/

#ifndef	SYS_BOOL
#define	SYS_BOOL		int
#endif	/*!defined(SYS_BOOL)*/

#ifndef	SYS_SMALLINT
#define	SYS_SMALLINT	int
#endif	/*!defined(SYS_SMALLINT)*/

#ifndef	SYS_LARGEINT
#define	SYS_LARGEINT	long
#endif	/*!defined(SYS_LARGEINT)*/

#if		!SYS_SUPPRESS_BASE_TYPEDEFS
typedef	SYS_INT8		int8;
typedef	SYS_UINT8		uint8;
typedef	SYS_INT16		int16;
#define INT16_MAX SHRT_MAX
typedef	SYS_UINT16		uint16;
typedef	SYS_INT32		int32;
typedef	SYS_UINT32		uint32;
typedef	SYS_UCHAR		uchar;
#endif	/*!SYS_SUPPRESS_BASE_TYPEDEFS*/

#if		!SYS_SUPPRESS_BSD_TYPEDEFS
typedef SYS_USHORT		ushort;
typedef SYS_UINT		uint;
typedef SYS_ULONG		ulong;
#endif	/*!SYS_SUPPRESS_BSD_TYPEDEFS*/

typedef	SYS_BOOL		Bool;
typedef	SYS_SMALLINT	smallint;
typedef	SYS_LARGEINT	largeint;


typedef int32 fixed32;

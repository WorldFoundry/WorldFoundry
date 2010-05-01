//============================================================================
// endian.h:
//============================================================================

#ifndef	_PIGS_ENDIAN_H
#define	_PIGS_ENDIAN_H

//============================================================================

#ifndef	_PIGSYS_H
#include <pigsys/pigsys.hp>
#endif	//!defined(_PIGSYS_H)

typedef enum
{
	EendianNULL = 0,	// invalid value
	EendianNone,		// none, just a stream of bytes
	EendianBig,			// e.g. Motorolla 68000
	EendianLit,			// e.g. Intel x86
	EendianPDP,			// e.g. PDP11, not now supported
	EendianEND
} tEndianType;


START_EXTERN_C

#if		SYS_TOOL
extern uint32	utl_getLW( sys_FILE * _fp, tEndianType );
extern uint16	utl_getSW( sys_FILE * _fp, tEndianType );
extern void		utl_putLW( sys_FILE * _fp, tEndianType, uint32 l );
extern void		utl_putSW( sys_FILE * _fp, tEndianType, uint16 s );
#endif	//SYS_TOOL
extern uint32	utl_flipLW( uint32 v );
extern uint16	utl_flipSW( uint16 v );
extern uint32	utl_buildLW( uint8 * s, tEndianType );
extern uint16	utl_buildSW( uint8 * s, tEndianType );
extern uint32	utl_xtol( const char * theStr );

END_EXTERN_C

//============================================================================
#endif	//!defined(_ENDIAN_H)
//============================================================================

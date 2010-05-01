//============================================================================
// endian.c:
//============================================================================
#include <pigsys/endian.h>

//============================================================================

#define	BAD_ENDIAN(_e_)	\
	Fail("Invalid or unsupported (" << (int)_e_ << ") endianess specified\n");

// ********************** Misc I/O **********************

#if !defined( __PSX__ )

//============================================================================

uint32
utl_getLW( sys_FILE * fp, tEndianType endian )
{
	uint32 val = 0;

        assert( ValidPtr(fp) );
	if( endian == EendianLit )
	{
		val |= (uint32)fgetc( fp );
		val |= (uint32)fgetc( fp ) << 8;
		val |= (uint32)fgetc( fp ) << 16;
		val |= (uint32)fgetc( fp ) << 24;
	}
	else if( endian == EendianBig )
	{
		val |= (uint32)fgetc( fp ) << 24;
		val |= (uint32)fgetc( fp ) << 16;
		val |= (uint32)fgetc( fp ) << 8;
		val |= (uint32)fgetc( fp );
	}
	else
	{
		BAD_ENDIAN(endian);
	}

	return( val );
}

//============================================================================

uint16
utl_getSW( sys_FILE * fp, tEndianType endian )
{
	uint32 val = 0;

        assert( ValidPtr(fp) );
	if( endian == EendianLit )
	{
		val |= (uint32)fgetc( fp );
		val |= (uint32)fgetc( fp ) << 8;
	}
	else if( endian == EendianBig )
	{
		val |= (uint32)fgetc( fp ) << 8;
		val |= (uint32)fgetc( fp );
	}
	else
	{
		BAD_ENDIAN(endian);
	}

	return( (uint16)val );
}

//============================================================================

void
utl_putLW( sys_FILE * fp, tEndianType endian, uint32 l )
{
        assert( ValidPtr(fp) );
	if( endian == EendianLit )
	{
		fputc( (int)( ( l ) & 0xff ), fp );
		fputc( (int)( ( l >> 8 ) & 0xff ), fp );
		fputc( (int)( ( l >> 16 ) & 0xff ), fp );
		fputc( (int)( ( l >> 24 ) & 0xff ), fp );
	}
	else if( endian == EendianBig )
	{
		fputc( (int)( ( l >> 24 ) & 0xff ), fp );
		fputc( (int)( ( l >> 16 ) & 0xff ), fp );
		fputc( (int)( ( l >> 8 ) & 0xff ), fp );
		fputc( (int)( ( l ) & 0xff ), fp );
	}
	else
	{
		BAD_ENDIAN(endian);
	}
}

//============================================================================

void
utl_putSW( sys_FILE * fp, tEndianType endian, uint16 s )
{
        assert( ValidPtr(fp) );
	if( endian == EendianLit )
	{
		fputc( (int)( ( s ) & 0xff ), fp );
		fputc( (int)( ( s >> 8 ) & 0xff ), fp );
	}
	else if( endian == EendianBig )
	{
		fputc( (int)( ( s >> 8 ) & 0xff ), fp );
		fputc( (int)( ( s ) & 0xff ), fp );
	}
	else
	{
		BAD_ENDIAN(endian);
	}
}

#endif

//============================================================================

uint32
utl_flipLW( uint32 v )
{
	register uint32	val = 0;
	register uint8	tmp;

	tmp = v >> 24;
	val = (uint32)tmp;

	tmp = v >> 16;
	val |= (uint32)tmp << 8;

	tmp = v >> 8;
	val |= (uint32)tmp << 16;

	tmp = v >> 0;
	val |= (uint32)tmp << 24;

	return( val );
}

//============================================================================

uint16
utl_flipSW( uint16 v )
{
	uint32 val = 0;
	register uint8	tmp;

	tmp = v >> 8;
	val = (uint32)tmp;

	tmp = v >> 0;
	val |= (uint32)tmp << 8;

	return( (uint16)val );
}

//============================================================================

uint32
utl_buildLW( uint8 * s, tEndianType endian )
{
	uint32 val = 0;

	assert( s != NULL );
	if( endian == EendianLit )
	{
		val = (uint32)*s++;
		val |= (uint32)*s++ << 8;
		val |= (uint32)*s++ << 16;
		val |= (uint32)*s++ << 24;
	}
	else if( endian == EendianBig )
	{
		val = (uint32)*s++ << 24;
		val |= (uint32)*s++ << 16;
		val |= (uint32)*s++ << 8;
		val |= (uint32)*s++;
	}
	else
	{
		BAD_ENDIAN(endian);
	}

	return( val );
}

//============================================================================

uint16
utl_buildSW( uint8 * s, tEndianType endian )
{
	uint32 val = 0;

	assert( s != NULL );
	if( endian == EendianLit )
	{
		val = (uint32)*s++;
		val |= (uint32)*s++ << 8;
	}
	else if( endian == EendianBig )
	{
		val |= (uint32)*s++ << 8;
		val |= (uint32)*s++;
	}
	else
	{
		BAD_ENDIAN(endian);
	}

	return( (uint16)val );
}

//============================================================================

uint32
utl_xtol( const char * theStr )
{
	uint32	theVal = 0;
	unsigned int		i;

	assert( theStr != NULL );
	AssertMsg( isxdigit( *theStr ), "Non-hex number to utl_xtol" );
	assert( sizeof(theVal) == 4 );
	for ( i = 0; i < sizeof(theVal)*2 && isxdigit( *theStr ); i++, theStr++ )
	{
		theVal <<= 4;
		if( isdigit(*theStr) )
		{
			assert( (int32)(*theStr - '0') >= 0x0 );
			assert( (uint32)(*theStr - '0') <= 0x9u );
			theVal |= (uint32)(*theStr - '0') & 0x0f;
		}
		else
		{
			assert( (uint32)(tolower(*theStr) - 'a' + 10) >= 0xAu );
			assert( (uint32)(tolower(*theStr) - 'a' + 10) <= 0xFu );
			theVal |= (uint32)(tolower(*theStr) - 'a' + 10) & 0x0f;
		}
	}
	AssertMsg( i < sizeof(theVal)*2 || !isxdigit( *theStr ),
		"utl_xtol given to large a number" );
	return( theVal );
}

//============================================================================

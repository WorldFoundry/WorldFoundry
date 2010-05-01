
#include <pigsys/pigsys.hp>

void
strlwr( char* string )
	{
	assert( ValidPtr( string ) );
	while ( NULL != ( *string++ = (char)tolower( *string ) ) )
		;
	}

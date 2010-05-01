
#include <pigsys/pigsys.hp>

void
strlwr( char* string )
	{
	assert( ValidPtr( string ) );
	while ( 0 != ( *string++ = (char)tolower( *string ) ) )
		;
	}

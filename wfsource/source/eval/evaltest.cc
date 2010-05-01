// eval.cc

#include <string.h>
#include <stdio.h>
#include <eval/eval.h>

double
SymbolLookup( const char* szSymbolName )
{
	if ( strcmp( szSymbolName, "pi" ) == 0 )
		return 3.1415927;
	else if ( strcmp( szSymbolName, "e" ) == 0 )
		return 2.17;
	else
		return 0.0;
}


int
main( int argc, char* argv[] )
{
	int i;

	if ( argc == 1 )
		printf( "Usage: eval expression...\n" );

	for ( i=1; i<argc; ++i )
	{
		const char* szExpression = argv[ i ];
		float val = eval( szExpression, SymbolLookup );
		printf( "%s = %g (0x%lx)\n", szExpression, val, int( val ) );
	}

	return 0;
}

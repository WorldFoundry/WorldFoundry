//==============================================================================
// eval.cc: Copyright (c) 1996-1999, World Foundry Group  
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org
//==============================================================================

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

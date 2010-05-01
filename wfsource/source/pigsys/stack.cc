//==========================================================================
// stack.cc:
// Copyright (c) 1994,95,96,97,98,99 World Foundry Group  
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

// ===========================================================================
// Original Author: William B. Norris IV
// ===========================================================================

#define _SYS_NOCHECK_DIRECT_STD
#include <pigsys/pigsys.hp>

#define STACK_CALL_MAX_DEPTH	1024

static int call_depth = 0;
char* callStack[ STACK_CALL_MAX_DEPTH ];

//==============================================================================

void
addToCallList( const char* pRoutineName )
{
	if ( call_depth == STACK_CALL_MAX_DEPTH )
	{
		fprintf( stderr, "Overflow stack call tracer\n" );
		exit( 1 );
	}
	callStack[ call_depth++ ] = pRoutineName;
}

//==============================================================================

void
removeFromCallList( void )
{
	--call_depth;
	if ( call_depth < 0 )
	{
		fprintf( stderr, "Underflow stack call tracer\n" );
		exit( 1 );
	}
}

//==============================================================================

void
printCallStack( void )
{
	if ( call_depth > 0 )
	{
		int i;

    	puts( "ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´" );
    	puts( "³Call stack                                                                   ³" );
		for ( i=call_depth-1; i>=0; --i )
		{
			char szRoutineName[ 256 ];
			char* pRoutineName = callStack[ i ];

			strncpy( szRoutineName, pRoutineName-*pRoutineName, *pRoutineName );
			szRoutineName[ *pRoutineName ] = '\0';

			printf( "³%2d  %s()%*s³\n", i, szRoutineName, 70-strlen( szRoutineName ), "" );
		}
    	puts( "ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ" );
	}
}

//==============================================================================

//==============================================================================
// main.cc: Copyright (c) 1996-1999, World Foundry Group  
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

#include <cstring>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <fstream>

#include "aitoken.h"
#include "airun.hp"

extern char szVersion[];

void airun( int32* );
long ffilesize( FILE* );

#if defined( DEBUG )
void
print_stack( int32* pScript, int nTokens )
{
	int32* _pScript = pScript;

	printf( "Stack: [%d]: ", nTokens );
	for ( int i=0; i<nTokens; ++i )
		printf( "%lx ", *_pScript-- );
}
#endif


int32
read_mailbox( int32 nMailbox )
{
	return ( rand() % 500 ) << 15;
}


int
main( int argc, char* argv[] )
{
	if ( argc <= 1 )
	{
		cerr << "airun v" << szVersion
			<< "  Copyright 1996-1997 World Foundry Group." << endl;
		cerr << "By William B. Norris IV" << endl;
		cerr << "\nUsage: airun <filename>.s" << endl;
		return 10;
	}

	FILE* fp = fopen( argv[1], "rb" );
	assert( fp );

	int32 sizeOfFile = ffilesize( fp );
	assert( (sizeOfFile & 3) == 0 );
	int32* dataFile = (int32*)malloc( sizeOfFile );
	int nBytesRead = fread( dataFile, 1, sizeOfFile, fp );
	assert( nBytesRead == sizeOfFile );

	fclose( fp );

	airun( dataFile );

	return 0;
}

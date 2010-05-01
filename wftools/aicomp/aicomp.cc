//==============================================================================
// aicomp.cc: Copyright (c) 1996-1999, World Foundry Group  
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
#include <iostream>

#include <version/version.hp>

extern int aicomp( const char* _szInputFile );

bool bVerbose = true;
bool bQuiet;
extern int yydebug;			/*  nonzero means print parse trace	*/

//char* szInputFile;

int
main( int argc, char* argv[] )
{
	char* _szInputFile = NULL;

	for ( ++argv; *argv; ++argv )
	{
		if ( strcmp( *argv, "-v" ) == 0 )
        {
			bVerbose = true;
            yydebug = 1;
        }
		else if ( strcmp( *argv, "-q" ) == 0 )
			bQuiet = true;
		else
			_szInputFile = *argv;
	}

	if ( !_szInputFile )
	{
		std::cerr << "aicomp v" << szVersion << "  Copyright 1996,97,98,99,2000,2001 World Foundry Group." << std::endl;
		std::cerr << "By William B. Norris IV" << std::endl;
		std::cerr << "\nUsage: aicomp [-v] [-q] <filename>.c | .scm" << std::endl;
		return -10;
	}

	int nErrorCode = aicomp( _szInputFile );

	if ( nErrorCode )
	{
		std::cout << "Press ENTER to continue...";
		char szLine[ 1 ];
		std::cin.read( szLine, sizeof( szLine ) );
	}

	return nErrorCode;
}

/* ===========================================================================
 * menutest.cc
 * by William B. Norris IV
 * Copyright (c) 1998,2000,2003 World Foundry Group.  
 *
 * // Part of the World Foundry 3D video game engine/production environment
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

 * ===========================================================================
 */

#include <menu/menu.hp>
#include <cpplib/range.hp>
#include <gfx/tga.hp>

#define MAX_COMMAND_LINES 2

#if 0
char* commandLines[MAX_COMMAND_LINES] =
{
	"-rate20 1",
	"-rate20 -sfs -sSs -sss -pds -pws -srs 1",
	"-rate20 0",
	"-rate20 -srs 0",
};
char* descStrings[MAX_COMMAND_LINES] =
{
	"&Cyberthug",
	"&Snow Goons",
	"&Minecart",
	"&Physics"
};
#else
char* commandLines[MAX_COMMAND_LINES] =
{
	"Abort the program",
	"Ignore the error and continue [dangerous!]",
};
char* descStrings[MAX_COMMAND_LINES] =
{
	"&Abort",
	"&Ignore"
};
#endif


int
main( int argc, char* argv[] )
{
	for ( int i=0; i<MAX_COMMAND_LINES; ++i )
	{
//		pictures[ i ] = new Pixelmap( tga_pictures[ i ] );
//		assert( pictures[ i ] );
	}

	Range_Wrap range( 0, MAX_COMMAND_LINES-1 );
//      return SimpleMenu( "World Foundry[TM] Level Loader", "Command Line", descStrings, commandLines, range, 0 );
        return SimpleMenu( NULL, NULL, descStrings, commandLines, range, 0 );
}

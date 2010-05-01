//==============================================================================
// beep.cc: Copyright (c) 1996-1999, World Foundry Group  
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

#if defined( MSDOS )
#	include <dos.h>
#elif defined( _WIN32 )
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#else
#	include <stdio.h>
#endif

//==============================================================================

void
beep()
	{
#if defined( MSDOS )
    sound (440);
    delay (25);
    nosound ();
    delay (25);
    sound (660);
    delay (25);
    nosound ();
    delay (25);
    sound (440);
    delay (25);
	nosound ();
#elif defined( _WIN32 )
	Beep( 440, 1000 );
#else
	putchar( '\a' );
#endif
	}

//==============================================================================

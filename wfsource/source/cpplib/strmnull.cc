//=============================================================================
// strmnull.cc
// Copyright 1997,1999,2000,2002 World Foundry Group. 
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

#include <cpplib/strmnull.hp>
#include <pigsys/assert.hp>

#if defined(OLD_IOSTREAMS)
#    if defined (__WIN__)

// ===========================================================================

nullstreambuf::nullstreambuf( char* s, int n ) : streambuf()
{
    setb( s, s+n );
	setp( base(), ebuf() );
}

// ===========================================================================

nullstreambuf::~nullstreambuf()
{
	sync();
}

// ===========================================================================

int
nullstreambuf::overflow( int nChar )
{
	sync();
	return nChar != EOF ? sputc( nChar ) : nChar;
}

// ===========================================================================

int
nullstreambuf::sync()
{
	if ( out_waiting() )
		setp( base(), ebuf() );

	return 0;
}

// ===========================================================================

nullstream::nullstream()
{
	strbuf = new nullstreambuf( msgs, bsize );
	assert( strbuf );
	ostream::init( strbuf );
	setf( ios::unitbuf );
}

// ===========================================================================

nullstream::~nullstream()
{
	assert( strbuf );
	delete strbuf;
}
#    endif              // defined(__WIN__)                                 

//==============================================================================

#    if defined(__LINUX__)
// strmnull.cc

#include <iostream.h>

//#include <pclib/compat.hp>

#include "strmnull.hp"

//nullstreambuf::nullstreambuf( char* s, int n ) : streambuf( s, n )
nullstreambuf::nullstreambuf( char* s, int n ) : streambuf()
	{
    setb( s, s+n );

	setp( base(), ebuf() );
	}


nullstreambuf::~nullstreambuf()
	{
	sync();
	}


int
nullstreambuf::overflow( int nChar )
	{
	sync();
	if ( nChar != EOF )
		return sputc( nChar );
	else
		return nChar;
	}


int
nullstreambuf::sync()
	{
	if ( out_waiting() )
		setp( base(), ebuf() );

	return 0;
	}


nullstream::nullstream()
	{
	strbuf = new nullstreambuf( msgs, bsize );
	ostream::init( strbuf );
	setf( ios::unitbuf );
	}


nullstream::~nullstream()
	{
	delete strbuf;
	}


//nullstream cnull;				// Create a global instance of the output

#    endif         // defined(__LINUX__)

#else          // defined(OLD_IOSTREAMS)

#    if defined(__LINUX__)
   nullstreambuf globalnullstreambuf;
   std::ostream globalnullstream(&globalnullstreambuf);
#    endif

#endif		// defined(OLD_IOSTREAMS)

// ===========================================================================

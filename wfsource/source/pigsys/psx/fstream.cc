//=============================================================================
// fstream.cc:
// Copyright ( c ) 1998,99 World Foundry Group  
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

//=============================================================================
// Description:
//		Here is my first stab at a simple ostream (kts)
//
// Original Author: Brad McKee
// Since then: Kevin T. Seghetti
// ------------------------------------------------------------------------

#include <pigsys/pigsys.hp>
#include <iostream>
#include <_iostr.hp>
#include <pigsys/genfh.hp>
#include <hal/hal.h>
//#include <visual/validate.hp>
//#include <stream/str_type.hp>
#include <libgte.h>
#include <libgpu.h>

// used by screen_flush
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>

#if DO_IOSTREAMS

#if DO_DEBUG_FILE_SYSTEM			// !defined (SW_STREAMING )
//=============================================================================

ofstream::ofstream( const char * filename ) :
//ostream(*NEW(StreamOutput_file(filename)))
ostream(* new (HALLmalloc) StreamOutput_file(filename))
{
}

//=============================================================================

StreamInput_file::StreamInput_file(const char* filename)
{
	_fh = FHOPENRD( filename );
	if( _fh == -1 )
	{
		cerr << "Problems opening ifstream " << filename << endl;
		exit( -1 );
	}
}

//-----------------------------------------------------------------------------

StreamInput_file::~StreamInput_file(const char* filename)
{
	assert( _fh  >= 0);
	FHCLOSE( _fh );
}

//-----------------------------------------------------------------------------

void
StreamInput_file::InputChars(char* string, int length)
{
	assert(ValidPtr(string));
	assert(_fh >= 0);
	ASSERTIONS( int read = ) FHREAD( _fh, string, length );
	AssertMsg( read == length, "Read from hard disk failed!" );
}

//-----------------------------------------------------------------------------

ifstream::ifstream( const char * filename ) :
//	istream(*NEW(StreamInput_file(filename)))
	istream(* new (HALLmalloc) StreamInput_file(filename))
{
//	printf("constructing ostream at address %p from StreamOutput at address %p\n",this,&output);
	Validate();
}

//=============================================================================
//=============================================================================

StreamOutput_file::StreamOutput_file(const char* filename)
{
	_fh = FHOPENWR( filename );
	if( _fh == -1 )
	{
		cerr << "Problems opening ostream " << filename << endl;
		exit( -1 );
	}
}

//-----------------------------------------------------------------------------

StreamOutput_file::~StreamOutput_file(const char* filename)
{
	assert( _fh  >= 0);
	FHCLOSE( _fh );
}

//-----------------------------------------------------------------------------

void
StreamOutput_file::OutputChars(const char* string, int length)
{
	assert(string[length] == 0);
	assert(_fh >= 0);
   	ASSERTIONS( int written = ) FHWRITE( _fh, string, length );
	AssertMsg( written == length, "Write to hard disk failed!" );
}

#endif	// DO_DEBUG_FILE_SYSTEM
#endif	// DO_IOSTREAMS

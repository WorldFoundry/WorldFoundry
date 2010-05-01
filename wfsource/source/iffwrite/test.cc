//=============================================================================
// test.cc
// Copyright 1997,99 World Foundry Group. 
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
// History:
// ?? ??? ??    WBNIV   Created
//============================================================================

#include <pigsys/assert.hp>
#include <math/scalar.hp>
#include <iffwrite/iffwrite.hp>
#include <time.h>
#include <fstream>
#include <strstream>

static void
writeFile( _IffWriter& wr )
{
	srand( time( NULL ) );

	wr.enterChunk( ID( "\\\\" ) );
		wr << "\\";
	wr.exitChunk();
	wr.enterChunk( ID( "\\\\\\\\" ) );
		wr << "\\\\";
	wr.exitChunk();
	wr.enterChunk( ID( "\\\\\\\\" ) );
		wr << "Tab=\\t Backslash=\\\\ Newline=\\n";
	wr.exitChunk();

	wr.enterChunk( ID( " id " ) );
		wr << ID( " id " );
	wr.exitChunk();

	istrstream stream( "Test" );
	wr << stream;

	istrstream stream20( "Test", 20 );
	wr << stream20;

	wr.enterChunk( ID( "FILE" ) );

		wr.enterChunk( ID( "file" ) );
			wr << 0UL;
			wr << File( "test.cc" );
		wr.exitChunk();

		wr.enterChunk( ID( "byte" ) );
			wr << (unsigned char)( 1 );
			wr << (unsigned char)( 1 );
			wr << (unsigned char)( 1 );
			wr << (unsigned char)( 1 );
			wr << (unsigned char)( 1 );
			wr << (unsigned char)( 1 );
		wr.exitChunk();

		{
		const ChunkSizeBackpatch* cs = wr.findSymbol( "::'FILE'::'byte'" );
		assert( cs );
		}

		{
		const ChunkSizeBackpatch* cs = wr.findSymbol( "::'FILE'::'byte'" );
		assert( cs );
		}

		wr.enterChunk( ID( "word" ) );
			wr << (unsigned short)( 2 );
		wr.exitChunk();

		wr.enterChunk( ID( "long" ) );
			wr << 0UL;
		wr.exitChunk();

		wr.enterChunk( ID( "pstr" ) );
//                      wr.out_string( "test", 5 );
			std::string str( "This ia a C++ string" );
			wr << str;
		wr.exitChunk();

#if 1
		const char szString[] = "This is the first line";
#else
		const char szString[] = "This is the first line\n"
			"This line has a \" in it\n"
			"The third line also has a \" in it\n"
			"[fourth]\n"
			"Last and fifth line\n";
#endif
		wr.enterChunk( ID( "strn" ) );
			wr.out_string( szString );
		wr.exitChunk();

		wr.enterChunk( ID( "fixp" ) );
			for ( int i=0; i<1000; ++i )
			{
				size_specifier ss = { 1, 15, 16 };
				wr << Fixed( ss, 3.0 );
				wr << Fixed( ss, 2.1 );
				Scalar fx( SCALAR_CONSTANT( 42 ) );
				wr << fx;
				wr << Comment( "" );
			}
		wr.exitChunk();

		//wr.enterChunk( ID( 0 ) );
			wr << Comment( "FUCK THIS!" );
		//wr.exitChunk();

		wr.enterChunk( ID( "mem" ) );
			strstream mem( "1234567890", 20, ios::in | ios::out );
			wr << mem;
//                      wr.out_mem( "1234567890", 5 );
		wr.exitChunk();

		wr.enterChunk( ID( "time" ) );
			Timestamp ts;
			wr << ts;
		wr.exitChunk();

		PrintF x( "This is some text with %d, %d, and %d [%s]", 2, 3, 5, "substitutions" );
		wr << Comment( x() );

	wr.exitChunk();
}


static void
usage()
{
	std::cout << "Usage: test.exe [-binary] [-ascii] [-v] [-q]" << std::endl;
}


int
main( int argc, char* argv[] )
{
	bool bBinary = false;
	bool bVerbose = false;

	for ( ++argv; *argv; ++argv )
	{
		if ( strcmp( *argv, "-binary" ) == 0 )
			bBinary = true;
		else if ( strcmp( *argv, "-ascii" ) == 0 )
			bBinary = false;
		else if ( strcmp( *argv, "-v" ) == 0 )
			bVerbose = true;
		else if ( strcmp( *argv, "-q" ) == 0 )
			bVerbose = false;
		else if ( strcmp( *argv, "-?" ) == 0 )
		{
			usage();
			return 10;
		}
	}

	ofstream* out = new ofstream( "test.iff", ios::out | ios::binary );
	assert( out );

	_IffWriter* iffWriter;
	if ( bBinary )
		iffWriter = new IffWriterBinary( *out );
	else
		iffWriter = new IffWriterText( *out );
	assert( iffWriter );
	if ( bVerbose )
		iffWriter->log( cout );

	writeFile( *iffWriter );
	//writeFile( theNullIffWriter );

	delete iffWriter;

	delete out;

	return 0;
}


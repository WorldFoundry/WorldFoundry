// savetest.cc
//
// Sample application to test the "savegame" library
//
// by William B. Norris IV
// Copyright 1997,99 World Foundry Group.  

#include <cpplib/scalar.hp>
#include <savegame/savegame.hp>
#include "version.h"

ostream cmem( NULL );
char szGameName[] = "savetest v1.0";

void
PIGSMain( int argc, char* argv[] )
{
	cout << "savegame library test program" << endl;

	{
	saveostream game( "savetest", 8192+55, 128 );		// to simulate PlayStation
	assert( game.good() );

	int32 i32 = 10;
	game << i32;

	game << "This is a test";
	game << i32;
	game << i32;
	game << i32;

	game << Scalar::negativeOne;
	for ( int i=0; i<300; ++i )
		game <<  Scalar(i,0);
	game << Scalar::one;

	// Test buffer overflow
	game << "01234567890123456789012345678901234567890123456789012345678901234567890123456789"
		"01234567890123456789012345678901234567890123456789012345678901234567890123456789"
		"01234567890123456789012345678901234567890123456789012345678901234567890123456789";
	}

	{
	saveistream game( "savetest", 128 );
	assert( game.good() );

	int32 i32;
	char string[ 512 ];
	Scalar scalar;

#define READ( var ) 	game >> var, cout << "Read " << dec << var << endl;

	READ( i32 );
	READ( string );
	READ( i32 );
	READ( i32 );
	READ( i32 );

	READ( scalar );
	for ( int i=0; i<300; ++i )
		READ( scalar );
	READ( scalar );

	READ( string );
	}

	// Need this to prevent lots of linker errors--why?
	cout << Scalar::zero << endl;
}

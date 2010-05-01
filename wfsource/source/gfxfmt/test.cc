// test.cc

// by William B. Norris IV
// Copyright 1998,99 World Foundry Group.  

#include <cassert>
#include <gfxfmt/tga.hp>
#include <gfxfmt/bmp.hp>
#include <gfxfmt/sgi.hp>

bool bDebug = true;
bool bVerbose = true;
uint16 colTransparent = 0;

void
usage()
{
	cout << "Usage: test <imagemap>.{bmp|tga|sgi}" << endl;
}


int
main( int argc, char* argv[] )
{
	if ( argc != 2 )
	{
		usage();
		return 10;
	}

	cout << "gfxfmt test program" << endl;

	ifstream* input = new ifstream( argv[ 1 ], ios::binary );
	assert( input );
	assert( input->good() );


	#pragma message( "TODO: Can filename be read from i[f]stream?" )
	Bitmap* image;

	if ( 0 )
		;
	else if ( image = new SgiBitmap( *input, argv[ 1 ] ) )
		;
	else if ( image = new WindowsBitmap( *input, argv[ 1 ] ) )
		;
	else if ( image = new TargaBitmap( *input, argv[ 1 ] ) )
		;

	delete input;

	if ( !image )
	{
		cerr << "Couldn't decode image " << argv[ 1 ] << endl;
		cerr << "[currently understandad Windows and Targa file formats" << endl;
		return 10;
	}

	assert( image );

	TargaBitmap* out_image = new TargaBitmap( *image );
	assert( out_image );
	ofstream output( "out.tga", ios::binary );
	assert( output.good() );
	out_image->save( output );
	delete out_image;

	delete image;

	return 0;
}

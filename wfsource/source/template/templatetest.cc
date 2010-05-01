////////////////////////////////////////////////////////////////////////////////
//
// By William B. Norris IV
// Copyright 1995,1996 Cave Logic Studios, Ltd.  
//
// v0.0.1	12 Dec 96	Created
//
////////////////////////////////////////////////////////////////////////////////

#include <pigsys/assert.hp>

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
using namespace std;

#include <template/template.hp>
#include <loadfile/loadfile.hp>

ofstream* templatelog;

#include <version/version.hp>

void
usage( int argc, char* argv[] )
{
//	cout << "template v" << szVersion << " Copyright 1996,1998 William B. Norris IV.  " << endl;
	cout << "template v" << " Copyright 1996,1998 William B. Norris IV.  " << endl;
	cout << "by William B. Norris IV" << endl;
	cout << "Expand a template file by replacing fields with variables in a .set file" << endl;
	cout << endl;
	cout << "Usage: template templateFile MACRO=value..." << endl;
}


#if 0
void
shutdown()
{
	//mc_endcheck();
}
#endif


std::vector<Symbol> tblSymbols;

void
parseMacro( const string line )
{
	size_t pos_equal = line.find( '=' );
	assert( pos_equal < line.length() );

	string sSymbol( line, 0, pos_equal );
	string sValue( line, pos_equal+1, -1 );
	//cout << sSymbol << " = " << sValue << endl;

	Symbol* sym = new Symbol( sSymbol, sValue );
	assert( sym );
	assert( find( tblSymbols.begin(), tblSymbols.end(), *sym ) == tblSymbols.end() );
	tblSymbols.push_back( *sym );
}


int
main( int argc, char* argv[] )
{
	//mc_startcheck( NULL );
	//atexit( shutdown );

	if ( argc < 3 )
	{
		usage( argc, argv );
		return -1;
	}

	templatelog = new ofstream( "template.log" );
	assert( templatelog );
	assert( templatelog->good() );

	int32 cbFile;
	char* szFile = LoadTextFile( argv[1], cbFile );
	string szTemplate( szFile );

//	string vars = LoadTextFile( argv[2] );

	for ( int idxArg=2; idxArg<argc; ++idxArg )
	{
//		cout << idxArg << " = " << argv[ idxArg ] << endl;
		string line( argv[ idxArg ] );
		parseMacro( line );
	}


	// keep around in case I add a switch to parse a set file
#if 0
	{ // Parse set file
		size_t old_pos = 0;
		size_t pos = 0;

		do
		{
			pos = vars.find( '\n', old_pos );
			if( pos == -1 ) break;
			string line( vars, old_pos, pos-old_pos-1 );
			cout << line << endl;

			parseMacro( line );

			old_pos = pos + 1;
		}
		while ( pos < vars.length() );
	}
#endif

	string szOutput = _template( szTemplate, tblSymbols );
	// I know this is silly...
//	if ( argc == 4 )
//		WriteTextFile( argv[3], szOutput.c_str() );
//	else
		cout << szOutput.c_str() << endl;

	assert( templatelog );
	delete templatelog;

	return 0;
}

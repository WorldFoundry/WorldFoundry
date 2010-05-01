//==============================================================================
// link.cc
// Copyright (c) 1997-1999, World Foundry Group  
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

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <pigsys/assert.hp>

#include <string>
#include <vector>
#include <algorithm>

extern char szVersion[];

#include "util.hp"
#include "dumpobj.hp"

void usage();
void printname( char* & );
void printpatch( char* &, unsigned long address = 0UL );
void dumpcode( char* &, long int, unsigned long );

#include "symbol.hp"
#include "fn.hp"
#include "patch.hp"
#include "section.hp"
#include "code.hp"
#include "objfile.hp"

vector<Function> lstGlobals;
vector<ObjectFile> lstObjectFiles;
bool bPrint;
bool bPrintAll;

int link( int argc, char* argv[] );
void _link( const char* szFilename );

int
main( int argc, char* argv[] )
{
	if( argc == 1 || *argv[1]=='?' || (*argv[1]=='/' && *(argv[1]+1)=='?') )
	{
		usage();
		exit( 0 );
	}

	return link( argc, argv );
}


int
link( int argc, char* argv[] )
{
	for ( int i=1; i<argc; ++i )
	{	// Verify file
		char* szFilename = argv[ i ];
		int cbData;
		void* data = LoadBinaryFile( szFilename, cbData );
		if( !data )
		{
			printf( "Error : unable to open file \"%s\" for input - exiting\n", szFilename );
			exit( 1 );
		}

		ObjectFile of( szFilename, data, cbData );
		lstObjectFiles.push_back( of );
		ObjectFile* first = lstObjectFiles.begin();
		cout << first << endl;
	}

	for ( int nPass = 0; nPass < 2; ++nPass )
	{
		vector<ObjectFile>::iterator of;

		for ( of = lstObjectFiles.begin(); of != lstObjectFiles.end(); ++of )
			of->link( nPass );

		if ( nPass == 0 )
		{
			const int maxSection = 9;

			for ( int nSection=1; nSection<=maxSection; ++nSection )
			{
				unsigned long address = 0UL;
				for ( of = lstObjectFiles.begin(); of != lstObjectFiles.end(); ++of )
					of->relocate( nSection, address );
			}

			for ( of = lstObjectFiles.begin(); of != lstObjectFiles.end(); ++of )
				of->applyPatches();
		}
	}

	{ // Write output to disk
	FILE* fp = fopen( "test.bin", "wb" );
	assert( fp );

	vector<ObjectFile>::iterator of;
	for ( of = lstObjectFiles.begin(); of != lstObjectFiles.end(); ++of )
		of->writeExe( fp );

	fclose( fp );
	}

	return 0;
}


void
ObjectFile::relocate( int nSection, unsigned long& addr )
{
	vector<Code>::iterator code;
	for ( code = lstCode.begin(); code != lstCode.end(); ++code )
	{
		if ( code->section() == nSection )
		{
			code->offset( addr );
			cout << "Relocating code " << code << " in section " << nSection << " size=" << code->size() << " offset=" << code->offset() << endl;
			addr += code->size();
		}
	}
}


void
ObjectFile::applyPatches()
{
	vector<Patch>::iterator patch;
	for ( patch = patches.begin(); patch != patches.end(); ++patch )
		patch->apply();
}


void
ObjectFile::writeExe( FILE* fp )
{
	vector<Code>::iterator code;
	for ( code = lstCode.begin(); code != lstCode.end(); ++code )
	{
		int cbWritten = fwrite( code->data(), 1, code->size(), fp );
		assert( cbWritten == code->size() );
	}
}


void
ObjectFile::link( int nPass )
{
	_nPass = nPass;

	int ch = 0;
	long int i1, i2, i3, linenum;

	bPrint = ( pass() == 1 );
	bPrintAll = bPrint && 0;
	bPrintAll = bPrint && 1;

	assert( pEndData - pStartData == cbData );
	bool inloop;
	char* data;
	for ( inloop = true, data = pStartData+4; data < pEndData && inloop; )
	{
		ch = *data++;
		switch ( ch )
		{
			case 0:		// END OF FILE
				cout << "-------------------------------------------------------------------------------" << endl;
				assert( data == pEndData );
				break;

			case 2:		// CODE
			{
				int cbCode = i1 = readw( data );
				pCode = data;
				pSectionData = data;
				Symbol& section = sections[ currentSection ];
				if ( pass() == 0 )
				{
					Code code( data, section.address(), cbCode, currentSection );
					lstCode.push_back( code );
					data += i1;
				}
				else
				{
					cout << "Code " << i1 << " bytes (section \'" << section.name() << "\') @ "
						<< hex << section.address() << endl;
					assert( section.value() >= 0 );

					dumpcode( data, i1, section.address() );
				}
				section.address( section.address() + i1 );
				break;
			}

			case 6:		// SWITCH TO SECTION
			{
				currentSection = i1 = readw( data );
				Section& section = sections[ currentSection ];
				assert( section.value() == i1 );
				if ( bPrint )
					cout << "Switch to section " << i1 << " \"" << section.name() << '\"' << endl;
				break;
			}

			case 14:	// XREF
			{
				i1 = readw( data );
				Function newFunction( data, i1, currentSection, i1, false );
				//Function newFunction( data, i1, currentSection, i1, true );

				if ( pass() == 0 )
				{
					lstGlobals.push_back( newFunction );
				}
				else
				{
					if ( bPrintAll )
					{
						const Function* function = find( lstGlobals.begin(), lstGlobals.end(), newFunction );
						assert( function != lstGlobals.end() );

						assert( function->value() == i1 );
						cout << "14: XREF symbol number " << function->value() <<
							" \'" << function->name().c_str() << "\'" << endl;
					}
				}
				break;
			}

			case 12:	// XDEF
			{
				int value = readw( data );
				int section = readw( data );
				long offset = readl( data );

				if ( pass() == 0 )
				{
					Function fnToFind( data, value, section, offset, true );
					SymbolByName toFind( fnToFind );
					Function* found = find_if( lstGlobals.begin(), lstGlobals.end(), toFind );
					if ( found == lstGlobals.end() )
					{	// new symbol -- add to list
						//Function function( data, value, section, offset );
						lstGlobals.push_back( fnToFind );
					}
					else
					{	// existing symbol -- update
						found->offset( 0xFFFFFF );
					}
				}
				else
				{
					Function toFind( data, value, section, offset );
					if ( bPrintAll )
					{
						const Function* function = find( lstGlobals.begin(), lstGlobals.end(), toFind );
						assert( function != lstGlobals.end() );
						cout << "XDEF symbol number " << function->value() << " \'" << function->name()
							<< "\' at offset " << function->offset() << " in section " << function->section() << endl;
					}
				}
				break;
			}

			case 16:	// DEFINE SECTION
			{
				i1 = readw( data );
				i2 = readw( data );
				ch = *data++;

				if ( pass() == 0 )
				{
					Section section( data, i1 );
					section.alignment( int( ch ) );
					while ( sections.size() < i1 )
					{
						Section sectionPlaceholder( string( "invalid section (placeholder)" ), ~0 );
						sections.push_back( sectionPlaceholder );
					}

					sections.push_back( section );
					//sections.reserve( i1+1 );
					//sections[ i1 ] = section;
					int i = sections.end() - sections.begin() - 1;
					i = sections.size();
					assert( section.value() == sections.size() - 1 );
				}
				else
				{	// reset section back to 0 for second pass
					bPrint && printf( "Section symbol number %lx \"", i1 );
					data += 1 + *data;
					Symbol& section = sections[ i1 ];
					assert( section.value() == i1 );
					section.address( 0UL );
					cout << section.name();
					bPrint && printf( "\" in group %lx alignment %d\n", i2, ch );
			}

				break;
			}

			case 18:	// LOCAL SYMBOL
			{
				i1 = readw(data);
				i2 = readl(data);
				Function function( data, -1, i1, i2 );	//section, offset );
				if ( pass() == 0 )
				{
					functions.push_back( function );
				}
				else
				{
					if ( bPrintAll )
					{
						const Function* foundFunction = find( functions.begin(), functions.end(), function );
						assert( foundFunction != functions.end() );

						cout << "Local symbol \'" << foundFunction->name()
							<< "\' at offset " << foundFunction->offset() << " in section "
							<< foundFunction->section() << endl;
					}
				}
				break;
			}

			case 10:	// PATCH
			{
				assert( pCode );
				ch = *data++;
				i1 = readw( data );
				bPrintAll && printf( "Patch type %d at offset %lx with ", ch, i1 );
				if ( pass() == 0 )
				{
					Patch patch( i1, data, ch, pSectionData, this, currentSection );
					patches.push_back( patch );
				}
				printpatch( data, i1 );
				bPrintAll && printf( "\n" );
				break;
			}

#if 0
			case 8:		// UNITIALIZED DATA
			{
				i1 = readl( data );
				bPrintAll && printf( "8: Uninitialised data, %ld bytes\n", i1 );
				break;
			}
#endif

			// Unused information
			case 28:
				bPrint && printf( "28: Define file " );
				i1 = readw( data );
				bPrint && printf( "number %x as \"", i1 );
				printname( data );
				bPrint && printf( "\"\n" );
				break;

			case 46:
				ch = *data++ & 0xFF;
				bPrintAll && printf( "46: Processor type %d\n", ch );
				break;


			// Debugging chunks, currently not supported
			case 50:
			case 52:
			case 54:
			case 56:
			case 58:
			case 60:
				cout << "Unsupported debugging chunk $" << hex << int( ch ) << endl;
				exit( -1 );
				break;

			// "DON'T CARE" CHUNKS
			case 4:
			case 8:
			case 20:
			case 22:
			case 24:
			case 26:
			case 30:
			case 32:
			case 34:
			case 36:
			case 38:
			case 40:
			case 42:
			case 44:
			case 48:
			case 62:
			case 64:
			case 66:
			case 68:
			case 70:
			case 72:
			case 78:
			case 80:
			case 82:
			case 84:
			default:
				cout << "Unsupported chunk $" << hex << int( ch ) << endl;
				exit( -1 );
		}
	}
}


long int
readl( char* & data )
{
	unsigned long int v = 0UL;
	unsigned char ch;

	ch = *data++; v += ch;
	ch = *data++; v += (ch << 8);
	ch = *data++; v += (((unsigned long int) ch) << 16);
	ch = *data++; v += (((unsigned long int) ch) << 24);
	return(v);
}


long int
readw( char* & data )
{
	unsigned long int v = 0UL;
	unsigned char ch;

	ch = *data++; v += ch;
	ch = *data++; v += (ch << 8);
	return(v);
}


void
printname( char* & data )
{
	int len;

	len = *data++;
	while ( len-- )
	{
		if ( bPrint )
			putchar( *data++ );
		else
			++data;
	}
}


void
printpatch( char* & data, unsigned long address )
{
	int op;
	long int i1;

	op = *data++;
	switch( op )
	{
		case 0 :
			i1 = readl(data);
			bPrintAll && printf("$%lx", i1);
			return;

		case 2:
		{
			i1 = readw(data);
			bPrintAll && printf("[%lx]", i1);
			return;
		}

		case 4 :
			i1 = readw(data);
			bPrintAll && printf("sectbase(%lx)", i1);
			return;

		case 6 :
			i1 = readw(data);
			bPrintAll && printf("bank(%lx)", i1);
			return;

		case 8 :
			i1 = readw(data);
			bPrintAll && printf("sectof(%lx)", i1);
			return;

		case 10 :
			i1 = readw(data);
			bPrintAll && printf("offs(%lx)", i1);
			return;

		case 12 :
			i1 = readw(data);
			bPrintAll && printf("sectstart(%lx)", i1);
			return;

		case 14 :
			i1 = readw(data);
			bPrintAll && printf("groupstart(%lx)", i1);
			return;

		case 16 :
			i1 = readw(data);
			bPrintAll && printf("groupof(%lx)", i1);
			return;

		case 18 :
			i1 = readw(data);
			bPrintAll && printf("seg(%lx)", i1);
			return;

		case 20 :
			i1 = readw(data);
			bPrintAll && printf("grouporg(%lx)", i1);
			return;

		case 22 :
			i1 = readw(data);
			bPrintAll && printf("sectend(%lx)", i1);
			return;

		case 24 :
			i1 = readw(data);
			bPrintAll && printf("groupend(%lx)", i1);
			return;

		default:
			bPrintAll && printf("(");
			printpatch(data);
			switch( op )
			{
				case 32 : bPrintAll && printf("="); break;
				case 34 : bPrintAll && printf("<>"); break;
				case 36 : bPrintAll && printf("<="); break;
				case 38 : bPrintAll && printf("<"); break;
				case 40 : bPrintAll && printf(">="); break;
				case 42 : bPrintAll && printf(">"); break;
				case 44 : bPrintAll && printf("+"); break;
				case 46 : bPrintAll && printf("-"); break;
				case 48 : bPrintAll && printf("*"); break;
				case 50 : bPrintAll && printf("/"); break;
				case 52 : bPrintAll && printf("&"); break;
				case 54 : bPrintAll && printf("!"); break;
				case 56 : bPrintAll && printf("^"); break;
				case 58 : bPrintAll && printf("<<"); break;
				case 60 : bPrintAll && printf(">>"); break;
				case 62 : bPrintAll && printf("%"); break;
				case 64 : bPrintAll && printf("---"); break;
				case 66 : bPrintAll && printf("-revword-"); break;
				case 68 : bPrintAll && printf("-check0-"); break;
				case 70 : bPrintAll && printf("-check1-"); break;
				case 72 : bPrintAll && printf("-bitrange-"); break;
				case 74 : bPrintAll && printf("-arshift_chk-"); break;
				default : bPrintAll && printf("?%d?", op); break;
			}
		printpatch( data );
		bPrintAll && printf( ")" );
	}
	return;
}


void
dumpcode( char* & data, long int len, unsigned long address )
{
	assert( len > 0 );

	if ( !bPrint )
	{
		data += len;
		return;
	}

	unsigned char ch;
	unsigned int l = (unsigned int)len;
	unsigned long addr = 0;

	unsigned long start = address & ~0xF;
	unsigned long end = address + len;
	for ( addr = start; addr < end; ++addr )
	{
		if ( ( addr & 15) == 0 )
			bPrint && printf( "\n%04x:", addr );
		if ( ( addr % 4 ) == 0 )
			bPrint && printf( " " );
		if ( addr >= address )
		{
			ch = *data++;
			bPrint && printf( "%02x ", ch );
		}
		else
			bPrint && printf( "   " );
	}

	bPrint && printf( "\n\n" );
}


void
usage()
{
	char *usage_strs[]={
		"link v%s Copyright 1997-1999 World Foundry Group.\n",
		"Psygnosis PSY-Q/Sony PlayStation Linker\n",
		"by William B. Norris IV\n\n",
		"Usage: link object_file*\n",
		0
	};

	int i = 0;
	while ( usage_strs[i] )
		printf( usage_strs[i++], szVersion );
}

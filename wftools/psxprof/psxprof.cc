//==============================================================================
// psxprof.cc
// Copyright (c) 1998-1999, World Foundry Group  
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

#include <cstdio>
#include <cassert>
#include <cstring>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
using namespace std;

#include "symbol.hp"
#include "sample.hp"

vector<Symbol> tblSymbol;

template <class T>
struct greater_addr : public binary_function< T, T, bool >
{
	bool operator()( const T& x_, const T& y_ ) const
	{
		return y_.Address() < x_.Address();
	}
};


void
psxprof( const char* szMapFile, const char* szSampleFile )
{
	FILE* fpMap = fopen( szMapFile, "rt" );
	assert( fpMap );

	FILE* fpSample = fopen( szSampleFile, "rb" );
	assert( fpSample );

	// MAP (.map) FILE
	char _[ 255 ];
	char szLine[ 512 ];
	while ( fgets( szLine, sizeof( szLine ), fpMap ) )
	{
		unsigned long address;
		char szFunctionName[ 255 ];
		int nParams;

		nParams = sscanf( szLine, " %X %s%s\n", &address, szFunctionName, _ );
		if ( nParams == 2 )
		{
			Symbol sym( string( szFunctionName ), address );

			if ( find( tblSymbol.begin(), tblSymbol.end(), sym ) == tblSymbol.end() )
				tblSymbol.push_back( sym );
		}
	}

	fclose( fpMap );

	greater_addr<Symbol> g;
	sort( tblSymbol.begin(), tblSymbol.end(), g );

	vector<Symbol>::iterator symbol;
	for ( symbol = tblSymbol.begin(); symbol != tblSymbol.end(); ++symbol )
	{
		symbol->_size = (symbol-1)->Address() - symbol->Address();
	}

	// SAMPLE (.sym) FILE
	Sample sampleData( fpSample );
	sampleData.calculatePercentages();
	sampleData.print();
}

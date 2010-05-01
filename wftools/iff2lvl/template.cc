////////////////////////////////////////////////////////////////////////////////
//
// By William B. Norris IV
// Copyright 1995,1996 Cave Logic Studios, Ltd.  All Rights Reserved.
//
// v0.0.1	12 Dec 96	Created
//
////////////////////////////////////////////////////////////////////////////////

#include "template.hp"

//#include <iostream.h>
//#include <fstream.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//#include <stl/bstring.h>
//#include <stl/vector.h>
//#include <stl/algo.h>

//#include "memcheck.h"

const char FIELD_BEGIN = '«';
const char FIELD_END = '»';

string
_template( string szTemplate, vector<Symbol> tblSymbols )
{
	string ret;

	size_t old_pos = 0;
	size_t pos = 0;
	while ( ( pos = szTemplate.find( FIELD_BEGIN, old_pos ) ) < szTemplate.length() )
	{
//		cout << "found [ at " << pos << endl;
		ret.append( szTemplate, old_pos, pos-old_pos+1-1 );
		size_t endOfField = szTemplate.find( FIELD_END, pos+1 );
		assert( endOfField < szTemplate.length() );				// Unmatched «
		string field( szTemplate, pos+1, endOfField-pos-1 );
//		cout << FIELD_BEGIN << field << FIELD_END << endl;

		Symbol symFind( field );
		vector<Symbol>::iterator pSymbolFound;
		pSymbolFound = find( tblSymbols.begin(), tblSymbols.end(), symFind );
		if ( pSymbolFound == tblSymbols.end() )
			cerr << "Symbol \"" << symFind.Name() << "\" not found" << endl;
		assert( pSymbolFound != tblSymbols.end() );

		ret += (*pSymbolFound).Value();

		old_pos = endOfField + 1;
	}
	ret.append( szTemplate, old_pos+1 );				// Add rest of file

	return ret;
}

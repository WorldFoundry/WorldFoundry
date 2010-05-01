////////////////////////////////////////////////////////////////////////////////
//
// By William B. Norris IV
// Copyright 1995,1996 Cave Logic Studios, Ltd.  
//
// v0.0.1	12 Dec 96	Created
//
////////////////////////////////////////////////////////////////////////////////

#include <template/template.hp>

#include <pigsys/assert.hp>
#include <algorithm>

const char FIELD_BEGIN = '«';
const char FIELD_END = '»';

std::string
_template( std::string szTemplate, std::vector<Symbol> tblSymbols )
{
	std::string ret;

	size_t old_pos = 0;
	size_t pos = 0;
	while ( ( pos = szTemplate.find( FIELD_BEGIN, old_pos ) ) < szTemplate.length() )
	{
//		std::cout << "found [ at " << pos << endl;
		ret.append( szTemplate, old_pos, pos-old_pos+1-1 );
		size_t endOfField = szTemplate.find( FIELD_END, pos+1 );
		assert( endOfField < szTemplate.length() );				// Unmatched «
		std::string field( szTemplate, pos+1, endOfField-pos-1 );
//		std::cout << FIELD_BEGIN << field << FIELD_END << endl;

		Symbol symFind( field, std::string( "" ) );
		std::vector<Symbol>::iterator pSymbolFound;
		pSymbolFound = std::find( tblSymbols.begin(), tblSymbols.end(), symFind );
		if ( pSymbolFound == tblSymbols.end() )
			std::cerr << "Symbol \"" << symFind.Name() << "\" not found" << std::endl;
		assert( pSymbolFound != tblSymbols.end() );

		ret += (*pSymbolFound).Value();

		old_pos = endOfField + 1;
	}
	ret.append( szTemplate.c_str() + old_pos+1 );				// Add rest of file

	return ret;
}

//==============================================================================


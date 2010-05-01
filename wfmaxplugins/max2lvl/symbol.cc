// symbol.cc

#include <stdlib.h>
#include <string.h>

#include "symbol.hpp"

Symbol::Symbol( const string szName, const string szValue ) :
	_szName( szName ),
	_szValue( szValue )
{
}


Symbol::Symbol()
{
}


Symbol::~Symbol()
{
}

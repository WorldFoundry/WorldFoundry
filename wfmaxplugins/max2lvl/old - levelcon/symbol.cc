// symbol.cc

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "symbol.hp"

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
// symbol.cc

#include <cstdlib>

#include <pigsys/pigsys.hp>
#include <template/symbol.hp>

Symbol::Symbol( const std::string& szName, const std::string& szValue ) :
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

// oad.cpp

#include "oad.hpp"
#include "util.h"
#include <assert.h>

#include "attrib.h"

extern Attributes theAttributes;

Oad::Oad( const char* szOadFile )
{
	char szOadPath[ _MAX_PATH ];

	char* pOadDir = theAttributes.szOadDir;
	assert( pOadDir );

	sprintf( szOadPath, "%s/%s.oad", pOadDir, szOadFile );

	_oadFileChunk = (oadFile*)LoadBinaryFile( szOadPath, _len );
	if ( !_oadFileChunk )
		throw Exception();
	assert( _oadFileChunk );
	assert( _len > 0 );

	_td = (typeDescriptor*)(((char*)_oadFileChunk)+sizeof( oadHeader ) );
	assert( _td );

	_szFileName = std::string( szOadFile );
	_szClassName = std::string( _oadFileChunk->header.name );
}


Oad::~Oad()
{
	assert( _oadFileChunk );
	free( _oadFileChunk );
}


const std::string&
Oad::className() const
{
	return _szClassName;
}


const std::string&
Oad::fileName() const
{
	return _szFileName;
}


typeDescriptor*
Oad::startOfTypeDescriptors() const
{
	assert( _td );
	return _td;
}


int
Oad::len() const
{
	return _len - sizeof( _oadHeader );
}

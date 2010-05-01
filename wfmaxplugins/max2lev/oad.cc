// oad.cc

#include "global.hpp"
#include "oad.hpp"
#include <stdstrm.hp>
//#include <pigsys/pigsys.h>
#include "wf_id.hp"
#include "registry.h"
void* LoadBinaryFile( const char* _szFilename, int& nSize );

Oad::Oad( const char* szOadFile )
{
	DEBUGLOG((*debuglog) << "\nOad::Oad:" << endl;)

	char szOadPath[ _MAX_PATH ];

	char szOadDir[ _MAX_PATH ];
	DEBUGLOG((*debuglog) << "\nOad::Oad: look up wf in registry" << endl;)
	int success = GetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK, "OAD_DIR", szOadDir, sizeof( szOadDir ) );
	DEBUGLOG((*debuglog) << "\nOad::Oad: check to see if found" << endl;)
	AssertMessageBox(success,"OAD_DIR not set in registry");
	if ( !success )
		throw LVLExporterException();	//Error( "OAD_DIR not set in registry" );

	DEBUGLOG((*debuglog) << "\nOad::Oad: construct oad path" << endl;)
	sprintf( szOadPath, "%s/%s.oad", szOadDir, szOadFile );

	DEBUGLOG((*debuglog) << "\nOad::Oad: read binary file: " <<  szOadPath << endl;)
	_oadFileChunk = (oadFile*)LoadBinaryFile( szOadPath, _len );
	AssertMsg(_oadFileChunk,"file " << szOadPath << " not found");
	if ( !_oadFileChunk )
		throw LVLExporterException(1);
	assert( _oadFileChunk );
	assert( _len > 0 );

	DEBUGLOG((*debuglog) << "\nOad::Oad: type descriptor" << endl;)

	_td = (typeDescriptor*)(((char*)_oadFileChunk)+sizeof( oadHeader ) );
	assert( _td );

	_szFileName = string( szOadFile );
	_szClassName = string( _oadFileChunk->header.name );
	DEBUGLOG((*debuglog) << "\nOad::Oad: done" << endl;)
}


Oad::~Oad()
{
	assert( _oadFileChunk );
	free( _oadFileChunk );
}


const string&
Oad::className() const
{
	return _szClassName;
}


const string&
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

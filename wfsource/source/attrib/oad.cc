// oad.cc

#include <pigsys/pigsys.hp>
#include <attrib/oad.hp>
#include <attrib/util.hp>
#include <attrib/attrib.hp>
#include <loadfile/loadfile.hp>

Oad::Oad( const char* szOadFile )
{
	_oadFileChunk = (oadFile*)LoadBinaryFile( szOadFile, _len );
	if ( !_oadFileChunk )
		throw Exception();
	assert( _oadFileChunk );
	assert( _len > 0 );

	_td = (typeDescriptor*)(((char*)_oadFileChunk)+sizeof( oadHeader ) );
	assert( _td );

	_szFileName = std::string( szOadFile );
	_szClassName = std::string( _oadFileChunk->header.name );
}


#if 0
// Helper function to locate .oad in OAD_DIR if not in current directory
const char*
Oad::Find( const char* szOadFile )
{
	static char szOadPath[ _MAX_PATH ];

	const char szRegWorldFoundry[] = "Software\\World Foundry\\GDK";

	char szOadDir[ _MAX_PATH ];
	int success = GetLocalMachineStringRegistryEntry( szRegWorldFoundry, "OAD_DIR", szOadDir, sizeof( szOadDir ) );
	AssertMsg( success == 0, "OAD_DIR not set in registry" );
	char* pOadDir = szOadDir;
	assert( pOadDir && *pOadDir );

	sprintf( szOadPath, "%s/%s.oad", pOadDir, szOadFile );

#if 0
	_oadFileChunk = (oadFile*)LoadBinaryFile( szOadPath, _len );
	if ( !_oadFileChunk )
		throw Exception();
	assert( _oadFileChunk );
	assert( _len > 0 );
#endif

	return ;
}
#endif


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

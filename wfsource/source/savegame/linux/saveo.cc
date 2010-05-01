// saveio.cc

#include <savegame/savegame.hp>
#include <unistd.h>

//#include <io.h>
#include <fcntl.h>
//#include <direct.h>
#include <cerrno>

int
saveostream::createSaveFile( const char* name )
{
#if 0
	const char* szSavedGameDirectory = "savegame/";
	int error = mkdir( szSavedGameDirectory );
//MSVC:	assert( ( error == 0 ) || ( errno == EACCES ) );
	assert( ( error == 0 ) || ( errno == EEXIST ) );

	char szFilename[ _MAX_PATH ];
	strcpy( szFilename, szSavedGameDirectory );
	strcat( szFilename, name );

	DBSTREAM1( cout << "Creating save game file \"" << szFilename << "\" of " << _nBlocks << " blocks" << endl; )

	// TODO: what should third parameter be?
	int fh = open( szFilename, _O_CREAT | _O_TRUNC | _O_WRONLY, 0 );
	assert( fh != -1 );
	close( fh );

	fh = open( szFilename, _O_BINARY | _O_WRONLY );
	assert( fh != -1 );
	writeHeader();

	return fh;
#else
    assert(0);          // kts write unix version
#endif
}


void
saveostream::closeSaveFile() const
{
    assert(0);          // kts write unix version
	assert( _fh != -1 );
	close( _fh );
}


int
saveostream::writeSaveFile() const
{
    assert(0);          // kts write unix version
	assert( _fh != -1 );
	assert( _buf );
	assert( _bufsz > 0 );
	return ::write( _fh, _buf, _bufsz );
}

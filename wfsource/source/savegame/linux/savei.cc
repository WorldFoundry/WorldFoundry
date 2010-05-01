// savei.cc

#include <savegame/savegame.hp>
#include <unistd.h>

//#include <io.h>
#include <fcntl.h>
//#include <direct.h>
#include <cerrno>

int
saveistream::openSaveFile( const char* name )
{
#if 0
	const char* szSavedGameDirectory = "savegame/";
	int error = mkdir( szSavedGameDirectory );
//MSVC:	assert( ( error == 0 ) || ( errno == EACCES ) );
	assert( ( error == 0 ) || ( errno == EEXIST ) );

	char szFilename[ _MAX_PATH ];
	strcpy( szFilename, szSavedGameDirectory );
	strcat( szFilename, name );

	DBSTREAM1( cout << "Open save game file \"" << szFilename << "\"" << endl; )

	int fh = open( szFilename, _O_BINARY | _O_RDONLY );
	assert( fh != -1 );

	return fh;
#else
    assert(0);          // kts write unix version
    return 0;
#endif
}


void
saveistream::closeSaveFile() const
{
    assert(0);          // kts write unix version
	assert( _fh != -1 );
	close( _fh );
}


int
saveistream::readSaveFile()
{
    assert(0);              // kts write unix version
	assert( _fh != -1 );
	assert( _buf );
	assert( _bufsz > 0 );

	return ::read( _fh, (void*)_buf, _bufsz );
}

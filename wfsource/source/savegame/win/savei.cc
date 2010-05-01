// savei.cc

#include <savegame/savegame.hp>

#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>

int
saveistream::openSaveFile( const char* name )
{
	const char* szSavedGameDirectory = "savegame/";
	int error = mkdir( szSavedGameDirectory );
//MSVC:	assert( ( error == 0 ) || ( errno == EACCES ) );
	assert( ( error == 0 ) || ( errno == EEXIST ) );

	char szFilename[ _MAX_PATH ];
	strcpy( szFilename, szSavedGameDirectory );
	strcat( szFilename, name );

	DBSTREAM1( std::cout << "Open save game file \"" << szFilename << "\"" << std::endl; )

	int fh = open( szFilename, _O_BINARY | _O_RDONLY );
	assert( fh != -1 );

	return fh;
}


void
saveistream::closeSaveFile() const
{
	assert( _fh != -1 );
	close( _fh );
}


int
saveistream::readSaveFile()
{
	assert( _fh != -1 );
	assert( _buf );
	assert( _bufsz > 0 );

	return ::read( _fh, (void*)_buf, _bufsz );
}

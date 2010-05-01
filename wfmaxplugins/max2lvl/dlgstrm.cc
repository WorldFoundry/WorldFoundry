// dlgstream.cpp

#include "dlgstrm.hpp"
#include <string.h>
#include <assert.h>
#include <stdlib.h>

dialogstreambuf::dialogstreambuf()
{
}


dialogstreambuf::~dialogstreambuf()
{
}


int
dialogstreambuf::xsputn( const char* s, int n )
{
	return 0;
}


int
dialogstreambuf::overflow( int c )
{
	return 0;
}


//////////////////////////////////////////////////////////////////////

dialogstream::dialogstream( const char* szTitle )
{
	_szTitle = strdup( szTitle );
	assert( _szTitle );

#if 0
	DialogBox(

	MAKEINTRESOURCE( IDD_DIALOGSTREAM );
	_hDlg = 
#endif
}


dialogstream::~dialogstream()
{
	// Enable OK button, wait for button press

	assert( _szTitle );
	free( _szTitle );
}





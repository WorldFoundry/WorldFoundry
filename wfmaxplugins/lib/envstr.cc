// envstr.cc

#include <windows.h>
#include <string>

string
ExpandEnvironmentStrings( string strIn )
{
	char szBuffer[ 2048 ];

	ExpandEnvironmentString( strIn.c_str(), szBuffer, sizeof( szBuffer ) );

	return string( szBuffer );
}

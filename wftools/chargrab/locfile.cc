//==============================================================================
// locfile.cc: Copyright (c) 1995-1999, World Foundry Group  
// By William B. Norris IV
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org
//==============================================================================

#include "global.hp"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
#include <stdlib.h>
#include <pigsys/assert.hp>
#include <malloc.h>

#include "srccntrl.hp"
#include "textile.hp"
extern bool bSourceControl;

static const bDebug = 0;
static char szFoundFile[ PATH_MAX ];

static bool
try_file( const char* filename )
{
	bDebug && cout << "Checking " << filename << endl;
//	ifstream* input = new ifstream( filename, ios::nocreate );
	ifstream* input = new ifstream( filename );
	if ( input && input->rdstate() == ios::goodbit )
	{
		delete input;
		strcpy( szFoundFile, filename );
		bDebug && szFoundFile && cout << "Found " << szFoundFile << endl;
		return true;
	}
	else
		return false;
}


char*
locateFile( const char* filename, const char* _path, const char* _anotherDirectory )
	{
	ifstream* input;
	char* path = (char*)alloca( strlen( _path ) + 1 );
	assert( path );
	strcpy( path, _path );

	*szFoundFile = '\0';

	{ // Try the whole filename passed name first (in case of an absolute path)
	if ( try_file( filename ) )
		goto done;
	}

#if 0
	if ( !getenv( "WF_DIR" ) )
	{
		static bool bWarned = false;
		if ( !bWarned )
		{
			cout << "Warning: \"WF_DIR\" not set in environment; returning from locateFile()" << endl;
			bWarned = true;
		}
		goto done;
	}

	{ // VELOCITY_DIR
	char szVelocityPath[ PATH_MAX ];
	strcpy( szVelocityPath, getenv( "WF_DIR" ) );
	strcat( szVelocityPath, "/" );
	strcat( szVelocityPath, filename );
	if ( try_file( szVelocityPath ) )
		goto done;

	if ( _anotherDirectory )
	{
		strcpy( szVelocityPath, getenv( "WF_DIR" ) );
		strcat( szVelocityPath, "/" );
		strcat( szVelocityPath, _anotherDirectory );
		strcat( szVelocityPath, "/" );
		strcat( szVelocityPath, filename );
		if ( try_file( szVelocityPath ) )
			goto done;
	}
	}
#endif

	char szPath[ PATH_MAX ];
	{ // strok path passed in
	char szDrive[ _MAX_DRIVE ];
	char szDir[ _MAX_DIR ];
	char szFilename[ _MAX_FNAME ];
	char szExt[ _MAX_EXT ];

	bDebug && (cout << "locateFile(" << filename << ':' << path << ")" << "->" << filename << endl);

	{
	char* p;
	for ( p = strtok( path, ";" ); p; p = strtok( NULL, ";" ) )
		{
		_splitpath( filename, NULL, NULL, szFilename, szExt );
		_makepath( szPath, NULL, p, szFilename, szExt );

		if ( try_file( szPath ) )
			goto done;

		if ( _anotherDirectory )
		{
			_makepath( szPath, NULL, p, NULL, NULL );
			strcat( szPath, "/" );
			strcat( szPath, _anotherDirectory );
			strcat( szPath, "/" );
			strcat( szPath, szFilename );
			strcat( szPath, "." );
			strcat( szPath, szExt );
			if ( try_file( szPath ) )
				goto done;
		}
		}
	}
	}

	{
	strcpy( path, _path );
	bDebug && cout << "(Whole filename) path = " << path << endl;
	char* p;
	for ( p = strtok( path, ";" ); p; p = strtok( NULL, ";" ) )
		{
		strcpy( szPath, p );
		strcat( szPath, "/" );
		strcat( szPath, filename );

		if ( try_file( szPath ) )
			goto done;

		if ( _anotherDirectory )
		{
			strcpy( szPath, p );
			strcat( szPath, "/" );
			strcat( szPath, _anotherDirectory );
			strcat( szPath, "/" );
			strcat( szPath, filename );

			if ( try_file( szPath ) )
				goto done;
		}
		}
	}

done:
	if ( bSourceControl && *szFoundFile )
		{
		if ( !fileInSourceCodeControlSystem( szFoundFile ) )
			{
			cerr << warning() << "\"" << filename << "\" has not been added to the project." << endl;
			*log << warning() << "\"" << filename << "\" has not been added to the project." << endl;
			}
		}

	return szFoundFile;
	}

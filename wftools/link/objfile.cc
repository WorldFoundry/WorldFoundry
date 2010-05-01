//==============================================================================
// objfile.cc
// Copyright (c) 1998-1999, World Foundry Group  
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

#include "objfile.hp"
#include <pigsys/assert.hp>
#include <iostream>

ObjectFile::ObjectFile()
{
	assert( 0 );
}


ObjectFile::ObjectFile( char* szFilename, void* dataIn, int cbDataIn )
{
	cbData = cbDataIn;
	pStartData = (char*)dataIn;
	pEndData = pStartData + cbData;

	currentSection = 0;

	Section section0( string( "invalid section #0 (placeholder)" ), 0 );
	sections.push_back( section0 );

	char* data = (char*)dataIn;
	char header[ 4 ];
	strncpy( header, data, 3 );
	*( header + 3 ) = '\0';

	if ( strncmp( header, "LNK", 3 ) != 0 )
	{
		free( pStartData );
		throw;
	}
	data += 3;

	int version;
	if ( ( version = *data++ ) != 2 )
	{
		printf("Error - file \'%s\' is in an %s SNLink object file format - exiting\n",
						szFilename, version<2 ? "obsolete" : "unknown");
		exit( 1 );
	}

	cout << szFilename << '\t' << header << " version " << version << endl;
}


ObjectFile::~ObjectFile()
{
	assert( pStartData );
//TEMP	free( pStartData );

	sections.erase( sections.begin(), sections.end() );
	functions.erase( functions.begin(), functions.end() );

}

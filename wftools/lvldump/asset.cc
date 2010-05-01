//============================================================================
// asset.cc:
// Copyright (c) 1996-1999, World Foundry Group  
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
/* Documentation:

	Abstract:
			class containing array of assets
	History:
			Created	6/19/96 11:51AM Kevin T. Seghetti

	Class Hierarchy:
			none

	Dependancies:
		PIGS, STL, iostream
	Restrictions:

	Example:
*/
//============================================================================

#include "global.hp"
#include <pigsys/assert.hp>

//#include <pclib/stdstrm.hp>
#include <cpplib/stdstrm.hp>
#include "asset.hp"
#include <iterator>

//============================================================================

void
AssetExtensions::Load(istream& in)
{
	DBSTREAM3( cdebug << "AssetExtensions::Load: " << endl; )
	char stringBuffer[100];

	while(in.rdstate() == ios::goodbit)
	 {
		in.getline(stringBuffer,100);

		if(strlen(stringBuffer) != 0 && stringBuffer[strlen(stringBuffer)-1] == '\n')
			stringBuffer[strlen(stringBuffer)-1] = 0;
		if(strlen(stringBuffer) != 0 && (stringBuffer[0] != '/' && stringBuffer[1] != '/'))
		 {
			// only read 1st 4 chars
			stringBuffer[4] = 0;
			DBSTREAM3( cdebug << "AssetExtensions::Load: parsing line <" << stringBuffer << ">" << endl; )
			assert(Lookup(stringBuffer) == -1);				// insure it is not already in the list
			extensions.push_back(stringBuffer);
		 }
	 }
	DBSTREAM3( cdebug << "AssetExtensions::Load: list:" << endl; )
	DBSTREAM3( Save(cdebug); )
}

//============================================================================

void
AssetExtensions::Save(ostream& out)
{
	copy(extensions.begin(),extensions.end(),ostream_iterator<string>(out,"\n"));
}

//============================================================================

string
AssetExtensions::operator[](int index)
{
	return(extensions[index]);
}

//============================================================================

int
AssetExtensions::Lookup(string extension)
{
	DBSTREAM3( cdebug << "AssetExtensions::Lookup: string = <" << extension << ">" << endl; )

	for(int index=0;index<extensions.size();index++)			// kts does STL have a better way to do this?
	 {
		DBSTREAM3( cdebug << "AssetExtensions::Lookup: checking index " << index << endl; )
		if(extensions[index] == extension)
		 {
			DBSTREAM3( cdebug << "found, index = " << index << endl; )
			return(index);
		 }
	 }
	DBSTREAM3( cdebug << "AssetExtensions::Lookup: failed" << endl; )
	return(-1);
}

//============================================================================

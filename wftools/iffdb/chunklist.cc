//=============================================================================
// chunklist.cc: kts iff reader, creates an in-memory database of an iff stream
// By Kevin T. Seghetti
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

#include "global.hp"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <recolib/command.hp>
#include <recolib/hdump.hp>
#include <cpplib/stdstrm.hp>
#include <streams/binstrm.hp>
#include <iff/iffread.hp>
#include "chunklist.hp"
#include "iffdb.hp"
using namespace std;

//=============================================================================

void
ChunkList::Load(istream& in)
{
	DBSTREAM3( cdebug << "ChunkList::Load: " << endl; )
	char stringBuffer[100];

	while(in.good())
	 {
		stringBuffer[0] = 0;
		stringBuffer[1] = 0;
		stringBuffer[2] = 0;
		stringBuffer[3] = 0;
		in.getline(stringBuffer,100);

		if(strlen(stringBuffer) != 0 && stringBuffer[strlen(stringBuffer)-1] == '\n')
			stringBuffer[strlen(stringBuffer)-1] = 0;
		if(strlen(stringBuffer) != 0 && (stringBuffer[0] != '/' && stringBuffer[1] != '/'))
		 {
			// only read until whitespace
			stringBuffer[ strcspn(stringBuffer, " \t") ] = 0;
			DBSTREAM3( cdebug << "ChunkList::Load: parsing line <" << stringBuffer << ">" << endl; )

			assert(strlen(stringBuffer) <= 4);
			int32 id = *(int32*)(&stringBuffer[0]);
			ChunkID cid(id);
			chunks.push_back(cid);
		 }
	 }
}

//=============================================================================

int
ChunkList::Lookup(ChunkID id) const
{
	for(unsigned int index=0;index<chunks.size();index++)			// kts does STL have a better way to do this?
	 {
		DBSTREAM3( cdebug << "ChunkList::Lookup: checking index " << index << " which contains " << chunks[index] << endl; )
		if(chunks[index] == id)
		 {
			DBSTREAM3( cdebug << "found, index = " << index << endl; )
			return(index);
		 }
	 }
//	AssertMessageBox(0,"ChunkList::Lookup: failed, looked for extension " << extension );
	return(-1);
}


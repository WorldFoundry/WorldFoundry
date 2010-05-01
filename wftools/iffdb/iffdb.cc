//=============================================================================
// IFFDataBase.cc: kts iff reader, creates an in-memory database of an iff stream
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
#include <cpplib/stdstrm.hp>
#include <streams/binstrm.hp>
#include "chunklist.hp"
#include "IFFDb.hp"
#include "indent.hp"
#include <hal/hal.h>
#include <recolib/hdump.hp>

#include <map>
using namespace std;

//=============================================================================

extern int charsPerLine;
extern bool dumpBinary;
extern bool useHDump;

//=============================================================================

IFFDataBaseChunk::IFFDataBaseChunk()
{
	_hasChildren = false;
	_data = NULL;
	_size = 0;
}

//=============================================================================

IFFDataBaseChunk::IFFDataBaseChunk(IFFChunkIter& topIter, const IFFDataBase& database)
{
	_name = topIter.GetChunkID();
	_hasChildren = database.IsWrapperChunk(_name);
	_size = topIter.Size();
	_data = NULL;
	if(_hasChildren)
	{
		while(topIter.BytesLeft())
		{
			IFFChunkIter* chunkIter = topIter.GetChunkIter(HALLmalloc);
			IFFDataBaseChunk child(*chunkIter,database);
			_children.push_back(child);
			delete chunkIter;
		}
	}
	else		                        // must be leaf
	{
		_data = new char[_size];
		assert(ValidPtr(_data));
		topIter.ReadBytes(_data,_size);
	}
	Validate();
}

//=============================================================================

IFFDataBaseChunk::IFFDataBaseChunk(const IFFDataBaseChunk& other)
{
	other.Validate();
	_size = other._size;
	_name = other._name;
	_hasChildren = other._hasChildren;
	_children = other._children;
	if(other._data)
	{
		_data = new char[_size];
		assert(ValidPtr(_data));
		memcpy(_data, other._data,_size);
	}
	else
		_data = NULL;
	Validate();
}

//=============================================================================

IFFDataBaseChunk::~IFFDataBaseChunk()
{
	Validate();
	if(_data)
		delete [] _data;
}

//=============================================================================

IFFDataBaseChunk&
IFFDataBaseChunk::operator= (const IFFDataBaseChunk& other)
{
	other.Validate();
	if(this != &other)
	{
		if(_data)
			delete [] _data;
		_name = other._name;
		_size = other._size;
		_hasChildren = other._hasChildren;
		_children = other._children;
		if(other._data)
		{
			_data = new char[_size];
			assert(ValidPtr(_data));
			memcpy(_data, other._data,_size);
		}
	}
	Validate();
	return *this;
}

//=============================================================================

void
IFFDataBaseChunk::Print(ostream& out, Indent ind)
{
	Validate();
	out << ind << "{ '" << _name << "'\t//size = " << _size << ", hasChildren = " << _hasChildren << endl;
	if(_data)
	{
		if(dumpBinary)
		{
			ind++;
			if(useHDump)
				HDump(_data, _size, ind.Value(), "\t", out,charsPerLine);
			else
			{
				char* data = _data;
				int count = _size;
				int lineCounter=0;
				out << setfill('0') << hex;
				while(count--)
				{
					if(!(lineCounter % charsPerLine))
						out << endl << ind;
					unsigned char value = *data++;
					out << "$" << setw(2) << ((unsigned int)value) << " ";
					lineCounter++;
				}
				out << endl;
			}
			ind--;
		}
	}
	if(_hasChildren)
	{
		vector<SmartPtr<IFFDataBaseChunk> >::iterator iter = _children.begin();
		while(iter != _children.end())
		{
			ind++;
			(**iter).Print(out,ind);
			ind--;
			++iter;
		}
	}
	out << ind << "}" << endl;
}

//=============================================================================

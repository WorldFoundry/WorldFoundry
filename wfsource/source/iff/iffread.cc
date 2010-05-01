//=============================================================================
// iffread.cc: iff reader class
// Copyright ( c ) 1997,99 World Foundry Group  
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

// ===========================================================================
// Description:
//	istrstream
//	istrstreamIter is an iterator class that iterates over a single chunk of data
//	it has several operator<< functions, for strings and int32 and fixed32, etc.
//	It also has an istrstreamIter constructor, which allows for reading nested chunks.
//
// Original Author: Kevin T. Seghetti
//============================================================================

#define _IFFREAD_CC
#include <cpplib/stdstrm.hp>
#include <streams/binstrm.hp>
#include <iff/iffread.hp>
#include <memory/memory.hp>
#include <cpplib/libstrm.hp>

const int IFF_CHUNK_HEADER_SIZE = 8;

//=============================================================================

IFFChunkIter::IFFChunkIter(binistream& bstream) :
	_stream(bstream)
{
	_locked = false;
	_parent = NULL;
	assert(_stream.good());
	_stream >> _chunkID;
	AssertMsg(_chunkID.Valid(),"_chunkID = " << _chunkID);
    DBSTREAM3( ciff << "IFFChunkIter constructed, istream position = " << _stream.tellg() << ", chunkID = " << _chunkID << std::endl; )
	_stream >>_chunkSize;
//	AssertMsg(_chunkSize,"_chunkID = " << _chunkID);			// kts 03-23-98 10:50am zero length chunks are ok

	_bytesLeft = _chunkSize;
	int32 _alignedChunkSize = (_chunkSize + 3) & ~3;
	assert((_alignedChunkSize % 4) == 0);
	_extraBytesToRead = _alignedChunkSize - _chunkSize;
	assert(_extraBytesToRead < 4);
	Validate();
//	cout << "IFFChunkIter created: " << _chunkID << ", size = " << _bytesLeft << std::endl;
	assert(_bytesLeft == 0 || _stream.good());
}

//=============================================================================

IFFChunkIter::IFFChunkIter(const IFFChunkIter& rhs) :
	_stream(rhs._stream), _chunkSize(rhs._chunkSize), _bytesLeft(rhs._bytesLeft)
{
	assert(_stream.good());
}

//=============================================================================

IFFChunkIter::~IFFChunkIter()
{
	assert(!_locked);
	Validate();

    DBSTREAM3( ciff << "IFFChunkIter destructor, istream position = " << _stream.tellg() << ", chunkID = " << _chunkID << std::endl; )

	char c;
	DBSTREAM1(
	if(_bytesLeft)
		cwarn << _chunkID << " closed without all data read (bytesLeft = " << _bytesLeft << ")" << std::endl;
	)
//	cout << "~IFFChunkIter: bytesleft = " << _bytesLeft << std::endl;
	while(_bytesLeft--)
		_stream >> c;
	while(_extraBytesToRead--)
		_stream >> c;
	if(_parent)
		_parent->_locked = false;
}

//=============================================================================
// kts change this to use the multi-byte read command in istream

void
IFFChunkIter::ReadBytes(void* buffer, int count)
{
	assert(!_locked);
	if(count)
	{
		assert(_bytesLeft > 0);
		assert(_stream.good());
	//	cout << "ReadBytes: count = " << count << ", bytesleft = " << _bytesLeft << std::endl;
		Validate();
		AssertMsg(_bytesLeft >= count,"Attempted to read past end of chunk: " << _chunkID
			<< "\tcount = " << count << ", _bytesLeft = " << _bytesLeft );
		char* pBuffer = (char*)buffer;
		int loopCount = count;
#pragma message ("KTS " __FILE__ ": call lower level multi-byte read")
		while(loopCount--)
		{
			AssertMsg(_stream.good(), "ReadBytes stream failed while trying to read " << count << " bytes, loopCount  = " << loopCount);
			assert(_stream.good());
			_stream >> *pBuffer++;
		}
		_bytesLeft -= count;
		Validate();
	}
//	cout << "now bytesleft = " << _bytesLeft << std::endl;
}

//=============================================================================
// kts change this to use the multi-byte read command in istream

void
IFFChunkIter::SkipBytes(int count)
{
	assert(!_locked);
	if(count)
	{
      char devnull;
		assert(_bytesLeft > 0);
		assert(_stream.good());
	//	cout << "ReadBytes: count = " << count << ", bytesleft = " << _bytesLeft << std::endl;
		Validate();
		AssertMsg(_bytesLeft >= count,"Attempted to read past end of chunk: " << _chunkID
			<< "\tcount = " << count << ", _bytesLeft = " << _bytesLeft );
		int loopCount = count;
#pragma message ("KTS " __FILE__ ": call lower level multi-byte read")
		while(loopCount--)
		{
			AssertMsg(_stream.good(), "ReadBytes stream failed while trying to read " << count << " bytes, loopCount  = " << loopCount);
			assert(_stream.good());
			_stream >> devnull;
		}
		_bytesLeft -= count;
		Validate();
	}
//	cout << "now bytesleft = " << _bytesLeft << std::endl;
}

//=============================================================================
// kts I don't know how to do this right

IFFChunkIter*
IFFChunkIter::GetChunkIter(Memory& memory)
{
	assert(!_locked);
	assert(_stream.good());
//	cout << "GetChunkIter: bytesLeft before = " << _bytesLeft << std::endl;
	assert(_bytesLeft >= IFF_CHUNK_HEADER_SIZE);					// a new chunk must contain at least 8 bytes

//	IFFChunkIter* retVal = NEW(IFFChunkIter(_stream));
	IFFChunkIter* retVal = new (memory) IFFChunkIter(_stream);
	assert(ValidPtr(retVal));
	AssertMsg(_bytesLeft >= retVal->Size() + retVal->_extraBytesToRead + IFF_CHUNK_HEADER_SIZE, "bytesLeft = " << _bytesLeft << ", retVal->Size = " << retVal->Size() << ", retVal->_extraBytesToRead = " <<  retVal->_extraBytesToRead << ",  IFF_CHUNK_HEADER_SIZE = " <<  IFF_CHUNK_HEADER_SIZE );
	_bytesLeft -= retVal->Size() + retVal->_extraBytesToRead + IFF_CHUNK_HEADER_SIZE;
//	cout << ", chunk size = " << retVal->Size() << ", new bytes left = " << _bytesLeft << std::endl;
	retVal->Validate();
	assert(_bytesLeft == 0 || _stream.good());
	_locked = true;
	retVal->_parent = this;
	return retVal;
}

//=============================================================================

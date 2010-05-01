//=============================================================================
// assslot.cc: AssetSlot class, represents either the permanent assets, or a rooms worth
// Copyright ( c ) 1997,1998,1999,2000,2001,2002,2003 World Foundry Group  
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
//	Abstract: AssetSlot class: manages the room assets
// Original Author: Kevin T. Seghetti
//=============================================================================

#include "assslot.hp"
#include "assets.hp"
#include <cpplib/libstrm.hp>
#include <iff/iffread.hp>
#include <gfx/texture.hp>

//=============================================================================

AssetSlot::AssetSlot(int slotIndex, int roomIndex, binistream& roomStream, int maxAsset, void* memory, int memorySize, VideoMemory& videoMemory) :
	_slotIndex(slotIndex),
	_assetsMemory(memory, memorySize MEMORY_NAMED( COMMA "Assets Memory " ) ),
    _videoMemory(videoMemory)
{
	MEMORY_NAMED(
		char szRoomName[ 32 ];
		sprintf( szRoomName, "Room #%d", roomIndex );
		_assetsMemory.Name( szRoomName );
	);

	DBSTREAM2 (casset << "AssetSlot::AssetSlot: slotIndex = " << slotIndex << std::endl; )
	assert(_slotIndex >=0);
	assert(_slotIndex < VideoMemory::MAX_SLOTS);

	// allocate large enough asset map
	_assetMap = new (_assetsMemory) AssetMap(maxAsset,_assetsMemory);
	assert(ValidPtr(_assetMap));

	assert(roomStream.good());

#if defined(DESIGNER_CHEATS)
	_videoMemory.ClearSlot(slotIndex);
#if defined(__PSX__)
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
	FntLoad(960,256);
	SetDumpFnt(FntOpen(0,12, 320, 200, 0, 1024));
#endif
#endif

	// loop through all strings, loading each asset into memory, and add it to the asset map
	DBSTREAM3( casset << "maxAsset = " << maxAsset << std::endl; )
	for(int assetIndex=0;assetIndex < maxAsset;assetIndex++)
	{
		DBSTREAM3 (casset << "assetIndex = " << assetIndex << std::endl; )
		assert(roomStream.good());
		IFFChunkIter chunkIter(roomStream);
		assert(chunkIter.GetChunkID().ID() == IFFTAG('A','S','S','\0'));

		// first read asset id
		int32 thisID;
		chunkIter.ReadBytes(&thisID,4);
		DBSTREAM3( casset << "thisID = " << (void*)thisID << std::endl; )
		packedAssetID id(thisID);
		DBSTREAM2( casset << "reading asset at id " << id << std::endl; )
#pragma message ("KTS: remove this extra copy, and just load the room data into the room slot, and calc pointers to it")
		int fileSize = chunkIter.BytesLeft();

		AssertMsg( id.Room() == roomIndex, "id.Room() = " << id.Room() << ", roomIndex = " << roomIndex );
		if(id.Type() == packedAssetID::TGA)
		{
			switch(id.Index())
			{
				case 0xFFE:		// palette tga is always at index 0xffe
				{
					LoadTexture(chunkIter,_videoMemory.GetSlotPalettePixelMap(slotIndex));
					break;
				}
				case 0xFFF:			// texture tga is always at index 0xfff
				{
					LoadTexture(chunkIter,_videoMemory.GetSlotTexturePixelMap(slotIndex));
					break;
				}
				default:
					AssertMsg(0,"id = " << id);
			}
			_assetMap->NULLAsset(assetIndex,id);
		}
		else
		{
			void* memory = new (_assetsMemory) char[fileSize];
			assert(ValidPtr(memory));

			chunkIter.ReadBytes((char*)memory,fileSize);
			assert(chunkIter.BytesLeft() == 0);

			// add to asset map
			assert(ValidPtr(_assetMap));
			_assetMap->SetAsset(assetIndex,id,memory,fileSize);
		}

	}
	DBSTREAM2( casset << "done reading assets" << std::endl; )
	_assetMap->Validate();
	DBSTREAM1( casset << *_assetMap << std::endl; )

   PixelMap& pm = _videoMemory.GetSlotTexturePixelMap(_slotIndex);

// room ruv is always at index 4095
	packedAssetID rmuvID;
	rmuvID.Type( packedAssetID::RUV );
	rmuvID.Room( roomIndex );
	rmuvID.Index( 4095 );			    // maximum
	binistream rmuvstream = LookupAsset( rmuvID );
	_rmuv = RMUV(LoadChunk(rmuvstream, IFFTAG('r','m','u','v')),pm.GetXPositionInVideoMemory(),pm.GetYPositionInVideoMemory());

// room cyc is always at index 4095
	packedAssetID ccycID;
	ccycID.Type( packedAssetID::CYC );
	ccycID.Room( roomIndex );
	ccycID.Index( 4095 );
	binistream ccycstream = LookupAsset( ccycID );
	_ccyc = CCYC( LoadChunk(ccycstream, IFFTAG('c','c','y','c')), pm.GetXPositionInVideoMemory(), pm.GetYPositionInVideoMemory() );

	Validate();
}

//=============================================================================

AssetSlot::~AssetSlot()
{
	Validate();
#pragma message ("KTS: how do I control the order of destruction? (I need to make sure the RMUV has been destroyed by the time the next line runs")
//	DELETE_CLASS (_rmuvMemory);		    // goes away automaticaly when slot is destructed
}

//=============================================================================

#if SW_DBSTREAM
std::ostream&
AssetSlot::_print( std::ostream& s ) const
{
	s << _assetsMemory << std::endl;

	return s;
}
#endif


//=============================================================================

void*
AssetSlot::LoadChunk( binistream& stream,  int32 tag)
{
	Validate();
	int32 chunkName;
	stream >> chunkName;
	AssertMsg( chunkName == tag, "chunkName = " << ChunkID( chunkName ) );
	int32 size;
	stream >> size;
	assert(size >= 0);
	assert(size < 10000);			    // some limit
	//std::cout << "size = " << size << std::endl;

//	char* _rmuvMemory = (char*)_assetsMemory.Allocate(sizeof(char)*(size+8));
	char* data = new (_assetsMemory) char[ size+8 ];
	assert( ValidPtr( data ) );
	*((int32*)data) = chunkName;
	*((int32*)(data+4)) = size;
	stream.read(data+8,size);
   return data;
}

//=============================================================================

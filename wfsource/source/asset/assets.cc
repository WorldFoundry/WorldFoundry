//=============================================================================
// Assets.cc: AssetManager class
// Copyright ( c ) 1997,1999,2000,2001,2002,2003 World Foundry Group  
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
//	Abstract: AssetManager class: manages the room assets
// Original Author: Kevin T. Seghetti
//=============================================================================

#include <cpplib/libstrm.hp>
#include "assets.hp"
#include "assslot.hp"

//=============================================================================

AssetManager::AssetManager( size_t cbPermMemory, size_t cbRoomMemory, VideoMemory& videoMemory, _DiskFile& diskFile, Memory& memory, AssetCallback& callback ) :
    _videoMemory(videoMemory),
   _diskFile(diskFile),
   _memory(memory),
   _callback(callback)
{
	_cbPermMemory = cbPermMemory;
	_cbRoomMemory = cbRoomMemory;

	_assetMemory = new ( HALLmalloc ) char [ _cbPermMemory + MAX_ACTIVE_ROOMS * _cbRoomMemory ];
	assert( ValidPtr( _assetMemory ) );

	for ( int idxRoomSlot=0; idxRoomSlot < MAX_ACTIVE_SLOTS; ++idxRoomSlot )
		_assets[idxRoomSlot] = NULL;

	_assetStringMapEntries = 0;

}

//=============================================================================

AssetManager::~AssetManager()
{
	for(int index=0;index < MAX_ACTIVE_SLOTS;index++)
	{
		if(_assets[index])
			MEMORY_DELETE(_memory,_assets[index],AssetSlot);
	}
}

//==============================================================================

void 
AssetManager::Validate() const
{
   assert(ValidPtr(_assets));
   assert(ValidPtr(_assetMemory));
   _videoMemory.Validate();
   _diskFile.Validate();
    _memory.Validate();
}

//=============================================================================

#if SW_DBSTREAM
std::ostream&
AssetManager::_print( std::ostream& s ) const
{
	s << "Asset manager" << std::endl;

	s << "Perm" << std::endl;
	assert( _assets[ PERM_SLOT_INDEX ] );
	s << *_assets[ PERM_SLOT_INDEX ] << std::endl;

	for ( int idxRoom=0; idxRoom<PERM_SLOT_INDEX; ++idxRoom )
	{
		s << "Room " << idxRoom << std::endl;
		if ( _assets[ idxRoom ] )
			s << *_assets[ idxRoom ];
	}

	return s;
}
#endif

//=============================================================================

struct CHUNKHDR
{
	int32 tag;
	int32 size;
};

//=============================================================================

void
AssetManager::LoadPermanents()
{
	DBSTREAM1( casset << "AssetManager::LoadPermanents:" << std::endl; )
	assert(_assets[PERM_SLOT_INDEX] == NULL);

	_levelTOC.Validate();
	_diskFile.Validate();

//	_diskFile.SeekForward(_toc[TOC_PERM_INDEX]._offsetInDiskFile);  // permanent offset index = 0
	_diskFile.SeekForward(_levelTOC.GetTOCEntry(TOC_PERM_INDEX)._offsetInDiskFile);  // permanent offset index = 0
	DBSTREAM3( casset << "roomoffset = " << _levelTOC.GetTOCEntry(TOC_PERM_INDEX)._offsetInDiskFile << std::endl; )
//	AssertMsg(_levelTOC.GetTOCEntry(TOC_PERM_INDEX)._size <  STREAM_BUFFER_SIZE,"_levelTOC.GetTOCEntry(TOC_PERM_INDEX)._size = " << _levelTOC.GetTOCEntry(TOC_PERM_INDEX)._size << ", STREAM_BUFFER_SIZE =" << STREAM_BUFFER_SIZE);  // otherwise it won't fit
	assert(_levelTOC.GetTOCEntry(TOC_PERM_INDEX)._size % DiskFileCD::_SECTOR_SIZE == 0);

//	char* streamBuffer = new (HALScratchLmalloc) char[_levelTOC.GetTOCEntry(TOC_PERM_INDEX)._size];
	char* streamBuffer = new (HALLmalloc) char[_levelTOC.GetTOCEntry(TOC_PERM_INDEX)._size];
	assert(ValidPtr(streamBuffer));
	_diskFile.ReadBytes(streamBuffer,_levelTOC.GetTOCEntry(TOC_PERM_INDEX)._size);
 	CHUNKHDR& chdr = *((CHUNKHDR*)streamBuffer);
	assert(chdr.tag = IFFTAG('P','E','R','M'));

	int maxAsset = 0;
	{
		binistream roomStream((void*)(((char*)streamBuffer)+sizeof(CHUNKHDR)),chdr.size);
		roomStream.seekg(0);
		while(roomStream.getFilelen() > roomStream.tellg())
		{
			assert(roomStream.good());
			IFFChunkIter roomChunkIter(roomStream);
			maxAsset++;
			assert(maxAsset < 1000);	// arbitrary
		}
	}
	binistream roomStream((void*)(((char*)streamBuffer)+sizeof(CHUNKHDR)),chdr.size);

	_assets[PERM_SLOT_INDEX] = new (_memory) AssetSlot(PERM_SLOT_INDEX, AssetManager::ROOM_PERM_INDEX,roomStream, maxAsset, (void*)&_assetMemory[_cbRoomMemory*PERM_SLOT_INDEX],_cbPermMemory,_videoMemory);
	assert(ValidPtr(_assets[PERM_SLOT_INDEX]));
	HALLmalloc.Free(streamBuffer,_levelTOC.GetTOCEntry(TOC_PERM_INDEX)._size);
}

//=============================================================================

void
AssetManager::LoadRoomSlot(int roomIndex, int slotNum)
{
	DBSTREAM2( casset << "AssetManager::LoadRoom: loading room #" << roomIndex << " into slot #" << slotNum << std::endl; )
	assert(roomIndex >= 0);

   //kts this will get caught by GetTOCEntry, below
	//AssertMsg(roomIndex < _maxRooms,"roomIndex = " << roomIndex << ", number of rooms = " << _maxRooms);
	assert(slotNum >= 0);
	assert(slotNum < MAX_ACTIVE_SLOTS);
	assert(_assets[slotNum] == NULL);

	_levelTOC.Validate();

	const DiskTOC::TOCEntry& entry = _levelTOC.GetTOCEntry(roomIndex+TOC_FIRST_ROOM_INDEX);
//	std::cout << "Toc = " << _levelTOC << std::endl;
//	std::cout << "roomIndex = " << roomIndex << std::endl;
//	std::cout << "loadRoomSlot seeking to : " << entry._offsetInDiskFile << std::endl;  // permanent offset index = 0
	_diskFile.SeekRandom(entry._offsetInDiskFile);  // permanent offset index = 0
	DBSTREAM3( casset << "roomoffset = " << entry._offsetInDiskFile << std::endl; )
	assert(entry._size % DiskFileCD::_SECTOR_SIZE == 0	);
	//char* streamBuffer = new (HALScratchLmalloc) char[entry._size];
	char* streamBuffer = new (HALLmalloc) char[entry._size];
	_diskFile.ReadBytes(streamBuffer,entry._size);
 	CHUNKHDR& chdr = *((CHUNKHDR*)streamBuffer);
#if DO_ASSERTIONS
	int roundedSize = chdr.size + (DiskFileCD::_SECTOR_SIZE-(chdr.size%DiskFileCD::_SECTOR_SIZE));  // round up to next sector size
#endif
	AssertMsg(roundedSize == entry._size, chdr.size << "," << entry._size << "," << roundedSize << ",toc = " << _levelTOC);

	int maxAsset = 0;
	{
		binistream roomStream((void*)(((char*)streamBuffer)+sizeof(CHUNKHDR)),chdr.size);
		roomStream.seekg(0);
		while(roomStream.getFilelen() > roomStream.tellg())
		{
			assert(roomStream.good());
			IFFChunkIter roomChunkIter(roomStream);
			maxAsset++;
			assert(maxAsset < 1000);	// arbitrary
		}
	}
	binistream roomStream((void*)(((char*)streamBuffer)+sizeof(CHUNKHDR)),chdr.size);

	_assets[slotNum] = new (_memory) AssetSlot(slotNum, roomIndex,roomStream, maxAsset, (void*)&_assetMemory[_cbRoomMemory*slotNum],_cbRoomMemory,_videoMemory);
	assert(ValidPtr(_assets[slotNum]));
	HALLmalloc.Free(streamBuffer);
}

//==============================================================================

void
AssetManager::FreeRoomSlot(int slotNum)
{
	DBSTREAM2( casset << "FreeRoom: " << slotNum << std::endl; )
	assert(slotNum >= 0);
	assert(slotNum < MAX_ACTIVE_SLOTS);
	assert(ValidPtr(_assets[slotNum]));

  	MEMORY_DELETE(_memory,_assets[slotNum],AssetSlot);
	_assets[slotNum] = NULL;
}

//==============================================================================

void
AssetManager::ReadAssetMap(binistream& mapStream)
{
	const int MAX_ASMP_SIZE = DiskFileCD::_SECTOR_SIZE * 4;  // kts abritrary

	IFFChunkIter mapChunkIter(mapStream);
	assert(mapChunkIter.GetChunkID().ID() == IFFTAG('A','S','M','P'));
	assert(mapChunkIter.BytesLeft() <=  MAX_ASMP_SIZE);  // didn't pre-allocate a large enough buffer, increase MAX_ASMP_SIZE above

	while(mapChunkIter.BytesLeft())
	{
		mapChunkIter.ReadBytes(&_assetStringMap[_assetStringMapEntries]._id,4);
		IFFChunkIter* stringIter = mapChunkIter.GetChunkIter(HALScratchLmalloc);
		assert(stringIter->GetChunkID().ID() == IFFTAG('S','T','R','\0'));
		assert(stringIter->BytesLeft() < 32);
		stringIter->ReadBytes(&_assetStringMap[_assetStringMapEntries]._name,stringIter->BytesLeft());
		MEMORY_DELETE(HALScratchLmalloc,stringIter,IFFChunkIter);
		_assetStringMapEntries++;
		assert(_assetStringMapEntries <= 300);
	}

}

//==============================================================================

binistream
AssetManager::GetAssetStream(packedAssetID asset) const  // return a binistream which will iterate the given asset
{
	DBSTREAM2( casset << "GetAssetStream: " << asset << std::endl; )
	// first figure out which list to look in
	int slotIndex = _callback.GetSlotIndex(asset.Room());
	DBSTREAM2( casset << "slotIndex =" << slotIndex << std::endl; )
	assert(slotIndex >= 0);
	assert(slotIndex < MAX_ACTIVE_SLOTS);

	return _assets[slotIndex]->LookupAsset(asset);
}

//=============================================================================

const void*
AssetManager::LookupAssetMemory(packedAssetID id)
{
   int slotIndex = _callback.GetSlotIndex(id.Room());
   return GetAssetSlot(slotIndex).LookupAssetMemory(id);
}

//=============================================================================

LookupTextureStruct
AssetManager::LookupTexture(const char* name, int32 userData)			// in streaming, this should be an asset ID
{
	DBSTREAM2( casset << "LookupTexture: " << std::endl; )

	assert(strlen(name));
	packedAssetID assetID(userData);
	DBSTREAM2( casset << "packedassetID = " << assetID << std::endl; )

	int slotIndex = _callback.GetSlotIndex(assetID.Room());
	AssetSlot& slot = GetAssetSlot(slotIndex);
	_RMUV* rmuv = slot.GetRMUV().GetRMUV(name);

	AssertMsg(ValidPtr(rmuv),"could not find texture <" << name <<"> in room " << assetID.Room());
	DBSTREAM2( casset << "returning " << *rmuv << std::endl; )
    return LookupTextureStruct(*((Texture*)rmuv),slot.GetTexturePixelMap());
}                 

//==============================================================================

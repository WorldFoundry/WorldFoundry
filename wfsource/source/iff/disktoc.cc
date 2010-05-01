//=============================================================================
// disktoc.cc:
// Copyright ( c ) 1998,99 World Foundry Group  
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
//	Abstract:
// Original Author: Kevin T. Seghetti
//=============================================================================

#include <iff/iffread.hp>
#include <cpplib/libstrm.hp>
#include <iff/disktoc.hp>

//=============================================================================

DiskTOC::DiskTOC()
{
	_toc = NULL;
}

//=============================================================================

DiskTOC::~DiskTOC()
{
	assert(ValidPtr(_toc));
	HALLmalloc.Free(_toc);
}

//=============================================================================

struct TOCENTRYONDISK
{
	int32 tag;
	int32 offset;
	int32 size;
};

//-----------------------------------------------------------------------------
// currently only works if toc is smaller than one sector

void
DiskTOC::LoadTOC(_DiskFile& diskFile, int32 tocOffset)
{
	DBSTREAM3( ciff << "AssetManager::LoadTOC" << std::endl; )
	assert(_toc == NULL);
	_tocEntries = 0;

	char* tocMem = new (HALScratchLmalloc) char[DiskFileCD::_SECTOR_SIZE];
	//assert(diskFile.FilePos() == tocOffset);
	// ok, read one sector from the file
	diskFile.SeekForward(tocOffset);
	diskFile.ReadBytes(tocMem,DiskFileCD::_SECTOR_SIZE);

	struct LVLHDR
	{
		int32 lvasTag;
		int32 fileSize;
//		int32 lvlTag;
//		int32 lvlSize;
//		int32 bogusID;
	};
#if DO_ASSERTIONS
	LVLHDR& hdr = *((LVLHDR*)tocMem);
	_name = hdr.lvasTag;
#endif
//	AssertMsg(hdr.lvasTag == 'SAVL',"lvasTag = " << (void*)hdr.lvasTag);
	// ok, assume the toc fits in one sector, and construct an iff chunk iterator for it
	binistream tocStream((void*)(tocMem+sizeof(LVLHDR)),DiskFileCD::_SECTOR_SIZE);

	IFFChunkIter tocChunkIter(tocStream);
	assert(tocChunkIter.BytesLeft() <=  DiskFileCD::_SECTOR_SIZE);
	int entryCount = tocChunkIter.BytesLeft()/sizeof(TOCENTRYONDISK);
#pragma message ("KTS " __FILE__ ": make this use a Memory& for the current level")
	_toc = new (HALLmalloc) TOCEntry[entryCount];

	// loop through converting TOCENTRYONDISK to TOCEntry
	while(tocChunkIter.BytesLeft())
	{
		TOCENTRYONDISK tocOnDisk;
		tocChunkIter.ReadBytes(&tocOnDisk,sizeof(tocOnDisk));

//#if DO_ASSERTIONS
//		if(_tocEntries == TOC_LEVEL_INDEX)
//			assert(tocOnDisk.tag == '\0LVL');
//		if(_tocEntries == TOC_PERM_INDEX)
//			assert(tocOnDisk.tag == 'MREP');
//#endif
		_toc[_tocEntries]._offsetInDiskFile = tocOnDisk.offset+tocOffset;
//#pragma message ("KTS " __FILE__ ": turn this assertion back on when iffcomp is fixed")
		assert(tocOnDisk.size >= 0); 
		_toc[_tocEntries]._size = tocOnDisk.size + (DiskFileCD::_SECTOR_SIZE-(tocOnDisk.size%DiskFileCD::_SECTOR_SIZE));  // round up to next sector size
		_tocEntries++;
		assert(_tocEntries <= entryCount);
	}
	HALScratchLmalloc.Free(tocMem,DiskFileCD::_SECTOR_SIZE);
	AssertMsg(_tocEntries == entryCount,"_tocEntries = " << entryCount << ", entryCount = " << entryCount);
	Validate();
}

//=============================================================================

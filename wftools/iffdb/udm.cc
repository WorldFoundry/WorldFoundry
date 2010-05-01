//==============================================================================
// udm.cc: Copyright (c) 1996-1999, World Foundry Group  
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
// Description: DataType class hierarchy
// Original Author: Kevin T. Seghetti
//==============================================================================

DataType_I32::DataType_I32(IFFChunkIter& iter)
{
	// read all chunks in here
}


DataType_I32::Read(binistream& data)
{
	data >> _value;
}

//==============================================================================

DataType_STRU::DataType_STRU(IFFChunkIter& iter)
{
	// read all chunks in here
}


DataType_STRU::Read(binistream& data)
{
	data >> _value;
}

//==============================================================================

DataType_IFFC::DataType_IFFC(IFFChunkIter& iter)
{

}


DataType_IFFC::Read(binistream& data)
{
	IFFChunkIter& subChunk = *chunkIter.GetChunkIter();

}

//==============================================================================

DataType*
ConstructDataType(IFFChunkIter& chunkIter)
{
	DataType* dt = NULL;
	switch (chunkIter.GetChunkID().ID())
	{
		case 'ARRY':
			dt = new DataType_Array(chunkIter);
			break;
		case 'I32':
			dt = new DataType_I32(chunkIter);
			break;
		default:
			assert(0);
			break;
	};
	return dt;
}

//==============================================================================

DataTypoe*
ConstructUDMTreeFromFile(binistream& udmFile, binistream& dataFile)
{
	DataType* dt = NULL;
	IFFChunkIter chunkIter(udmFile);

	assert(udmFile.GetChunkID() == ChunkID('UDM');
	dt = ConstructDataType(udmFile);
	dt->Read(dataFile);
	return dt;
}

//==============================================================================

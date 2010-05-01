//-----------------------------------------------------------------------------
// animcomp.cc:		Animation compressor class for use in STRIPPER.EXE
// Created 11/14/97 19:10 by Phil Torre
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

#include "animcomp.hp"
#include <iff/iffread.hp>
#include <iffwrite/iffwrite.hp>

//-----------------------------------------------------------------------------

AnimationCompressor::AnimationCompressor( IFFChunkIter& animChunkIter )
{
	assert(animChunkIter.GetChunkID() == 'MINA');

	// First, find the AHDR chunk with the frame count (and some day, the sample rate)
	_frameCount = 0;
	_vertexCount = 0;

	while (_frameCount == 0)
	{
		assert(animChunkIter.BytesLeft() > 0);
		IFFChunkIter* subChunkIter = animChunkIter.GetChunkIter();
		if (subChunkIter->GetChunkID() == 'RDHA')
		{
			assert(subChunkIter->Size() == sizeof(int16));
			subChunkIter->ReadBytes(&_frameCount, sizeof(int16));
		}
		delete subChunkIter;
	}

	// All that remains in this ANIM chunk should be <frameCount> of VRTX subchunks
	while(animChunkIter.BytesLeft() > 0)
	{
		IFFChunkIter* subChunkIter = animChunkIter.GetChunkIter();

		if (subChunkIter->GetChunkID() == 'XTRV')
		{
			std::vector<Vertex3D> frameVector;
			// if the following assertion fails, the input file contains a bogus VRTX chunk
			assert( (_vertexCount == 0) || (_vertexCount == (subChunkIter->Size() / sizeof(Vertex3D))) );
			_vertexCount = subChunkIter->Size() / sizeof(Vertex3D);
			assert(subChunkIter->Size()%sizeof(Vertex3D) == 0);
			assert(_vertexCount);
			Vertex3D tempVertex;
			for (int vertexIdx=0; vertexIdx < _vertexCount; ++vertexIdx)
			{
				subChunkIter->ReadBytes(&tempVertex, sizeof(Vertex3D));
				frameVector.push_back( tempVertex );
			}
			assert(subChunkIter->BytesLeft() == 0);
			_animVertexList.push_back( frameVector );
		}
		else
			cerror << "WARNING -- Found chunks after the AHDR chunk that weren't VRTXs!" << endl;

		delete subChunkIter;
	}

	int framesCreated = _animVertexList.size();

	assert( _animVertexList.size() == _frameCount );
}

//-----------------------------------------------------------------------------
// This sucks:  I have to write operator< and operator== for the _ChannelOnDiskEntry
// struct, otherwise Microsoft's std::vector won't accept them.
#pragma message("This sucks.  Change it.  (animcomp.cc, _WriteChannel())")

inline bool operator < (const _ChannelOnDiskEntry& me, const _ChannelOnDiskEntry& him) { return false; }
inline bool operator == (const _ChannelOnDiskEntry& me, const _ChannelOnDiskEntry& him) { return false; }

void
AnimationCompressor::_WriteChannel( int32* sampleArray, int frameRate, IffWriter& iffOut)
{
	_ChannelOnDisk outputChannel;

	// Check to see if the data is constant
	bool isConstant(true);
	for (int frameIdx=1; (frameIdx < _frameCount) && isConstant; ++frameIdx)
	{
		if (sampleArray[frameIdx] != sampleArray[0])
			isConstant = false;
	}

	if (isConstant)
	{
		_ChannelOnDiskEntry theEntry;
		outputChannel.compressorType = CONSTANT_COMPRESSION;
		outputChannel.endTime = 0;
		outputChannel.numKeys = 1;
		theEntry.time = Scalar::zero;
		theEntry.value = sampleArray[0];

		iffOut.out_mem( &outputChannel, sizeof(_ChannelOnDisk) );
		iffOut.out_mem( &theEntry, sizeof(_ChannelOnDiskEntry) );
		return;
	}

	// OK, it's not constant, so we have to do some actual work
	assert(_frameCount > 1);
	Scalar slope = (sampleArray[1] - sampleArray[0]) * frameRate;
	_ChannelOnDiskEntry thisEntry;
	std::vector<_ChannelOnDiskEntry> entries;

	// grab the first sample
	thisEntry.time = Scalar::zero;
	thisEntry.value = sampleArray[0];
	entries.push_back( thisEntry );

	for (frameIdx=1; frameIdx < _frameCount-1; ++frameIdx)
	{
		float slopeFromLastKey = sampleArray[frameIdx] - sampleArray[frameIdx-1];
		float slopeToNextKey   = sampleArray[frameIdx+1] - sampleArray[frameIdx];

		if ( ((slopeFromLastKey < 0) && (slopeToNextKey > 0)) ||
			 ((slopeFromLastKey > 0) && (slopeToNextKey < 0)) )
		{
			thisEntry.time = Scalar(frameIdx) / Scalar(frameRate);
			thisEntry.value = sampleArray[frameIdx];
			entries.push_back( thisEntry );
		}
	}

	// Add the last sample
	thisEntry.time = Scalar(_frameCount) / Scalar(frameRate);
	thisEntry.value = sampleArray[_frameCount-1];
	entries.push_back( thisEntry );

	// Now dump the actual _ChannelOnDisk data
	outputChannel.compressorType = LINEAR_COMPRESSION;
	outputChannel.endTime = thisEntry.time;
	outputChannel.numKeys = entries.size();
	iffOut.out_mem( &outputChannel, sizeof(_ChannelOnDisk) );
	for (int entryIdx=0; entryIdx < entries.size(); ++entryIdx)
	{
		thisEntry = entries[entryIdx];
		iffOut.out_mem( &thisEntry, sizeof(_ChannelOnDiskEntry) );
	}
}

//-----------------------------------------------------------------------------

void
AnimationCompressor::Save( IffWriter& iffOut )
{
	// Start the CANM chunk, then start writing _ChannelOnDisk structs
	iffOut.enterChunk( ID('CANM') );

#pragma message("NOTE: Still using a fixed sample rate of 10 Hz for anim compression...")
	int frameRate = 10;

	cout << "=======================================================================" << endl;
	cout << "= WARNING!!! Stripper.exe is zeroing all u, v, and Color values!!!!!! =" << endl;
	cout << "=======================================================================" << endl;

	for (int vertexIdx=0; vertexIdx < _vertexCount; ++vertexIdx)
	{
		// Build and output 6 channels for this vertex, in order: u,v,color,x,y,z
		int32* uArray = new int32[_frameCount];
		int32* vArray = new int32[_frameCount];
		int32* cArray = new int32[_frameCount];
		int32* xArray = new int32[_frameCount];
		int32* yArray = new int32[_frameCount];
		int32* zArray = new int32[_frameCount];

		for (int frameIdx=0; frameIdx < _frameCount; ++frameIdx)
		{
// SEE WARNING ABOVE!!!
//			uArray[frameIdx] = ((_animVertexList[frameIdx])[vertexIdx]).u.AsLong();
//			vArray[frameIdx] = ((_animVertexList[frameIdx])[vertexIdx]).v.AsLong();
//			cArray[frameIdx] = ((_animVertexList[frameIdx])[vertexIdx]).color.AsLong();

			uArray[frameIdx] = 0;
			vArray[frameIdx] = 0;
			cArray[frameIdx] = 0;
			xArray[frameIdx] = ((_animVertexList[frameIdx])[vertexIdx]).position.X().AsLong();
			yArray[frameIdx] = ((_animVertexList[frameIdx])[vertexIdx]).position.Y().AsLong();
			zArray[frameIdx] = ((_animVertexList[frameIdx])[vertexIdx]).position.Z().AsLong();
		}

		_WriteChannel(uArray, frameRate, iffOut);
		_WriteChannel(vArray, frameRate, iffOut);
		_WriteChannel(cArray, frameRate, iffOut);
		_WriteChannel(xArray, frameRate, iffOut);
		_WriteChannel(yArray, frameRate, iffOut);
		_WriteChannel(zArray, frameRate, iffOut);

		delete uArray;
		delete vArray;
		delete cArray;
		delete xArray;
		delete yArray;
		delete zArray;
	}


	iffOut.exitChunk();
}

//-----------------------------------------------------------------------------

//=============================================================================
// Anim.cc:
// Copyright ( c ) 1997,1998,1999,2000,2001,2002 World Foundry Group  
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
//
// Original Author: Kevin T. Seghetti
//============================================================================

#include "anim.hp"
#include <hal/hal.h>
#include <iff/iffread.hp>
#include "channel.hp"

//============================================================================

AnimateRenderObject3D::AnimateRenderObject3D(Memory& memory, IFFChunkIter& animIter,const RenderObject3D& renderObject)
{

// This really isn't a functional switch, as the private data of this class has to be changed also
// before the old code will work again...
#ifdef READ_INTERMEDIATE_ANIM_FORMAT
	AssertMsg(animIter.GetChunkID().ID() == IFFTAG('A','N','I','M'),"chunk = " << animIter.GetChunkID());

	// create AnimateRenderObject3D from model on disk data
	uint16 vertexCount = renderObject.GetVertexCount();
	int frame=0;
#if DO_ASSERTIONS
	bool headerLoaded = false;
	_vertexCount = vertexCount;
#endif
	Vertex3D* vertexList = NULL;

	while(animIter.BytesLeft() > 0)
	{
		IFFChunkIter* chunkIter = animIter.GetChunkIter(HALScratchLmalloc);
//		ciffread << "chunkid = " << chunkIter->GetChunkID() << std::endl;
		switch(chunkIter->GetChunkID().ID())
		{
			case IFFTAG('A','H','D','R'):			    // anim header
				assert(!headerLoaded);
				assert(chunkIter->Size() == 2 );
				chunkIter->ReadBytes(&_frameCount,2);
				RangeCheck(0,vertexCount,5000);  		// arbitrary
				RangeCheckExclusive(0,_frameCount,500);  		// arbitrary
				_animArray = new (memory) Vertex3D[vertexCount*_frameCount];
				assert(ValidPtr(_animArray));
				// kts put this into the header
				_rate = SCALAR_CONSTANT(10);
#if DO_ASSERTIONS
				headerLoaded = true;
#endif
				break;
			case IFFTAG('V','R','T','X'):
			{
//				ciffread << "VRTX: ";
				assert(headerLoaded);
				assert(vertexCount == chunkIter->Size()/sizeof(Vertex3DOnDisk ));
//				ciffread << "Count = " << vertexCount << std::endl;
				assert(chunkIter->Size()%sizeof(Vertex3DOnDisk) == 0);
				RangeCheck(0,vertexCount,5000);  		// arbitrary
				RangeCheck(0,frame,_frameCount);
				vertexList = &_animArray[frame*vertexCount];
				assert(ValidPtr(vertexList));

				Vertex3DOnDisk tempVertex;
				for(int count=0;count<vertexCount;count++)
				{
					chunkIter->ReadBytes(&tempVertex,sizeof(Vertex3DOnDisk));

					vertexList[count].u = Scalar::FromFixed32(tempVertex.u);
					vertexList[count].v = Scalar::FromFixed32(tempVertex.v);
					vertexList[count].color = tempVertex.color;
					vertexList[count].position = Vector3_PS(Scalar::FromFixed32(tempVertex.x),Scalar::FromFixed32(tempVertex.y),Scalar::FromFixed32(tempVertex.z));

				}
//				chunkIter->ReadBytes(vertexList,chunkIter->Size());
				assert(chunkIter->BytesLeft() == 0);
				++frame;
				break;
			}
			default:
//				ciffread << "ignoring chunk with ID of " << chunkIter->GetChunkID() << std::endl;
				break;
		}
		MEMORY_DELETE(HALScratchLmalloc,chunkIter,IFFChunkIter);
		//delete chunkIter;
//		animIter.NextChunk();
//		ciffread << "animIter bytes left = " << animIter.BytesLeft() << std::endl;
	}
	assert(frame == _frameCount);
	assert( animIter.BytesLeft() == 0 );
	Validate();

#else
	AssertMsg(animIter.GetChunkID() == IFFTAG('C','A','N','M'),"chunk = " << animIter.GetChunkID());

extern Channel* ConstructChannelObject( _ChannelOnDisk*, Memory& );

#pragma message("NOTE: AnimateRenderObject3D constructor assumes SIX CHANNELS PER VERTEX")

	int channelCount = renderObject.GetVertexCount() * 6;
	_channels.SetMax( channelCount );

	// Load the big blob of data from the CANM chunk
	_channelData = new (memory) char[animIter.BytesLeft()];
	animIter.ReadBytes( _channelData, animIter.BytesLeft() );

	// Now walk through it constructing Channel objects
	char* channelDataPtr = _channelData;
	int numKeys;
	_endTime = Scalar::zero;

	for (int channelIndex=0; channelIndex < channelCount; ++channelIndex)
	{
		numKeys = ((_ChannelOnDisk*)channelDataPtr)->numKeys;
		_endTime = _endTime.Max( ((_ChannelOnDisk*)channelDataPtr)->endTime );
		_channels.Add( ConstructChannelObject( (_ChannelOnDisk*)channelDataPtr, memory ) );
		channelDataPtr += sizeof(_ChannelOnDisk) + (sizeof(_ChannelOnDiskEntry) * numKeys);
	}
#endif	// !READ_INTERMEDIATE_ANIM_FORMAT
}

//============================================================================

void
AnimateRenderObject3D::Animate(Scalar time,RenderObject3D& renderObject)
{
#ifdef READ_INTERMEDIATE_ANIM_FORMAT

	Vertex3D* vertexList = renderObject.GetWrittableVertexList();
	assert(ValidPtr(vertexList));

	int32 vertexCount = renderObject.GetVertexCount();
	assert(vertexCount == _vertexCount );

	int frame = (time*_rate).WholePart() % _frameCount;

	long* source = (long*)&_animArray[frame*vertexCount];
	long* dest = (long*)vertexList;
	assert(((vertexCount*sizeof(Vertex3D)) % 4) == 0);
	int count = (vertexCount*sizeof(Vertex3D)) / 4;
	for(int memIndex=0;memIndex<count;memIndex++)
		*dest++ = *source++;
//	memcpy(vertexList,&_animArray[frame*vertexCount],vertexCount*sizeof(Vertex3D));

#else

	Vertex3D* vertexList = renderObject.GetWrittableVertexList();
	assert(ValidPtr(vertexList));

	int32 vertexCount = renderObject.GetVertexCount();
	Vertex3D tempVertex;
	int channelIdx = 0;

#pragma message("HEY, Scalar really could use a modulo operator!")
	Scalar wholePart( (time / _endTime).WholePart() << 16 );
	Scalar clippedTime = time - (wholePart * _endTime);

	for (int vertexIdx=0; vertexIdx < vertexCount; ++vertexIdx)
	{
		assert(_channels.Size() > channelIdx);


		tempVertex.u = _channels[channelIdx++]->Value(clippedTime);
		tempVertex.v = _channels[channelIdx++]->Value(clippedTime);
		tempVertex.color = Color( _channels[channelIdx++]->Value(clippedTime) );
		tempVertex.position = Vector3( _channels[channelIdx++]->Value(clippedTime),
									   _channels[channelIdx++]->Value(clippedTime),
									   _channels[channelIdx++]->Value(clippedTime) );

		vertexList[vertexIdx] = tempVertex;
	}
#endif	// READ_INTERMEDIATE_ANIM_FORMAT
}

//============================================================================

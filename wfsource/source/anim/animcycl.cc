//=============================================================================
// AnimCycl.cc:
// Copyright ( c ) 1997,1999,2000,2001,2003 World Foundry Group  
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

#include <anim/animcycl.hp>
#include <iff/iffread.hp>

//=============================================================================

AnimationCycleArray::AnimationCycleArray(Memory& memory, binistream& input, const RenderObject3D& object) :
 _cycleIndex(0)
{
#if DO_ASSERTIONS
	for(int index=0;index<AnimationManager::MAX_ANIMATION_CYCLES;index++)
		_cycles[index] = NULL;
#endif

	if(input.getFilelen()>input.tellg())
	{
		int16 cycleIndexArray[AnimationManager::MAX_ANIMATION_CYCLES];
#if DO_ASSERTIONS
		for(int debugcycleIndex=0;debugcycleIndex<AnimationManager::MAX_ANIMATION_CYCLES;debugcycleIndex++)
			cycleIndexArray[debugcycleIndex] = -1;
#endif

		// if there is any more data, it must be animation
		// so the next chunk has to be an cycle map (CYMP)
		{
			assert(input.good());
			// CYMP is an array of int32's, mapping all possible cycles the game can request to available cycles for this particular mesh
			IFFChunkIter animMapIter(input);
//			DBSTREAM3( cgraphics << "Reading CYMP chunk" << std::endl; )
			AssertMsg(animMapIter.GetChunkID().ID() == IFFTAG('C','Y','M','P'),"chunk = " << animMapIter.GetChunkID());
			int _cycleCount = animMapIter.Size()/sizeof(int16);
			AssertMsg(_cycleCount == AnimationManager::MAX_ANIMATION_CYCLES,"_cycleCount = " << _cycleCount << ", MAX = " << AnimationManager::MAX_ANIMATION_CYCLES);
			animMapIter.ReadBytes(cycleIndexArray,animMapIter.Size());
			assert(animMapIter.BytesLeft() == 0);
		}

#pragma message ("KTS: no redundancy checking, making new animations for each object")
		int32 currentCyclePtrIndex = 0;
		AnimateRenderObject3D* cyclePtrArray[AnimationManager::MAX_ANIMATION_CYCLES];  // array of pointers to animateRenderObject3D's
#if DO_ASSERTIONS
		for(int cyclePtrIndex=0;cyclePtrIndex<AnimationManager::MAX_ANIMATION_CYCLES;cyclePtrIndex++)
			cyclePtrArray[cyclePtrIndex] = NULL;
#endif
		while(input.getFilelen() > input.tellg())
		{
			assert(input.good());
			assert(currentCyclePtrIndex < AnimationManager::MAX_ANIMATION_CYCLES);
			assert(cyclePtrArray[currentCyclePtrIndex] == NULL);
			IFFChunkIter animIter(input);
//			DBSTREAM3( cgraphics << "Reading CANM chunk" << std::endl; )
#ifdef READ_INTERMEDIATE_ANIM_FORMAT
			AssertMsg(animIter.GetChunkID().ID() == IFFTAG('A','N','I','M'),"chunk = " << animIter.GetChunkID());
#else
			AssertMsg(animIter.GetChunkID().ID() == IFFTAG('C','A','N','M'),"chunk = " << animIter.GetChunkID());
#endif
			cyclePtrArray[currentCyclePtrIndex] = new (memory) AnimateRenderObject3D(memory,animIter,object);
			assert(ValidPtr(cyclePtrArray[currentCyclePtrIndex]));
			currentCyclePtrIndex++;
		}

		for(int cycleIndex=0;cycleIndex<AnimationManager::MAX_ANIMATION_CYCLES;cycleIndex++)
		{
			assert(cycleIndexArray[cycleIndex] < currentCyclePtrIndex);
			assert(ValidPtr(cyclePtrArray[cycleIndexArray[cycleIndex]]));
			_cycles[cycleIndex] = cyclePtrArray[cycleIndexArray[cycleIndex]];
		}
//	_cycleIndex = AnimationManager::RUN_HARD;

#if DO_ASSERTIONS
		for(int index=0;index<AnimationManager::MAX_ANIMATION_CYCLES;index++)
			assert(_cycles[index] != NULL);
#endif
	}
	else
	{
		DBSTREAM1( cwarn << "RenderActor3DAnimates constructed with no animations!" << std::endl; )
		for(int index=0;index<AnimationManager::MAX_ANIMATION_CYCLES;index++)
			_cycles[index] = NULL;
	}
}

//=============================================================================

AnimationCycleArray::~AnimationCycleArray()
{
}

//=============================================================================



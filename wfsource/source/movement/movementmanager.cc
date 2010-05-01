//=============================================================================
// movementmanager.cc:
// Copyright ( c ) 2002,2003 World Foundry Group  
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
// Original Author: Kevin Seghetti
// kts note: several of these functions could be inline, but right now I don't
// want to include the headers they need in the inline file
// ===========================================================================

#include "movementmanager.hp"
#include "movement.hp"

#include <oas/movement.h>
#include <cpplib/libstrm.hp>

//==============================================================================

Memory* MovementManager::_memory = NULL;

//==============================================================================

void 
MovementManager::init(MovementObject& movementObject )
{
   assert(ValidPtr(_currentMovementHandler));
   _currentMovementHandler->init(*this, movementObject);
}

bool 
MovementManager::update(MovementObject& movementObject, const BaseObjectList& baseObjectList )
{
	assert(!movementObject.GetPhysicalAttributes().HasRunUpdate());		// bail if this actor has already been updated
	movementObject.GetWritablePhysicalAttributes().HasRunUpdate(true);	// set flag so we won't be updated again this frame
    assert(ValidPtr(_currentMovementHandler));
    return _currentMovementHandler->update(*this, movementObject, baseObjectList);
}


void 
MovementManager::predictPosition(MovementObject& movementObject, const Clock& currentTime, const BaseObjectList& baseObjectList)
{
   if ( !_currentMovementHandler )
   	InitMovementHandler(movementObject);

   if (!movementObject.GetPhysicalAttributes().HasRunPredictPosition())
   {
       movementObject.GetWritablePhysicalAttributes().HasRunPredictPosition(true);
       assert(ValidPtr(_currentMovementHandler));
       _currentMovementHandler->predictPosition(*this, movementObject,currentTime, baseObjectList);
   }
}

void 
MovementManager::SetStunTime(Scalar duration, const Clock& currentTime)
{
   assert(ValidPtr(_currentMovementHandler));
   _currentMovementHandler->SetStunTime(*this, currentTime.Current()+duration);
}

//==============================================================================

MovementManager::MovementManager(const _Movement* moveBlock) :
   _movementBlock(moveBlock),
	//_currentMovementHandler( &theNullHandler ),
	_currentMovementHandler( NULL),
	_movementHandlerData( NULL ),
   _movementDataSize(0)
{
   if(moveBlock)
      assert(ValidPtr(moveBlock));
}

//==============================================================================

MovementManager::~MovementManager()
{
	// Delete this object's movement handler data struct, if it has one
	if ( _movementHandlerData )
	{
		DBSTREAM3( cmovement << "MovementManager destructor: deleting _movementHandlerData" << std::endl; )
		assert( ValidPtr(_movementHandlerData) );
      assert(ValidPtr(_memory));
		MEMORY_DELETE(*_memory, _movementHandlerData, MovementHandlerData);
	}
}

//==============================================================================

void 
MovementManager::Validate() const
{
   if(_movementHandlerData)
   {
      assert(_movementDataSize);
      assert(_currentMovementHandler);
      assert(_currentMovementHandler->DataSize() == _movementDataSize);
   }
   else
      assert(_movementDataSize == 0);
}

//==============================================================================

void
MovementManager::InitMovementHandler(MovementObject& movementObject) 
{
	if ( !_currentMovementHandler )
	{

      if(_movementBlock == NULL)
      {
         _currentMovementHandler = &theNullHandler;
      }
      else
      {
         DBSTREAM3( cmovement << movementObject; )
         assert(_movementHandlerData == NULL);
         assert(ValidPtr(_movementBlock));

         MovementHandlerEntry* mhe = &MovementHandlerArray[0];
         while(mhe->mobility != -1 && _currentMovementHandler == NULL)
         {
            if(mhe->mobility == _movementBlock->Mobility)
               _currentMovementHandler = mhe->handler;
            mhe++;

         }
         AssertMsg(_currentMovementHandler, movementObject << " has unknown Mobility of " << _movementBlock->Mobility);
      }

      AssertMsg( ValidPtr(_currentMovementHandler), *this );

      // kts this entire system needs to be redesigned (right now I am focusing
      // on reducing interdependecies, once that is done changing this sort of thing will be eaiser)
      assert(_movementHandlerData ==NULL);
      _movementDataSize = _currentMovementHandler->DataSize();
      if(_movementDataSize)
      {
         if(!_memory)
         {
            _memory = &movementObject.GetMemory();
            std::cout << "this = " << this << ",setting _memory to " << _memory << std::endl;
         }

         // kts this doesn't work because the destructors never run
         //assert(_memory == &movementObject.GetMemory());

         _movementHandlerData = (MovementHandlerData*) new (movementObject.GetMemory()) char[_movementDataSize];
         assert(ValidPtr(_movementHandlerData));
         memset(_movementHandlerData, 0, _movementDataSize);
      }

      _currentMovementHandler->init(*this,movementObject);
	}
}

//==============================================================================

MovementHandlerData*
MovementManager::SetMovementHandlerData(MovementHandlerData* handlerData)
{
   assert(_movementHandlerData == NULL);
	_movementHandlerData = handlerData;
	//memset( handlerData, 0, sizeof(MovementHandlerData));
	return _movementHandlerData;
}

//==============================================================================

MovementHandlerData*
MovementManager::InitMovementHandlerData(Memory& memory)
{
	_movementHandlerData = new (memory) MovementHandlerData;
	memset( _movementHandlerData, 0, sizeof(MovementHandlerData));
	return _movementHandlerData;
}

//==============================================================================

void
MovementManager::SetMovementHandler(MovementHandler* newHandlerAddr, MovementObject& movementObject)
{                                                  
    AssertMsg( ValidPtr( newHandlerAddr ), *this );
    _currentMovementHandler = newHandlerAddr;
    _currentMovementHandler->init( *this, movementObject );
}

//==============================================================================


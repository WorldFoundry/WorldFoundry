//==============================================================================
// MovementObject.cc:
// Copyright (c) 2003 World Foundry Group  
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
// Description: The MovementObject is a virtual base class (derived from PhysicalObject), 
// a client needs to derive from this and fill out the virutual functions to be able to 
// use the movement handlers and collision system
// This is how I am able to decouple the movement system from all of the other pieces it
// needs to access
// Original Author: Kevin T. Seghetti
//==============================================================================

#include <pigsys/pigsys.hp>
#include <movement/movementobject.hp>
#include <timer/clock.hp>
#include <cpplib/libstrm.hp>
#include <cpplib/algo.hp>

//==============================================================================

MovementObject::~MovementObject()
{
}

//==============================================================================

#if SW_DBSTREAM >= 1

std::ostream&
operator << ( std::ostream& s, const MovementObject& obj )
{
	s << "MovementObject:" << std::endl;
   obj.Print(s);
	return s;
}

#endif // SW_DBSTREAM >= 1

//==============================================================================''

void 
MovementObject::predictPosition(const Clock& /*currentTime*/)
{
    assert(0);
}


void 
MovementObject::update()
{
    assert(0);

}

void 
MovementObject::DoneWithPhysics()
{
	DBSTREAM5( cmovement << "MovementObject::DoneWithPhysics: " << *this << std::endl; )

    GetWritablePhysicalAttributes().Update();    
	GetWritablePhysicalAttributes().HasRunUpdate(false);				// clear these flags for next frame
	GetWritablePhysicalAttributes().HasRunPredictPosition(false);
}

//==============================================================================

void 
MovementObject::MovementStateChanged( const MovementObject::EMovementState /*newState*/ )
{
   // do nothing, override this method if you want to know when the movement state changes
}

//==============================================================================

class ObjectPredictPosition
{
public:
    ObjectPredictPosition(const Clock& clock) :
    _clock(clock)
    {
    }
	inline void operator()( BaseObject& bo )
	{
      MovementObject* object = dynamic_cast<MovementObject*>(&bo);
		assert(ValidPtr(object));
		object->predictPosition(_clock);
	}
private:
    const Clock& _clock;
};

//==============================================================================

void 
PredictPosition(BaseObjectIteratorWrapper iter, const Clock& currentTime)
{
	for_each( iter,ObjectPredictPosition(currentTime));
}

//==============================================================================

class ObjectUpdate
{
public:
    ObjectUpdate(const Clock& clock) :
    _clock(clock)
    {
    }

	inline void operator() (BaseObject& bo)
	{
      MovementObject* object = dynamic_cast<MovementObject*>(&bo);
		assert(ValidPtr(object));
		if ((!object->GetPhysicalAttributes().HasRunUpdate()))
		{
			DBSTREAM1( cmovement << "UpdatePhysics: updating object " << *object << std::endl; )
			assert(!object->GetPhysicalAttributes().HasRunUpdate());
		 	object->update();
		}
	}
    const Clock& _clock;
};

class ObjectDoneWithPhysics
{
public:
	inline void operator() (BaseObject& bo)
	{
      MovementObject* object = dynamic_cast<MovementObject*>(&bo);
		assert(ValidPtr(object));
		object->DoneWithPhysics();
	}
};

//==============================================================================

void 
UpdatePhysics(BaseObjectIteratorWrapper iter, const Clock& currentTime)
{
    BaseObjectIteratorWrapper doneIter(iter);

	for_each( iter,ObjectUpdate(currentTime));
	for_each( doneIter,ObjectDoneWithPhysics());
}

//==============================================================================


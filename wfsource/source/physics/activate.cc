//==============================================================================
// activate.cc
// Copyright ( c ) 2000,2001,2002,2003 World Foundry Group  
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

#include <physics/physicalobject.hp>
#include <baseobject/commonblock.hp>
#include <cpplib/libstrm.hp>
#include <oas/activate.ht>
#include "activate.hp"

//==============================================================================

Activation::Activation(const _Activation& activation, const CommonBlock& commonBlock)
{
	DBSTREAM1( cnull << "ActivatedByActor " << activation.ActivatedByActor << std::endl; )
	DBSTREAM1( cnull << "ActivatedByClass " << activation.ActivatedByClass << std::endl; )
	DBSTREAM1( cnull << "ActivatedByObjectList " << activation.ActivatedByObjectList << std::endl; )

#if DO_ASSERTIONS

	switch(activation.ActivatedBy)
	{
		case 1: // actor
			//AssertMsg( activation.ActivatedByActor >= 0, *this << " is activated by Actor but none is specified" );
			AssertMsg( activation.ActivatedByActor >= 0, "object is activated by Actor but none is specified" );
			break;

		case 2: // class
			AssertMsg( activation.ActivatedByClass >= 0, "object is activated by Class but none is specified" );
			break;

		case 3: // list
			AssertMsg( activation.ActivatedByObjectList > 0, "object is activated by ObjectList but none is specified" );
			break;

		case 0: // all
			// no assert, fields not used.
			break;

		default:
			AssertMsg(0, ": Unknown activation trigger!");
			break;
	}
#endif // DO_ASSERTIONS
	if ( activation.ActivatedByObjectList )
      _objList = (int32*)commonBlock.GetBlockPtr(activation.ActivatedByObjectList);
	else
		_objList = NULL;
}

//==============================================================================

BaseObject*
Activation::Activated( const PhysicalAttributes& myPa, BaseObjectIteratorWrapper objectIter, const _Activation& activation, BaseObjectList& masterObjectList ) const
{
	switch ( activation.ActivatedBy )
	{
		case 0:
		{	// All Actors
			while ( !objectIter.Empty() )
			{
            PhysicalObject* po;
				po = dynamic_cast<PhysicalObject*>(&(*objectIter));
				//DBSTREAM1( ccollision << std::endl << "checking against actor #" << theLevel->GetActorIndex( po ); )
				assert( ValidPtr( po ) );
				const PhysicalAttributes& pa = po->GetPhysicalAttributes();

				if ( pa.CheckCollision( myPa ) )
            {
					return po;
            }

				++objectIter;
			}
			break;
		}

		case 1:
		{	// by Actor
			AssertMsg( activation.ActivatedByActor != -1, "activated non-existant actor" << std::endl );
         BaseObject* colObject = masterObjectList[activation.ActivatedByActor];
			if ( colObject )
			{
				//AssertMsg( colObject, *this << " activated by actor " << *colObject << " activated non-existant actor" << std::endl );
            PhysicalObject* po = dynamic_cast<PhysicalObject*>(colObject);
				const PhysicalAttributes& pa = po->GetPhysicalAttributes();
				if ( pa.CheckCollision( myPa ) )
            {
					return colObject;
            }
			}
			break;
		}

		case 2:
		{	// by Class
			DBSTREAM1( ccollision << "(ActBox::class) Checking actors with class = " << activation.ActivatedByClass << std::endl; )

			while ( !objectIter.Empty() )
			{
				PhysicalObject* po = dynamic_cast<PhysicalObject*>(&(*objectIter));
				assert( ValidPtr( po ) );
				if ( activation.ActivatedByClass == po->kind() )
				{
					//DBSTREAM3( ccollision << std::endl << "checking against actor #" << theLevel->GetActorIndex( po ); )
					const PhysicalAttributes& pa = po->GetPhysicalAttributes();

					if ( pa.CheckCollision( myPa) )
               {
                  return po;
               }
				}

			++objectIter;
			}

		break;
        }

		case 3:
        {       // Object List
			DBSTREAM1( cerror << "Activation by object list unimplemented" << std::endl; )
			//DBSTREAM1( cerror << " invoked by object #" << theLevel->GetObjectIndex( this ) << std::endl; )
			assert( 0 );	// Untested
			DBSTREAM3( ccollision << "(ActBox::ObjectList) actors" << std::endl; )
			break;
        }

		default:
			assert( 0 );
		}

	return NULL;
}


//==============================================================================

bool
Activation::IsActivated(const _Activation& activation, PhysicalObject* colObject, BaseObjectList& masterObjectList ) const
{
	bool bActivated = false;

	switch ( activation.ActivatedBy )
		{
		case 0:
			{ // All Objects
			bActivated = true;
			break;
			}

		case 1:
			{ // by Object
			bActivated = ( colObject == masterObjectList[ activation.ActivatedByActor] );
			break;
			}

		case 2:
			{ // by Class
			bActivated = ( activation.ActivatedByClass == colObject->kind() );
			break;
			}

		case 3:
			{ // Object list
			int32* pObjList = _objList;

			AssertMsg( *pObjList != -1, *colObject << ": No choices made for ActivatedBy [Object List]" );
			do
				{
				DBSTREAM3( cnull << "Checking against " << *pObjList << std::endl; )
				if ( colObject == masterObjectList[ *pObjList] )
					{
					bActivated = true;
					break;
					}
				++pObjList;
				}
			while ( *pObjList != -1 );
			break;
			}
		}

	return bActivated;
}

//==============================================================================


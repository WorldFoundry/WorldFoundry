//============================================================================
// Generator.cc:
// Copyright ( c ) 1995,1996,1997,1999,2000,2002l,2003 World Foundry Group  
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

//=============================================================================
//	Abstract:
//	History:
//			Created	From object.ccs using Prep
//
//	Class Hierarchy:
//			none
//
//	Dependancies:
//
//	Restrictions:
//
//	Example:
//
//============================================================================

#include <oas/generator.ht>		// get oad structure information
#include "generator.hp"
#include "actor.hp"
#include "gamestrm.hp"
#include "level.hp"

//============================================================================

Generato::Generato( const SObjectStartupData* startupData ) : Actor( startupData )
{
	_generateMailBox = getOad()->ActivationMailBox;
	_delayBetweenGeneration = getOad()->GetGenerationRate();
	_delayBetweenGeneration = _delayBetweenGeneration.Invert();

	_timeToGenerate = startupData->currentTime.Current();				// generate one object immediatly

	_vect = Vector3( getOad()->GetObjectXVelocity(), getOad()->GetObjectYVelocity(), getOad()->GetObjectZVelocity() );
}

//============================================================================

/*virtual*/ Actor::EActorKind
Generato::kind() const
{
	return Actor::Generator_KIND;
}

//============================================================================

void
Generato::update()
{
	if( theLevel->LevelClock().Current() >= _timeToGenerate )
	{
		if( GetMailboxes().ReadMailbox(_generateMailBox) == Scalar::zero )
		{
			_timeToGenerate = theLevel->LevelClock().Current();				// reset timer so when goes on doesn't generate tons of objects
			return;
		}

		//[UNUSED]const _Movement* _movementData = GetMovementBlockPtr();
		int32 objectToGenerate = getOad()->ObjectToThrow;
		assert(objectToGenerate > 0);

		DBSTREAM1( cactor << "generating object " << objectToGenerate << std::endl; )
		_timeToGenerate += _delayBetweenGeneration;
		// kts added 4/9/96 7:04PM to prevent over-generation if it has been a while since we ran
		while(_timeToGenerate < theLevel->LevelClock().Current())
		{
			DBSTREAM1( cactor << "Generato::update: fast forward" << std::endl; )
			_timeToGenerate += _delayBetweenGeneration;
		}

		Scalar xRandomDisplacement = getOad()->GetRandomXRange();
		if ( xRandomDisplacement.AsBool() )
			//xRandomDisplacement = random( xRandomDisplacement ) - xRandomDisplacement/Scalar::two;
			xRandomDisplacement = Scalar::Random( -xRandomDisplacement, xRandomDisplacement );
		Scalar yRandomDisplacement = getOad()->GetRandomYRange();
		if ( yRandomDisplacement.AsBool() )
			//yRandomDisplacement = random( yRandomDisplacement );
			yRandomDisplacement = Scalar::Random( -yRandomDisplacement, yRandomDisplacement );
		Scalar zRandomDisplacement = getOad()->GetRandomZRange() - yRandomDisplacement/Scalar::two;
		if ( zRandomDisplacement.AsBool() )
			//zRandomDisplacement = random( zRandomDisplacement ) - zRandomDisplacement/Scalar::two;
			zRandomDisplacement = Scalar::Random( -zRandomDisplacement, zRandomDisplacement );

		Vector3 displacement( xRandomDisplacement, yRandomDisplacement, zRandomDisplacement );
		Vector3 pos = _physicalAttributes.GetColSpace().GetCenter( currentPos() ) + displacement;

		// generate an object
		Actor* createdObject = theLevel->ConstructTemplateObject(objectToGenerate, _idxActor, pos,_vect);

//		const SObjectStartupData* startupData = theLevel->FindTemplateObjectData(objectToGenerate);
//		assert( ValidPtr( startupData ) );
//		Actor* createdObject = ConstructTemplateObject( startupData->objectData->type, startupData );
//		assert( ValidPtr( createdObject ));
		if(createdObject)			    // in case we are out of memory, or it would be inside of something else
			theLevel->AddObject( createdObject, pos );
	}
	Actor::update();
}

//============================================================================
//============================================================================

Actor*
OadGenerator( const SObjectStartupData* startupData )
{
	return new (*startupData->memory) Generato(startupData);
}

//============================================================================


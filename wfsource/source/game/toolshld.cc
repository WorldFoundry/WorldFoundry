//============================================================================
// Shield.cc:
// Copyright ( c ) 1996,1997,1999,2000,2003 World Foundry Group  
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
// Shield options --
//		max damage absorbed
//		absorb all damage

#include <oas/tool.ht>		// get oad structure information
#include "tool.hp"
#include "actor.hp"
#include "toolshld.hp"
#include "shield.hp"

//============================================================================

ToolShield::ToolShield(const SObjectStartupData* startupData) : Tool(startupData)
{
}

//============================================================================

void
ToolShield::activate()
{
	if ( getOad()->Type == TOOL_TYPE_SHIELD )
	{
		assert( _owner > 0 );
		Actor* pActor = theLevel->getActor( _owner );
		assert( ValidPtr( pActor ) );

// Shield now free because gold is gone. There should probably be a way to activate this from a script, then the script could deal with charging for it
#if 0
		// Charge user for shield
		assert( theLevel->getActor(_owner)->numGold() >= getOad()->ActivationCost );
		theLevel->getActor(_owner)->removeGold( getOad()->ActivationCost );
#endif

		// I do this so that I can read _shield, pActor is probably NOT a ToolShield ptr, but watcom won't let me touch a base class member through a pointer
	#pragma message ("KTS: this sucks, is this a bug in watcom, or C++?")
		if ( ((ToolShield*)pActor)->_nonStatPlat->_shield )
		{
			Shield* shield = ((ToolShield*)pActor)->_nonStatPlat->_shield;
 			++(*shield);
		}
		else
		{
			Actor* createdObject = theLevel->ConstructTemplateObject( getOad()->ObjectToThrow, _owner,  currentPos());
//			SObjectStartupData* shieldStartupData = (SObjectStartupData*)theLevel->FindTemplateObjectData( getOad()->ObjectToThrow );
//			assert( ValidPtr( shieldStartupData ) );
//			shieldStartupData->idxCreator = _owner;
//			Actor* createdObject = ConstructTemplateObject( shieldStartupData->objectData->type, shieldStartupData );
			assert( ValidPtr( createdObject) );
			theLevel->AddObject( createdObject, currentPos() );
		}
	}
	// Run this tool's activation script (if it has one)
	if (_pScript)
      theLevel->EvalScript(_pScript,GetActorIndex());
}

//============================================================================

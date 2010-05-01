//=============================================================================
// Actor.cc:
// Copyright ( c ) 1994,1995,1996,1997,1999,2000,2001,2002,2003 World Foundry Group  
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
// Description: Actor is the base classs for game world objects.
// Original Author: Ann Mei Chang
// Completely re-written by: Various
// ===========================================================================
                                                         
#include <pigsys/pigsys.hp>
#include <particle/emitter.hp>
#include <movement/movement.hp>
#include <mailbox/mailbox.hp>
#include <room/rooms.hp>
#include <oas/movement.h>
#include <cpplib/libstrm.hp>
#include <memory/memory.hp>
#include <anim/animmang.hp>
#include <movement/movefoll.hp>
#include <movement/movepath.hp>

#include <oas/oad.h>
#include <oas/mesh.ht>
#include <oas/enemy.ht>
#include <oas/spike.ht>
#include <oas/activate.ht>
#include <oas/matte.ht>
                          
#include "level.hp"
#include "tool.hp"
#include "gamestrm.hp"
#include "shield.hp"
#include "missile.hp"				// kludge for owner field
#include "shadow.hp"
#include "camera.hp"
#include "movecam.hp"
#include "game.hp"
#include "actor.hp"
#include "matte.hp"
#include "callbacks.hp"

//==============================================================================
                   
// kts object collision list, matches kind enumeration
// this is the general collision list, special collisions like activation boxes don't happen here
// get collision table from oas dir

#include <oas/objects.col>
                   
//==============================================================================
                                                                   
class Tool;

const Scalar Actor::INDESTRUCTIBLE_HP = Scalar( SCALAR_CONSTANT(32767) );	// grrr, Scalar::max isn't constructed yet

#include <streams/binstrm.hp>
#include <iff/iffread.hp>

#if defined( __WIN__ )				// only here so play cd kludge will work, remove
#include <windows.h>
#endif

#if defined (__PSX__)
#	include	<libcd.h>
#	if defined( SOUND )
#		include <libspu.h>
#		include <libsnd.h>
#	endif
#endif

#if defined( SOUND )
#	include <climits>
#endif

// ===========================================================================

static InputNull theNullInput;
static QInputDigital theNullInputDigital( &theNullInput, int(0) );

extern bool gCDEnabled;

extern PathHandler			thePathHandler;
extern DelayCameraHandler	theDelayCameraHandler;
extern FollowHandler		theFollowHandler;


//==============================================================================

MovementHandlerEntry MovementHandlerArray[] = 
{
   { MOBILITY_ANCHORED,&theNullHandler},
   { MOBILITY_PHYSICS, &theAirHandler},
   { MOBILITY_PATH,    &thePathHandler},
   { MOBILITY_CAMERA,  &theDelayCameraHandler},
   { MOBILITY_FOLLOW,  &theFollowHandler},
   { -1,NULL}
};

//============================================================================

void
Actor::Validate() const
{
   // kts write tons more validation here
   assert(ValidPtr(_nonStatPlat));
}

//============================================================================

#if SW_DBSTREAM
#include <pigsys/genfh.hp>
#include "game.hp"

std::ostream&
Actor::Print( std::ostream& s ) const
{
	char szActor[ _MAX_PATH ];
	strcpy( szActor, "unknown" );

#pragma message( __FILE__ ": recode to use genfh.hp macros" )
#if !defined(__PSX__)

	static char* actorNames[2000];
	static int actorCount = 0;
	if (!actorNames[0])
	{
		int levelNum = theGame->GetLevelNum() + 1;
		char szLevelsTxt[ _MAX_PATH ];

		extern char szOadDir[];
		sprintf( szLevelsTxt, "%s\\..\\levels.txt", szOadDir );
		FILE* levelList = fopen(szLevelsTxt, "rt");
		if(levelList)
		{
			char szLevelDir[ 256 ];
			// read level name
			while(levelNum--)
				fgets(szLevelDir,sizeof(szLevelDir),levelList);
			assert(strlen(szLevelDir));
			szLevelDir[strlen(szLevelDir)-1] = 0;

			char szObjectsId[ _MAX_PATH ];
			//assert( getenv( "OAD_DIR" ) );
			assert( *szOadDir );
			sprintf( szObjectsId, "%s\\..\\%s\\objects.id", szOadDir, szLevelDir );
			FILE* fp = fopen( szObjectsId, "rt" );
			if ( fp )
			{
				AssertMsg( ValidPtr( fp ), "Unable to open file " << szObjectsId );

				while ( !feof( fp ) )
				{
					char szLine[ 256 ];
					char _szActor[ 64 ];
					int idxActor=0;

					fgets( szLine, sizeof( szLine ), fp );
					if (sscanf( szLine, "( define %s %d )\n", _szActor, &idxActor ) == 2)
					{
						AssertMsg(actorCount == idxActor,"ActorCount = " << actorCount << ", idxActor = " << idxActor);
						actorNames[idxActor] = strdup(_szActor);
						assert(actorNames[idxActor]);
						actorCount++;
					}
				}
				fclose( fp );
			}
		}
	}
	if ( actorNames[ 0 ] && ( GetActorIndex() <= actorCount && actorNames[ GetActorIndex() ] ) )
		strcpy( szActor, actorNames[ GetActorIndex() ] );

#endif

	s << "Actor #" << GetActorIndex() << " (" << szActor << ')';
    s << " Mailboxes = [" <<_mailboxes << ']';
	return s;
}


std::ostream&
Actor::printDetailed( std::ostream& s ) const
{
	s << *this;
//	s << " currentPos=" << currentPos();
//	s << " physicalAttributes=" << GetPhysicalAttributes();

	return s;
}

#endif // SW_DBSTREAM

//==============================================================================

#if 0
BaseObject*
Actor::Activated( BaseObjectIteratorWrapper objectIter, const _Activation* actbox, int32* objList ) const
{
   assert(theLevel);

	switch ( actbox->ActivatedBy )
	{
		case 0:
		{	// All Actors
			while ( !objectIter.Empty() )
			{
            PhysicalObject* po;
				po = dynamic_cast<PhysicalObject*>(&(*objectIter));
				//DBSTREAM1( cactor << std::endl << "checking against actor #" << theLevel->GetActorIndex( po ); )
				assert( ValidPtr( po ) );
				const PhysicalAttributes& pa = po->GetPhysicalAttributes();

				if ( pa.CheckCollision( GetPhysicalAttributes() ) )
            {
					return po;
            }

				++objectIter;
			}
			break;
		}

		case 1:
		{	// by Actor
			AssertMsg( actbox->ActivatedByActor != -1, *this << " activated non-existant actor" << std::endl );
         BaseObject* colObject = theLevel->GetObject( actbox->ActivatedByActor );
			if ( colObject )
			{
				//AssertMsg( colObject, *this << " activated by actor " << *colObject << " activated non-existant actor" << std::endl );
            PhysicalObject* po = dynamic_cast<PhysicalObject*>(colObject);
				const PhysicalAttributes& pa = po->GetPhysicalAttributes();
				if ( pa.CheckCollision( GetPhysicalAttributes() ) )
            {
					return colObject;
            }
			}
			break;
		}

		case 2:
		{	// by Class
			DBSTREAM1( cactor << "(ActBox::class) Checking actors with class = " << actbox->ActivatedByClass << std::endl; )

			while ( !objectIter.Empty() )
			{
				PhysicalObject* po = dynamic_cast<PhysicalObject*>(&(*objectIter));
				assert( ValidPtr( po ) );
				if ( actbox->ActivatedByClass == po->kind() )
				{
					//DBSTREAM3( cactor << std::endl << "checking against actor #" << theLevel->GetActorIndex( po ); )
					const PhysicalAttributes& pa = po->GetPhysicalAttributes();

					if ( pa.CheckCollision( GetPhysicalAttributes() ) )
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
			(void)objList;
			DBSTREAM1( cerror << "Activation by object list unimplemented" << std::endl; )
			DBSTREAM1( cerror << " invoked by object #" << theLevel->GetObjectIndex( this ) << std::endl; )
			assert( 0 );	// Untested
			DBSTREAM3( cactor << "(ActBox::ObjectList) actors" << std::endl; )
			break;
        }

		default:
			assert( 0 );
		}

	return NULL;
}
#endif
//=============================================================================

void
Actor::_InitScript( const SObjectStartupData* startupData )
{
	DBSTREAM1( cdebug << "Actor::_InitScript" << std::endl; )
	_nonStatPlat->_pScript = NULL;
	if ( GetCommonBlockPtr()->Script != -1 )
	{
		DBSTREAM1( cdebug << "GetCommonBlockPtr()->Script = " << GetCommonBlockPtr()->Script << std::endl; )
      _nonStatPlat->_pScript = startupData->commonBlock->GetBlockPtr(GetCommonBlockPtr()->Script);
		AssertMsg( ValidPtr( _nonStatPlat->_pScript ), "actor = " << *this );
	}
}

//============================================================================

void
Actor::_InitInput( const SObjectStartupData* startupData)
{
	if ( GetCommonBlockPtr()->ScriptControlsInput )
	{
		_nonStatPlat->_inputScript = new (*startupData->memory) InputScript;
		assert( ValidPtr( _nonStatPlat->_inputScript ) );
		_nonStatPlat->_input = new (*startupData->memory) QInputDigital( _nonStatPlat->_inputScript, *startupData->memory );
		assert( ValidPtr( _nonStatPlat->_input ));
	}
	else
	{
		_nonStatPlat->_inputScript = NULL;
		assert( ValidPtr( &theNullInputDigital ) );
		_nonStatPlat->_input = &theNullInputDigital;
		assert( ValidPtr( _nonStatPlat->_input ));
	}
	assert( ValidPtr( _nonStatPlat->_input ) );
}

//============================================================================

void
Actor::_InitShadow()
{
	int32 idxShadow = GetShadowPageBlockPtr()->ShadowObjectTemplate;
	if ( idxShadow > 0 )
	{	// Create shadow
		Shadow* pShadow = (Shadow*)theLevel->ConstructTemplateObject( idxShadow, _idxActor );
		assert( ValidPtr( pShadow ) );
		assert( pShadow->kind() == Shadow_KIND );
		pShadow->SetShadowedObject( _idxActor );
		theLevel->AddObject( pShadow, currentPos() );
	}
}

//============================================================================

void
Actor::_InitTools()
{
	const _Toolset* pToolset = GetToolsetBlockPtr();
	assert( ValidPtr( pToolset ) );
	int* pidxTools = (int*)( &pToolset->ToolA );
	assert( ValidPtr( pidxTools ) );

	if ( _nonStatPlat->_nTools )
	{
		for ( int idxTool=0; idxTool<_MAX_TOOLS; ++idxTool )
		{
			Tool* pTool = NULL;
			if ( pidxTools[ idxTool ] != -1 )
			{
				pTool = (Tool*)theLevel->ConstructTemplateObject( pidxTools[ idxTool ], GetActorIndex() );
				assert( ValidPtr( pTool ) );
	 			assert( pTool->kind() == Tool_KIND );
				pTool->PickedUpBy( GetActorIndex() );
				theLevel->AddObject( pTool, pTool->currentPos() );
			}
			_nonStatPlat->_tool[ idxTool ] = pTool;
		}
	}
}

//============================================================================

void
Actor::BindAssets(Memory& memory)
{
	assert( ValidPtr( GetMeshBlockPtr() ) );
	switch( GetMeshBlockPtr()->ModelType )
	{
		case MODEL_TYPE_NONE:
#if defined(SHOW_COLLISION_AS_BOXES)
			if(GetMovementBlockPtr()->Mass != Scalar::zero)
			{
				Vector3 min(_physicalAttributes.GetColSpace().Min());
				Vector3 max(_physicalAttributes.GetColSpace().Max());
				_renderActor = new (memory) RenderActor3DBox(memory,min,max);
				_physicalAttributes.SetRotation(Euler::zero);
			}
			else
#endif	                                // SHOW_COLLISION_AS_BOXES
			_renderActor = &NULLRenderActor;
			break;

		case MODEL_TYPE_BOX:
		{
			Vector3 min(_physicalAttributes.GetColSpace().Min());
			Vector3 max(_physicalAttributes.GetColSpace().Max());
			_renderActor = new (memory) RenderActor3DBox(memory,min,max);
			_physicalAttributes.SetRotation(Euler::zero);
			break;
		}

		case MODEL_TYPE_MESH:
		{
#if defined(SHOW_GEOMETRY_AS_BOXES)  || defined(SHOW_COLLISION_AS_BOXES)				// don't show geometry, show boxes instead
			if(GetMovementBlockPtr()->Mass != Scalar::zero)
			{
				Vector3 min(_physicalAttributes.GetColSpace().Min());
				Vector3 max(_physicalAttributes.GetColSpace().Max());
				_renderActor = new (memory) RenderActor3DBox(memory,min,max);
				_physicalAttributes.SetRotation(Euler::zero);
			}
			else
				_renderActor = &NULLRenderActor;
#else
			// load model from disk
            AssertMsg(GetMeshName(), *this);
			packedAssetID assetID( GetMeshName() );
			DBSTREAM3( casset << "lookup up asset stream for asset id " << assetID << std::endl; )
			binistream binis = theLevel->GetAssetManager().GetAssetStream(assetID);
			assert(binis.good());
#pragma message ("KTS: this sux, how should I notice redundant models, and how do I not have to new everything?")
			_renderActor = new (memory) RenderActor3DAnimates(memory,binis,assetID.ID(), GameGfxCallbacks(theLevel->GetAssetManager()));
			assert(ValidPtr(_renderActor));
#endif				                    // SHOW_GEOMETRY_AS_BOXES
			break;
		}

		case MODEL_TYPE_SCARECROW:
		{
#if 1
			packedAssetID meshName( GetMeshName() );
			DBSTREAM3( casset << "lookup up asset stream for asset id " << meshName << std::endl; )
			binistream binis = theLevel->GetAssetManager().GetAssetStream( meshName);
			assert( binis.good() );
			packedAssetID assetID = GetMeshName();
			Vector3 min(_physicalAttributes.GetColSpace().Min());
			Vector3 max(_physicalAttributes.GetColSpace().Max());
			_renderActor = new (memory) RenderActorScarecrow( memory, binis, assetID.ID(), GameGfxCallbacks(theLevel->GetAssetManager()), min, max );
#else
			_renderActor = new (memory) RenderActorScarecrow( memory );
#endif
			assert( ValidPtr( _renderActor ) );
			break;
		}

		case MODEL_TYPE_LIGHT:
			_renderActor = &NULLRenderActor;
			break;

      case MODEL_TYPE_MATTE:
         {
            packedAssetID tiles( GetMeshBlockPtr()->Tiles);
            assert(tiles.ValidAssetID());
            GameGfxCallbacks ggcb(theLevel->GetAssetManager());
            _renderActor = new (memory) RenderActorMatte( memory, *GetMeshBlockPtr(), tiles.ID(), theLevel->GetAssetManager(), ggcb);
            break;
         }

		case MODEL_TYPE_EMITTER:
		{
			Emitter::EmitterParameters ep;
			ep.nParticles = GetMeshBlockPtr()->nParticles;
			ep.generationType = GetMeshBlockPtr()->generationType;
			ep.period = GetMeshBlockPtr()->Getperiod();
			ep.startDelay = GetMeshBlockPtr()->GettimeDelay();
			ep.yArc = GetMeshBlockPtr()->GetyArc();
			ep.zArc = GetMeshBlockPtr()->GetzArc();
			ep.initialSphereRadius = GetMeshBlockPtr()->GetinitialSphereRadius();
			ep.sphereExpansionRate = GetMeshBlockPtr()->GetsphereExpansionRate();
			ep.position = _physicalAttributes.Position();
			ep.rotation = _physicalAttributes.Rotation();

			Emitter::ParticleParameters pp;
			pp.initialVelocity = GetMeshBlockPtr()->GetinitialVelocity();
			pp.initialRotation = Euler( Angle::Revolution( GetMeshBlockPtr()->GetinitialAngleA() ), Angle::Revolution( GetMeshBlockPtr()->GetinitialAngleB() ), Angle::Revolution( GetMeshBlockPtr()->GetinitialAngleC() ) );
			pp.deltaRotation = Euler( Angle::Revolution( GetMeshBlockPtr()->GetdeltaAngleA() ), Angle::Revolution( GetMeshBlockPtr()->GetdeltaAngleB() ), Angle::Revolution( GetMeshBlockPtr()->GetdeltaAngleC() ) );
			pp.lifeTime = GetMeshBlockPtr()->GetlifeTime();
			pp.initialAlpha = GetMeshBlockPtr()->GetinitialAlpha();
			pp.alphaDecrement = GetMeshBlockPtr()->GetalphaDecrement();
			//pp.templateObject = GetMeshBlockPtr()->

			packedAssetID assetID( GetMeshName() );
			DBSTREAM3( casset << "lookup up asset stream for asset id " << assetID << std::endl; )
			binistream binis = theLevel->GetAssetManager().GetAssetStream( assetID);
			assert( binis.good() );


         GameGfxCallbacks ggcb(theLevel->GetAssetManager());
			pp.templateObject = new ( memory ) RenderObject3D( memory, binis , assetID.ID(), ggcb );
			assert( ValidPtr( pp.templateObject ) );

			//_renderActor = &NULLRenderActor;
			_renderActor = new ( memory ) RenderActorEmitter( memory, *GetMeshBlockPtr(), ep, pp, theLevel->LevelClock() );
			break;
		}

		default:
			assert(0);
	}

	assert(ValidPtr(_renderActor));
	_renderActor->Validate();
	DBSTREAM3(casset << "BindAssets: actor:" << *this << ",now has a renderActor =" << _renderActor << std::endl; )

}

//============================================================================

void
Actor::UnBindAssets()
{
	DBSTREAM3(casset << " UnBindAssets: actor:" << *this << ",has a renderActor =" << _renderActor << std::endl; )
	if(_renderActor && _renderActor != &NULLRenderActor)
		_renderActor->~RenderActor();
	_renderActor = NULL;
	DBSTREAM3(casset << " UnBindAssets: done" << std::endl; )
}

//==============================================================================
// a few helper functions to make PhysicalAttributes construction from disk data easier

Vector3
OODPosition(const SObjectStartupData* startupData)
{
	const _ObjectOnDisk* objdata = startupData->objectData;
   return Vector3( Scalar::FromFixed32( objdata->x ), Scalar::FromFixed32( objdata->y ), Scalar::FromFixed32( objdata->z ) );
}

Euler
OODRotation(const SObjectStartupData* startupData)
{
	const _ObjectOnDisk* objdata = startupData->objectData;
   return Euler(
      Angle::Revolution(Scalar(0,objdata->rotation.a)),
      Angle::Revolution(Scalar(0,objdata->rotation.b)),
      Angle::Revolution(Scalar(0,objdata->rotation.c))
   );
}

Vector3
OODMin(const SObjectStartupData* startupData)
{
   const struct _CollisionRectOnDisk* cod =  &startupData->objectData->coarse;
   return Vector3( 
      Scalar::FromFixed32( cod->minX ), 
      Scalar::FromFixed32( cod->minY ), 
      Scalar::FromFixed32( cod->minZ ) 
   );
}

Vector3
OODMax(const SObjectStartupData* startupData)
{
   const struct _CollisionRectOnDisk* cod =  &startupData->objectData->coarse;
   return Vector3( 
      Scalar::FromFixed32( cod->maxX ), 
      Scalar::FromFixed32( cod->maxY ), 
      Scalar::FromFixed32( cod->maxZ ) 
   );
}

//==============================================================================

const Actor::NonStatPlatData Actor::_statPlatData(NULL,NULL);

Actor::Actor( const SObjectStartupData* startupData ) : 
   MovementObject( startupData, (void*)( startupData->objectData + 1 ),theLevel->GetCommonBlock(),theLevel->LevelClock()),
	_physicalAttributes( 
                           OODPosition(startupData),
                           OODRotation(startupData),
                           OODMin(startupData),
                           OODMax(startupData),
									GetCommonBlockPtr()->GetslopeA(),
									GetCommonBlockPtr()->GetslopeB(),
									GetCommonBlockPtr()->GetslopeC(),
									GetCommonBlockPtr()->GetslopeD()),
    _mailboxes(*this, EMAILBOX_LOCAL_START,GetCommonBlockPtr()->NumberOfLocalMailboxes,startupData->mailboxes)
{
	DBSTREAM3( cactor << "Actor::Actor:" << std::endl; )
	assert( ValidPtr( startupData ) );
    assert(ValidPtr( startupData->mailboxes));
	assert( ValidPtr( _oadData ) );

	_idxActor = startupData->idxActor;

	const _ObjectOnDisk* objdata = startupData->objectData;
	assert( ValidPtr( objdata ) );

	_physicalAttributes.SetPredictedPosition( _physicalAttributes.Position() );
	_physicalAttributes.Validate();

	_lastVisibility = false;
	_visibility = false;

	_renderActor = NULL;

	DBSTREAM3( cactor << "Created new Actor #" << _idxActor );
	DBSTREAM3( cactor << ", Physical Attributes: " << std::endl );
	DBSTREAM3( cactor << _physicalAttributes << std::endl; )

	// non-statplat data
	if ( objdata->type == Actor::StatPlat_KIND )
	{
		_nonStatPlat = const_cast<NonStatPlatData*>( &_statPlatData );

        assert( ValidPtr( GetCommonBlockPtr() ) );
		AssertMsg( GetCommonBlockPtr()->Script == -1, *this << " -- No scripts allowed on StatPlat's" );
		AssertMsg( !GetCommonBlockPtr()->ScriptControlsInput, *this <<" -- No scripts [and therefore no Script Controls Input] on StatPlat's" );
		AssertMsg( GetCommonBlockPtr()->NumberOfLocalMailboxes == 0, *this << " -- No local mailboxes allowed on StatPlat's" );
		AssertMsg( GetMovementBlockPtr()->Mobility == MOBILITY_ANCHORED, *this << " -- StatPlat's must be anchored -- Mobility is " << GetMovementBlockPtr()->Mobility );
	}
	else
	{
      assert(ValidPtr(GetMovementBlockPtr()));
		_nonStatPlat = new (theLevel->GetMemory()) NonStatPlatData(startupData->messagePortMemPool,GetMovementBlockPtr());
		assert( ValidPtr( _nonStatPlat ) );

      _nonStatPlat->_animManager = NULL;

		_nonStatPlat->_hitPoints = Scalar( GetCommonBlockPtr()->Gethp() );
		_nonStatPlat->_shield = NULL;

		_InitInput( startupData );
		_InitScript( startupData );

#pragma message( __FILE__ ": probably should have movepath handler initializer do this" )
		if ( GetMovementBlockPtr()->Mobility == MOBILITY_PATH )
		{
			DBSTREAM3( cactor << "  Has path data" << std::endl; )
			assert( ValidPtr( _path ) );
			setCurrentPos( _path->Position(theLevel->LevelClock().Current()) );
		}

		_nonStatPlat->_animManager = &AnimationManagerNull::theAnimationManagerNull;

      if( GetMovementBlockPtr()->Mobility == MOBILITY_PHYSICS)
      {
         // kts this is lame, there should be no connection between movement and animation (and it should be possible for a follow or pathed object to animate)
         _nonStatPlat->_animManager = new (theLevel->GetMemory()) AnimationManagerActual;
         assert(ValidPtr(_nonStatPlat->_animManager));
      }

		// setup the tools
		const _Toolset* pToolset = GetToolsetBlockPtr();
		assert( ValidPtr( pToolset ) );
		int* pidxTools = (int*)( &pToolset->ToolA );
		assert( ValidPtr( pidxTools ) );

		_nonStatPlat->_nTools = 0;
		for ( int idxTool=0; idxTool<_MAX_TOOLS; ++idxTool )
		{
			if ( pidxTools[ idxTool ] != -1 )
				++_nonStatPlat->_nTools;
		}
	}

	AssertMsg( _nonStatPlat->_hitPoints > Scalar::zero, *this << " has an invalid hit point setting of " << _nonStatPlat->_hitPoints );

   assert( ValidPtr( GetMeshBlockPtr() ) );
   if(
      GetMeshName()
      &&
      (
         GetMeshBlockPtr()->ModelType == MODEL_TYPE_SCARECROW
         || GetMeshBlockPtr()->ModelType == MODEL_TYPE_MESH
      )
   )
   AssertMsg( ( GetMeshBlockPtr()->ModelType == MODEL_TYPE_BOX )
//		|| ( GetMeshBlockPtr()->ModelType == MODEL_TYPE_NONE )
   || ( GetMeshBlockPtr()->ModelType == MODEL_TYPE_SCARECROW )
   || ( GetMeshBlockPtr()->ModelType == MODEL_TYPE_MESH ),
   *this );
	DBSTREAM3( cactor << "Actor::Actor: Done" << std::endl; )
}

//==============================================================================

Actor::~Actor()
{
	DBSTREAM3( cactor << "Actor destructor" << std::endl; )

	DBSTREAM3( cactor << "Actor destructor: unloading assets" << std::endl; )
	UnBindAssets();
	DBSTREAM3( cactor << "Actor destructor: unlinking tools" << std::endl; )

	assert( ValidPtr( _nonStatPlat->_animManager ) );
#pragma message( __FILE__ ": should override delete for this class and move this if in it -- clients can then just use delete _nonStatPlat->_animManager" )
	// delete this Actor's instance of AnimManager, unless it's using the global NullAnimManager
	if ( _nonStatPlat->_animManager != &AnimationManagerNull::theAnimationManagerNull )
	{
		DBSTREAM3( cactor << "Actor " << _idxActor << " destructor: deleting animManager" << std::endl; )
		assert( ValidPtr(_nonStatPlat->_animManager) );
		MEMORY_DELETE( theLevel->GetMemory(), _nonStatPlat->_animManager, AnimationManager);
	}

	// if there is a model attached to the root, make sure to get rid of it
	// (typically this is a VisualBox)

	DBSTREAM3( cactor << "Actor destructor: delete coarse space" << std::endl; )

	DBSTREAM3( cactor << "Actor destructor: Cleaning out MsgPort queue" << std::endl; )
	int16 msgType;
   char msgData[msgDataSize];

	while ( GetMsgPort().GetMsg( msgType, &msgData, msgDataSize ) )
		;

	DBSTREAM3( cactor << "Actor destructor done" << std::endl; )
}

//==============================================================================

void
Actor::predictPosition(const Clock& currentTime)
{
    AssertMsg( _nonStatPlat->_input, *this );
   _nonStatPlat->_movementManager.predictPosition(*this,currentTime,theLevel->GetObjectList());
}

//==============================================================================

bool
Actor::isVisible() const
{
	assert(ValidPtr(GetMeshBlockPtr()));
	return GetMailboxes().ReadMailbox( GetMeshBlockPtr()->VisibilityMailbox ).AsBool();
}

//==============================================================================

void
Actor::update()
{
	DBSTREAM1( cactor << "Actor::update" << std::endl; )

   assert(GetPhysicalAttributes().HasRunPredictPosition());

#if 0
#if SHADOW
	if ( !_hasGeneratedShadow )
	{	// Initialize shadow if requested
		int32 idShadow = GetShadowPageBlockPtr()->ShadowObjectTemplate;
		if ( idShadow > 0  )
		{	// Create shadow
			Actor* pCreatedActor = theLevel->ConstructTemplateObject(idShadow,_idxActor);
//			const SObjectStartupData* startupData = theLevel->FindTemplateObjectData( idShadow );
//			assert( ValidPtr( startupData ) );
//			Actor* pCreatedActor = ConstructTemplateObject( startupData->objectData->type, startupData );
			assert( pCreatedActor->kind() == Shadow_KIND );
			Shadow* _pShadow = (Shadow*)pCreatedActor;
			assert( ValidPtr( _pShadow ) );
			_pShadow->SetShadowedObject( _idxActor );
			theLevel->AddObject( _pShadow );
			_hasGeneratedShadow = true;
		}
	}
#endif
#endif
    //theLevel->GetMailboxes().WriteMailbox( EMAILBOX_NUM_COLLISIONS, Scalar::zero );

    char msgData[msgDataSize];
	while( GetMsgPort().GetMsgByType( MsgPort::DELTA_HEALTH, &msgData,  msgDataSize) )
	{
		AssertMsg( *(int32*)&msgData, "Unexpected delta health of zero in " << *this << std::endl );
		if ( _nonStatPlat->_hitPoints != INDESTRUCTIBLE_HP )
			deltaHealth( Scalar( *(Scalar*)&msgData) );
	}

#pragma message( __FILE__ ": should we do mailboxes and/or messages for sound effects?" )
//	while( GetMsgPort().GetMsgByType( MsgPort::PLAY_SOUND_EFFECT, msgData ) )
//  	{
//		DBSTREAM1( cdebug << "Playing sound # " << msgData << std::endl; )
//	}

//	cerror << "actor::update: ran actor #" << theLevel->GetActorIndex(this) << std::endl;
	_nonStatPlat->_movementManager.update( *this, theLevel->GetObjectList() );

	// kts added 1/3/97 3:06PM
	AssertMsg( ValidPtr(_nonStatPlat->_animManager), "Actor #" << _idxActor << ": _animManager = " << _nonStatPlat->_animManager );
   assert(theLevel);

   BaseObjectIteratorWrapper objectIter = theLevel->GetActiveRooms().GetObjectIter(ROOM_OBJECT_LIST_COLLIDE);
   
   assert(ValidPtr(_nonStatPlat));
	_nonStatPlat->_animManager->UpdateAnimation(_renderActor, *GetInputDevice(),*this,objectIter,_nonStatPlat->_movementManager.GetMovementHandlerData(),theLevel->LevelClock());

	// CamShot doesn't have anim (is this OK?  are there others?)
	assert( ValidPtr( GetMeshBlockPtr() ) );

	if ( _nonStatPlat->_nTools )
	{
		for ( int idxTool=0; idxTool < _MAX_TOOLS; ++idxTool )
		{
         assert( idxTool < _MAX_TOOLS );
			Tool* pTool = _nonStatPlat->_tool[ idxTool ];
			if ( pTool )
			{
				//cactor << "tool " << idxTool << " = " << *pTool << std::endl;
				pTool->trigger( *_nonStatPlat->_input );
			}
		}
	}

	if ( _nonStatPlat->_pScript )
	{
		DBSTREAM3( cactor << " executing script:" << _nonStatPlat->_pScript << std::endl; )
      theLevel->EvalScript(_nonStatPlat->_pScript,GetActorIndex());
	}

//#if DEBUG
//	if ( !GetMsgPort().Empty() )
//		cerror << "Message hog is " << pLevel->GetActorIndex( this ) << std::endl;
//#endif
//	assert(GetMsgPort().Empty());
}

//==============================================================================

void
Actor::deltaHealth( Scalar nHealth )
{
	assert( nHealth != Scalar::zero );
	assert( _nonStatPlat->_hitPoints != INDESTRUCTIBLE_HP );
	if ( nHealth < Scalar::zero )
	{
		if ( _nonStatPlat->_shield )
		{
			if ( _nonStatPlat->_shield->isInvulnerable() )
				return;
			else if ( (*_nonStatPlat->_shield)() > 0 )
			{
				--(*_nonStatPlat->_shield);
				return;
			}
		}
	}

	_nonStatPlat->_hitPoints += nHealth;
	if ( _nonStatPlat->_hitPoints > Scalar::zero )
	{	// TO DO: Switch to the "taking damage" movement handler/animation
//		assert( ValidPtr( anim ) );
      assert(ValidPtr(_renderActor));
		_nonStatPlat->_animManager->SetAnimCycle(*_renderActor, AnimationManager::HIT_CHEST_SOFT,theLevel->LevelClock() );
		GetMovementManager().SetStunTime(SCALAR_CONSTANT(2.0),theLevel->LevelClock());				// be stunned for 2 seconds
//			anim->setAnimationCycle(Animation::PUSHED, theLevel->LevelClock().Current());
//			groundData->stunnedUntil = theLevel->getWallClock + GetMovementBlockPtr()->StunDuration;
	}
	else
	{
		assert( nHealth <= Scalar::zero );
		_nonStatPlat->_hitPoints = Scalar::zero;
		die();
	}
}

//==============================================================================

void
Actor::die()
{
	theLevel->GetMailboxes().WriteMailbox( GetCommonBlockPtr()->WriteToMailboxOnDeath, Scalar::one );

	// TO DO: Set to the "die" movement handler/animation
//	assert( ValidPtr( anim ) );
//	anim->SetAnimationCycle( _Animation::EXPLODE_FRONT );
   assert(ValidPtr(_renderActor));
	_nonStatPlat->_animManager->SetAnimCycle( *_renderActor, AnimationManager::EXPLODE_FRONT,theLevel->LevelClock() );
	theLevel->SetPendingRemove( this );
}

//==============================================================================

void
Actor::reset()
{
	_InitShadow();
	_InitTools();
}

//============================================================================

bool
Actor::CanRender() const
{
	assert( ValidPtr( GetMeshBlockPtr() ) );

#if defined(SHOW_COLLISION_AS_BOXES)
	return( 1 );
#else
	return( GetMeshBlockPtr()->ModelType != MODEL_TYPE_NONE );
#endif
}

//==============================================================================

bool
Actor::CanUpdate() const
{
	//return !IsActivationBox();
	return true;
}

//==============================================================================

bool
Actor::CanCollide() const
{
   const _Movement* movementBloc = GetMovementBlockPtr();
   assert(ValidPtr(movementBloc));
   return( collisionTable[kind()] && movementBloc->Mass > Scalar::zero );
}

//==============================================================================

bool
Actor::IsLight() const
{
   return (kind() == BaseObject::Light_KIND);
}

//==============================================================================

bool 
Actor::IsActivationBox() const
{
   return (
      kind() == BaseObject::ActBox_KIND ||
      kind() == BaseObject::ActBoxOR_KIND
      );
}

//==============================================================================

void
Actor::StartFrame()
{
	DBSTREAM5( cflow << "Actor::StartFrame: " << *this << std::endl; )
	DBSTREAM5( cflow << "Actor::StartFrame: UpdateInput" << std::endl; )
	assert(ValidPtr(_nonStatPlat->_input));
	_nonStatPlat->_input->update();	// Moved this here so that both PredictPosition() and Update()
						// agree about isPressed vs. justPressed
	DBSTREAM5( cflow << "Actor::StartFrame: done" << std::endl; )
}

//==============================================================================
// If the handle exists, true is returned and dest is filled with the location
// of a handle relative to the root of the Actor (currently this is placed at
// the center of the collision box).  Otherwise, false is returned.

bool
Actor::LocalHandleLocation( HandleID id, Vector3& dest ) const
{
	return _renderActor->GetHandle(id, dest);
}

//==============================================================================
// If the handle exists, true is returned and dest is filled with the location
// of a handle in global coordinates (the coordinate frame of the level).
// Otherwise, false is returned.

bool
Actor::GlobalHandleLocation( HandleID id, Vector3& dest ) const
{
	bool bHandleFound;
	if ( bHandleFound = _renderActor->GetHandle(id, dest) )
		dest += currentPos();

	return bHandleFound;
}

//==============================================================================

void
Actor::spawnPoof() const
{
	if ( GetCommonBlockPtr()->Poof != - 1)
	{
		DBSTREAM3(cactor << "Spawning poof "; )

		Actor* createdObject = theLevel->ConstructTemplateObject( GetCommonBlockPtr()->Poof, _idxActor );
		assert( ValidPtr( createdObject ) );

		DBSTREAM3(cactor << "at position " << currentPos() << std::endl; )

//		theLevel->AddObject( createdObject, currentPos() );
		theLevel->AddObject( createdObject, GetPredictedPosition() );
		createdObject->GetWritablePhysicalAttributes().SetRotation(_physicalAttributes.Rotation());
	}
}

//==============================================================================

Scalar
Actor::GetDistanceTo( const Actor *actor ) const
{
	assert( ValidPtr( actor) );

	Vector3 myPos( currentPos() );
	Vector3 hisPos( actor->currentPos() );

	// find ABS deltas
	Scalar maxC( (hisPos.X() - myPos.X()).Abs() );
	Scalar medC( (hisPos.Y() - myPos.Y()).Abs() );
	Scalar minC( (hisPos.Z() - myPos.Z()).Abs() );

	// Sort to find max (med and min need not be sorted)
	if (maxC < medC)
		maxC.Swap(medC);
	if (maxC < minC)
		maxC.Swap(minC);

	// return maxC + 1/4(medC) + 1/4(minC)
	medC += minC;	// combine med and min values (to only divide once)
	return (maxC + ((medC + minC) / SCALAR_CONSTANT(4) ));
#pragma message ("KTS: use the distance function in vector3 instead!")
}

//==============================================================================

int32
Actor::getPower() const
{
	assert(ValidPtr(_nonStatPlat->_shield));
	return( (*_nonStatPlat->_shield)() );
}

//==============================================================================

#undef ERROR
#define ERROR( __msg__ )			{ DBSTREAM1( std::cerr << "ERROR: " << __msg__ << std::endl; ) sys_exit( -1); }
#undef UNIMPLEMENTED
#define UNIMPLEMENTED( __msg__ )	{ DBSTREAM1( std::cerr << "UNIMPLEMENTED: " << __msg__ << std::endl; ) sys_exit( -1 ); }

Scalar
Actor::ReadSystemMailbox( int boxnum ) const
{
  //  cerr << "read [" << boxnum << "]\t";
	assert( boxnum >= 0);
	assert( boxnum < 65536);		    // kts arbitrary

    RangeCheck(EMAILBOX_LOCAL_SYSTEM_START,boxnum,EMAILBOX_LOCAL_SYSTEM_MAX);

    assert( EMAILBOX_LOCAL_SYSTEM_START <= boxnum && boxnum < EMAILBOX_LOCAL_SYSTEM_MAX );
    switch ( boxnum )
    {
        case EMAILBOX_HITPOINTS:
            return _nonStatPlat->_hitPoints;

        case EMAILBOX_SHIELD:
            return Scalar( getPower(), 0 );

        case EMAILBOX_LOCAL_MIDI:
            UNIMPLEMENTED( "read local midi" );

        case EMAILBOX_ALIVE:
            return Scalar::one;

        case EMAILBOX_ACTOR_INDEX:
            return Scalar( GetActorIndex(), 0 );

        case EMAILBOX_JOYSTICK_ARE_PRESSED:
            return Scalar( _nonStatPlat->_input->arePressed(), 0 );

        case EMAILBOX_JOYSTICK_JUST_PRESSED:
            return Scalar( _nonStatPlat->_input->justPressed(), 0 );

        case EMAILBOX_JOYSTICK_JUST_RELEASED:
            return Scalar( _nonStatPlat->_input->justReleased(), 0 );

        case EMAILBOX_X_POS:
            return currentPos().X();

        case EMAILBOX_Y_POS:
            return currentPos().Y();

        case EMAILBOX_Z_POS:
            return currentPos().Z();

        case EMAILBOX_ROTATION_A:
            return GetPhysicalAttributes().Rotation().GetA().AsRevolution();

        case EMAILBOX_ROTATION_B:
            return GetPhysicalAttributes().Rotation().GetB().AsRevolution();

        case EMAILBOX_ROTATION_C:
            return GetPhysicalAttributes().Rotation().GetC().AsRevolution();

        case EMAILBOX_DELTA_YAW:
        {
            UNIMPLEMENTED( "DELTA_YAW mailbox is write-only!" );
            return Scalar::zero;
        }

        case EMAILBOX_DELTA_PITCH:
        {
            UNIMPLEMENTED( "DELTA_PITCH mailbox is write-only!" );
            return Scalar::zero;
        }

        case EMAILBOX_DELTA_ROLL:
        {
            UNIMPLEMENTED( "DELTA_ROLL mailbox is write-only!" );
            return Scalar::zero;
        }

        case EMAILBOX_KEYFRAME:
        {
            UNIMPLEMENTED( "read keyframe mailbox" );
            return Scalar::zero;
        }

     case EMAILBOX_ROOM:
        assert(0);                          // kts removed 11/4/2002 15:21:20 while removeing Actor's dependency on Rooms, do we even need this?
        return Scalar::zero;
            //return Scalar( GetRoom(), 0 );

        case EMAILBOX_SPEED:
            return GetPhysicalAttributes().LinVelocity().Length();

        case EMAILBOX_XSPEED:
            return GetPhysicalAttributes().LinVelocity().X();

        case EMAILBOX_YSPEED:
            return GetPhysicalAttributes().LinVelocity().Y();

        case EMAILBOX_ZSPEED:
            return GetPhysicalAttributes().LinVelocity().Z();

        case  EMAILBOX_GENERATOR:
        {
            assert( kind() == Actor::Missile_KIND);
            Missile* that = (Missile*)this;
            return Scalar( that->GetOwner(), 0 );
        }

        case EMAILBOX_KIND:
            return Scalar( kind(), 0 );

//			case EMAILBOX_ANIMATION_INDEX:
//				return _animationIndex;
//			case EMAILBOX_ANIMATION_DATA:
//			{
//				ValidatePtr( anim );
//				return long( anim->GetAnimationData( _animationIndex ) );
//				assert(EMAILBOX_ANIMATION_INDEX < 65536);			// kts temp
//				std::cout << "animation index = " << EMAILBOX_ANIMATION_INDEX << std::endl;
//				return long( anim->GetAnimationData( GetMailboxes().ReadMailbox(EMAILBOX_ANIMATION_INDEX) >> 16 ) );
//				return Scalar::zero;
//			}

        // Line-of-sight (vector vs. box collision checking)
        case EMAILBOX_LOS_RESULT:
        {
        assert(0);           // kts 11/4/2002 15:14:26 removing room references, if we want LineOfSight later uncomment this code and deal with the room references
        return Scalar::zero;
#if 0
            Vector3 endpoint2( _nonStatPlat->_localMailboxes[ EMAILBOX_LOS_X - EMAILBOX_LOCAL_USER_START ],
                               _nonStatPlat->_localMailboxes[ EMAILBOX_LOS_Y - EMAILBOX_LOCAL_USER_START ],
                               _nonStatPlat->_localMailboxes[ EMAILBOX_LOS_Z - EMAILBOX_LOCAL_USER_START ]);

            Actor* closestActor = NULL;

            //  Now check all renderable Actors in rooms 0, 1, and 2 for collision with the handle
            Room& theRoom = (Room&)theLevel->GetLevelRooms().GetRoom( GetRoom() );
            BaseObjectIteratorWrapper poIter = theRoom.GetCollisionIter();
            Actor* colActor;

            while ( !poIter.Empty() )
            {

                colActor = dynamic_cast<Actor*>(&(*poIter));
                assert( ValidPtr( colActor ) );
                if ( colActor->CanRender() )
                {
                    const PhysicalAttributes& colAttr = colActor->GetPhysicalAttributes();
                    if ( colAttr.CheckCollision( endpoint2 ) )
                    {
                        // Is this Actor closest to endpoint2?
                        if ( (colActor->currentPos() - endpoint2).Length() < (closestActor->currentPos() - endpoint2).Length() )
                            closestActor = colActor;
                    }
                }
                ++poIter;
            }

            return closestActor ? Scalar( theLevel->GetObjectIndex( closestActor ), 0 ) : Scalar::zero;
#endif
        }

        default:
                AssertMsg( 0, *this << "Actor::ReadSystemnMailbox(): Write-only local system mailbox " << boxnum << std::endl );
            return Scalar::zero;
    }

    assert(0);
    return Scalar::zero;
}

//=============================================================================

void
Actor::WriteSystemMailbox( int boxnum, Scalar value )
{
    RangeCheck(EMAILBOX_LOCAL_SYSTEM_START,boxnum,EMAILBOX_LOCAL_SYSTEM_MAX);

    static Euler rotationEuler;         // kts major kludge, what we really need is a wider interface to the scripting language

  //  cerr << "write-mailbox[" << boxnum << "]=[" << value << "]\t";
	DBSTREAM2( cmailbox << "local system mailbox " << boxnum << " set to " << value << std::endl; )

    switch ( boxnum )
    {
        case EMAILBOX_HITPOINTS:
            AssertMsg( _nonStatPlat->_hitPoints != INDESTRUCTIBLE_HP, "Attempted to modify hitpoints for " << *this << ", but is set to indestructible" );
            _nonStatPlat->_hitPoints = value;
            break;

        case EMAILBOX_SHIELD:
            break;

        case EMAILBOX_LOCAL_MIDI:
            break;

        case EMAILBOX_ALIVE:
            if ( value == Scalar::zero )
                theLevel->SetPendingRemove( this );
            break;

        case EMAILBOX_ACTOR_INDEX:
            ERROR( "Cannot set \"actor index\" mailbox" );
            break;

        case EMAILBOX_JOYSTICK_ARE_PRESSED:
            assert( ValidPtr( _nonStatPlat->_input ) );
            assert( _nonStatPlat->_input != &theNullInputDigital );
            _nonStatPlat->_input->setButtons( joystickButtonsF( value.WholePart() ) );
            break;

        case EMAILBOX_JOYSTICK_JUST_PRESSED:
            ERROR( "Cannot set \"just pressed\" mailbox" );
            break;

        case EMAILBOX_JOYSTICK_JUST_RELEASED:
            ERROR( "Cannot set \"just released\" mailbox" );
            break;

        case EMAILBOX_X_POS:
        {
//					AssertMsg(GetMovementBlockPtr()->Mass == Scalar::zero, " in actor " << *this);			// can only move objects with no mass so we don't fuck up the physics system
            Vector3 tVect = _physicalAttributes.Position();
            tVect.SetX(value);
            _physicalAttributes.SetPosition(tVect);
            _physicalAttributes.SetPredictedPosition(tVect);
            break;
        }
        case EMAILBOX_Y_POS:
        {
//					AssertMsg(GetMovementBlockPtr()->Mass == Scalar::zero, " in actor " << *this);			// can only move objects with no mass so we don't fuck up the physics system
            Vector3 tVect = _physicalAttributes.Position();
            tVect.SetY(value);
            _physicalAttributes.SetPosition(tVect);
            _physicalAttributes.SetPredictedPosition(tVect);
            break;
        }
        case EMAILBOX_Z_POS:
        {
//					AssertMsg(GetMovementBlockPtr()->Mass == Scalar::zero, " in actor " << *this);			// can only move objects with no mass so we don't fuck up the physics system
            Vector3 tVect = _physicalAttributes.Position();
            tVect.SetZ(value);
            _physicalAttributes.SetPosition(tVect);
            _physicalAttributes.SetPredictedPosition(tVect);
            break;
        }
        case EMAILBOX_ROTATION_A:
        {
            Angle rot( Angle::Revolution( value ) );
            DBSTREAM4( cstats << "actor got rotation a of " << rot << std::endl; )
       rotationEuler.SetA(rot);
            //_physicalAttributes.SetRotationA( rot );
            break;
        }

        case EMAILBOX_ROTATION_B:
        {
            Angle rot( Angle::Revolution( value ) );
            DBSTREAM4( cstats << "actor got rotation b of " << rot << std::endl; )
       rotationEuler.SetB(rot);
            //_physicalAttributes.SetRotationB( rot );
            break;
        }

        case EMAILBOX_ROTATION_C:
        {
            Angle rot( Angle::Revolution( value ) );
            DBSTREAM4( cstats << "actor got rotation c of " << rot << std::endl; )
       rotationEuler.SetC(rot);
            _physicalAttributes.SetRotation( rotationEuler );
            //_physicalAttributes.SetRotationC( rot );
            break;
        }

        case EMAILBOX_DELTA_YAW:
        {
            Matrix34 localMatrix(_physicalAttributes.Rotation(), Vector3::zero);
            Vector3 lookUp( Vector3::unitZ * localMatrix );
            _physicalAttributes.RotateAboutAxis( lookUp, Angle::Revolution(value) );
            break;
        }

        case EMAILBOX_DELTA_PITCH:
        {
            Matrix34 localMatrix(_physicalAttributes.Rotation(), Vector3::zero);
            Vector3 lookLeft( Vector3::unitY * localMatrix );
            _physicalAttributes.RotateAboutAxis( lookLeft, Angle::Revolution(value) );
            break;
        }

        case EMAILBOX_DELTA_ROLL:
        {
            Matrix34 localMatrix(_physicalAttributes.Rotation(), Vector3::zero);
            Vector3 lookAt( Vector3::unitX * localMatrix );
            _physicalAttributes.RotateAboutAxis( lookAt, Angle::Revolution(value) );
            break;
        }

        case EMAILBOX_KEYFRAME:
            UNIMPLEMENTED( "set keyframe" );
            break;

        case EMAILBOX_ROOM:
            ERROR( "Cannot set \"room\" mailbox" );
            break;

        case EMAILBOX_CD_TRACK:
        {
#if defined( __WIN__ )
#include <cdda/cd.hp>
            //extern HWND worldFoundryhWnd;
            //assert( worldFoundryhWnd );
            // kts 7/14/99 7:34AM doesn't link right now
            //playCDTrack( worldFoundryhWnd, value.WholePart() );
#endif
            break;
        }

        case EMAILBOX_SOUND:
        {
            int32 soundEffect = value.WholePart();
            DBSTREAM1( cdebug << "Playing sound # " << soundEffect << std::endl; )
            RangeCheck( soundEffect, 0, 128 );
            AssertMsg( theLevel->_sfx[ soundEffect ], "No sound effect loaded into slot #" << soundEffect );
            theLevel->_sfx[ soundEffect ]->play();
            break;
        }

    case EMAILBOX_XSPEED:
       {
          Vector3 linVelocity = _physicalAttributes.LinVelocity();
          linVelocity.SetX(value);
          _physicalAttributes.SetLinVelocity( linVelocity );
       }
            break;

    case EMAILBOX_YSPEED:
       {
          Vector3 linVelocity = _physicalAttributes.LinVelocity();
          linVelocity.SetY(value);
          _physicalAttributes.SetLinVelocity( linVelocity );

       }
            break;

    case EMAILBOX_ZSPEED:
       {
          Vector3 linVelocity = _physicalAttributes.LinVelocity();
          linVelocity.SetZ(value);
          _physicalAttributes.SetLinVelocity( linVelocity );
       }
            break;

        case EMAILBOX_INPUT :
        {
#pragma message( __FILE__ ": can _input be used instead of _inputScript? Then _inputScript wouldn't need to be stored in class Actor" )
            AssertMsg( ValidPtr( _nonStatPlat->_inputScript ),
                *this << " tried to set input with a script, but \"Script Controls Input\" wasn't set" << std::endl );
            _nonStatPlat->_inputScript->setButtons( value.WholePart() );
            break;
        }

//				case EMAILBOX_ANIMATION_INDEX:
//				{
//					_animationIndex = value >> 16;
//					break;
//				}
//				case EMAILBOX_ANIMATION_DATA:
//				{
//					ValidatePtr( anim );
//	//					anim->SetAnimationData( _animationIndex, (void *)value );
//					anim->SetAnimationData( GetMailboxes().ReadMailbox(EMAILBOX_ANIMATION_INDEX) >> 16, (void *)value );
//					break;
//				}

        case EMAILBOX_QUERY_HANDLE:
        {
//                DBSTREAM3(
//                   char id[ 5 ];
//                   *((int32*)id) = value.AsLong();
//                   id[ 4 ] = '\0';
//                   cmailbox << "Query handle: " << id << std::endl;
//                );

            //Vector3 vHandle;
       assert(0);        // kts Uh-oh, with floating point math can no longer assume this will work
            //ASSERTIONS(bool bSuccess = ) GlobalHandleLocation( HandleID(value.AsLong()), vHandle );
            //AssertMsg( bSuccess, "Actor " << *this << ": Can't find handle <" << id << ">" );
            //SetMailbox( EMAILBOX_HANDLE_X, vHandle.X() );
            //SetMailbox( EMAILBOX_HANDLE_Y, vHandle.Y() );
            //SetMailbox( EMAILBOX_HANDLE_Z, vHandle.Z() );
            break;
        }

        default:
            AssertMsg( 0, *this << ": (set) Unknown local system mailbox " << boxnum << std::endl );
            break;
    }
}

//==============================================================================
// This method can be called from the update() method of any Actor subclass
// that wants to receive special collsions.  It checks to make sure the
// collision is still valid, returning the msgData if still valid or NULL
// (in msgData) if not.  The bool return is true if there were any messages
// waiting, false otherwise.

bool
Actor::GetSpecialCollisionMessage(void * msgData, int32 maxsize)
{
	if ( GetMsgPort().GetMsgByType( MsgPort::SPECIAL_COLLISION, msgData,maxsize) )
	{
		// re-check this collision to make sure it's still valid
		const PhysicalAttributes& colAttr = ((Actor*)msgData)->GetPhysicalAttributes();
		if ( !(_physicalAttributes.CheckCollision(colAttr)) )
			*(long*)msgData = 0;
		return true;
	}
	else
		return false;
}

//==============================================================================
// PhysicalObject interface/overrides


const PhysicalAttributes& 
Actor::GetPhysicalAttributes() const
{
    return _physicalAttributes;
}

PhysicalAttributes& 
Actor::GetWritablePhysicalAttributes()
{
    return _physicalAttributes;
}

MovementManager& 
Actor::GetMovementManager()
{
    return _nonStatPlat->_movementManager;
}

//==============================================================================

void 
Actor::KillSelf()      // cause this object to get deleted from the world
{
   theLevel->SetPendingRemove(this);
}


//==============================================================================

MsgPort& 
Actor::GetMsgPort()
{
   Validate();
   assert(ValidPtr(_nonStatPlat));
   return _nonStatPlat->_msgPort;
}

void 
Actor::Collision(PhysicalObject& other, const Vector3& normal)
{
	if ( normal.Z() < Scalar::zero )
	{
		if (GetMovementManager().GetMovementHandlerData())
      {
         MovementObject* movementObject = dynamic_cast<MovementObject*>(&other);
         assert(ValidPtr(movementObject));
			GetMovementManager().GetMovementHandlerData()->supportingObject = movementObject;
      }
	}
}

//==============================================================================

const QInputDigital*
Actor::GetInputDevice() const
{
   assert(ValidPtr(_nonStatPlat));
    return _nonStatPlat->_input;
}

//==============================================================================

void 
Actor::MovementStateChanged( const MovementObject::EMovementState newState )    // so we can notify the animation system
{
   assert(ValidPtr(_nonStatPlat->_animManager));
   _nonStatPlat->_animManager->SetMovementState(newState);
}

//==============================================================================

Memory& 
Actor::GetMemory() const
{
   return theLevel->GetMemory();
}



Mailboxes& 
Actor::GetMailboxes()
{
    return _mailboxes;
}

const Mailboxes& 
Actor::GetMailboxes() const
{
    return _mailboxes;
}


ActorMailboxes::ActorMailboxes(Actor& actor,long mailboxesBase, long numberOfLocalMailboxes, Mailboxes* parent) :
MailboxesWithStorage(mailboxesBase, numberOfLocalMailboxes, parent),
_actor(actor)
{

}

ActorMailboxes::~ActorMailboxes()
{

}

Scalar 
ActorMailboxes::ReadMailbox(long mailbox) const
{
    if(mailbox >= EMAILBOX_LOCAL_SYSTEM_START && mailbox < EMAILBOX_LOCAL_SYSTEM_MAX)
        return _actor.ReadSystemMailbox(mailbox);
    else
        return MailboxesWithStorage::ReadMailbox(mailbox);
}


void 
ActorMailboxes::WriteMailbox(long mailbox, Scalar value)
{
    if(mailbox >= EMAILBOX_LOCAL_SYSTEM_START && mailbox < EMAILBOX_LOCAL_SYSTEM_MAX)
        _actor.WriteSystemMailbox(mailbox,value);
    else
        MailboxesWithStorage::WriteMailbox(mailbox, value);
}

//=============================================================================

#pragma message( "Put assertions in PutMsg() so that no messages are being sent to StatPlat's" )
Actor::NonStatPlatData::NonStatPlatData(SMemPool * memPool, const _Movement* moveBlock) :
	_hitPoints( Actor::INDESTRUCTIBLE_HP ),
	_shield( NULL ),
	_inputScript( NULL ),
	_input( &theNullInputDigital ),
	_pScript( NULL ),
   _movementManager(moveBlock),
	_animManager( &AnimationManagerNull::theAnimationManagerNull ),
   _msgPort(memPool),
	_nTools( 0 )
{
}

//==============================================================================


//==============================================================================
// level.cc:
// Copyright ( c ) 1994,1995,1996,1997,1998,1999,2000,2001,2002,2003 World Foundry Group.  
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
// Description: you name it, it's in here!
//==============================================================================

//#define USE_TEST_CAMERA

#define _LEVEL_CC

#include <anim/path.hp>
#include <movement/movement.hp>
#include <room/room.hp>
#include <physics/collision.hp>
#include <room/rooms.hp>
#include <room/actrooms.hp>
#include <room/actroit.hp>
#include <asset/assets.hp>
#include <asset/assslot.hp>

#include <hal/hal.h>
#include <hal/halconv.h>
#include <math/euler.hp>
#include <math/angle.hp>
#include <gfx/material.hp>
#include <gfx/rendmatt.hp>
#include <cpplib/libstrm.hp>
#include <cpplib/algo.hp>
#include <math/vector3.hp>
#if defined PHYSICS_ENGINE_ODE
#include <physics/ode/ode.hp>
#endif

#if defined( __PSX__ )
#	include <sys/types.h>
#	include <r3000.h>
#	include <asm.h>
#	include <kernel.h>
#	include <libgte.h>
#	include <libgpu.h>
#	include <libcd.h>
#	if defined( SOUND )
#		include <libsnd.h>
#	endif
#endif

#include <oas/levelobj.ht>
#include <oas/matte.ht>

//-----------------------------------------------------------------------------

#include "level.hp"
#include "actor.hp"
#include "camera.hp"
#include "levelobj.hp"
#include "game.hp"
#include "gamestrm.hp"

//==============================================================================
// objects.inc contains a list of includes to include all .hp files for game objects

#include "oas/objects.inc"

//==============================================================================
// this is where the case statment built by the oad conversion process is included

#include "oas/objects.c"

//==============================================================================
// Externs
//=============================================================================

extern Scalar FakeFrameRate;

#if defined(JOYSTICK_RECORDER)
extern bool bJoyPlayback;	  			// Set true to enable joystick playback
extern std::ifstream* joystickInputFile;		// File for joystick playback data
#endif

//==============================================================================
// Globals
//==============================================================================

bool gDoomStick = false;
bool gBungeeCam = false;

//[unused]int framenum = 0;

// If set to a nonzero value, break when wallClock reaches WALL_CLOCK_BREAKPOINT_VALUE
Scalar WALL_CLOCK_BREAKPOINT_VALUE = Scalar::zero;

//==============================================================================
// Functions
//==============================================================================

#if SW_DBSTREAM

std::ostream& operator <<( std::ostream& s, const Level& level )
{
	return level._print( s );
}

std::ostream&
Level::_print( std::ostream& s ) const
{
    s << "Level: " << std::endl;
	for (int i=0; i<_actors.Size(); ++i )
	{
		if ( _actors[ i ] )
		{
         Actor* actor = dynamic_cast<Actor*>(_actors[i]);
         assert(ValidPtr(actor));
			actor->printDetailed( s );
			s << std::endl;
		}
	}
	s << "Mailboxes: " << _mailboxes << std::endl;
	return s;
}

#endif // SW_DBSTREAM

//==============================================================================
// class Level
//==============================================================================

#if 0
void*
Level::findOADData( int idxActor ) const
{
	assert( idxActor > 0 );
	assert( idxActor <= _actors.Size() );
	int32 * objArray = ( int32 * )( (char * )_levelData + ( _levelData->objectsOffset ));
	_ObjectOnDisk * ood = ( _ObjectOnDisk * )( (( char * )_levelData ) + objArray[ idxActor ] );
	ValidatePtr( ood );
	++ood;	// oad data immediately follows the ood data
	return ood;
}
#endif

//==============================================================================
// Level constructor, this initializes a level
// we are going to rename this to World soon

void
Level::initLevelOad()
{
	// Find "LevelObj" object
	_levelOad = NULL;
	assert( ValidPtr( _levelData ) );
	assert( _levelData->objectCount);
	for ( int index = 0; index < _levelData->objectCount; index++ )
	{
		int32 * objArray = ( int32 * )( (char * )_levelData + ( _levelData->objectsOffset ));
		_ObjectOnDisk * objdata = ( _ObjectOnDisk * )( (( char * )_levelData ) + objArray[index] );
		if ( objdata->type == Actor::LevelObj_KIND )
		{
			AssertMsg( !_levelOad, "Multiple LevelObj objects in level" );
			_levelOad = ( _LevelObj * )( objdata + 1 );
		}
	}
	AssertMsg( _levelOad, "No LevelObject object in level" );
}

//==============================================================================

void
Level::initModels()
{
	// setup the 3D object table
	assert( ValidPtr( _levelData ) );
//	_modelData = ( _ModelOnDisk * )( (char * )_levelData + ( _levelData->modelsOffset ));
//	_modelData = NULL;	// Velocity is being weened off of filenames...
}

//==============================================================================

void
Level::constructObject( SObjectStartupData& startupData, int index )
{
	_ObjectOnDisk* objdata = startupData.objectData;
	assert( ValidPtr( objdata ));

	DBSTREAM2( cdebug << "Constructing object of type " << objdata->type << "..."; )
	AssertMsg( objdata->type, "Invalid object type %d" );
	assert( ValidPtr( &startupData ));
	assert( _actors[ index ] == NULL );
	startupData.idxActor = index;
	Actor * pCreatedActor = ConstructOadObject( objdata->type, &startupData );
	DBSTREAM2( cflow << "after Constructor " << std::endl; )
	//printf("creating actor index %d at addres %p\n",index,pCreatedActor);
//	if(index == 31)
//		printf("creating actor #31\n");
	_actors[ index ] = pCreatedActor;                             // this will null or set this entry
	if ( pCreatedActor )
	{
//	DBSTREAM2( cdebug << *pCreatedActor << std::endl; )
		assert( !_templateObjects[ index ] );

		if ( pCreatedActor->kind() == Actor::Player_KIND )
		{
			// only allow one main character for the level
			DBSTREAM2( cdebug << " constructing main character" << std::endl; )
			//[multiplayer]AssertMsg( mainCharacter() == NULL, "More than one Main Character placed in level ( only 1 allowed )" );
			setMainCharacter( pCreatedActor );
			updateMainCharacter();
		}
		else if ( pCreatedActor->kind() == Actor::Camera_KIND )
		{
			// only allow one camera for the level
			DBSTREAM2( cdebug << " constructing camera" << std::endl; )
			AssertMsg( _camera == NULL, "Multiple cameras found in level ( only 1 allowed )" );
			_camera = ( Camera * ) pCreatedActor;
			}
		else if ( pCreatedActor->kind() == Actor::Director_KIND )
		{
			// only allow one Director for the level
			DBSTREAM2( cdebug << " constructing director" << std::endl; )
			AssertMsg( _director == NULL, "Multiple Directors found in level ( only 1 allowed )" );
			_director = ( Director * ) pCreatedActor;
		}
		else
		{
			DBSTREAM2( cdebug << " ordinary object" << std::endl; )
		}
	}
	else if ( objdata->type == Actor::LevelObj_KIND )
	{
		DBSTREAM2( cdebug << " constructing level object" << std::endl; )
		#if defined( MIDI_MUSIC )
			if ( _levelOad->MidiMusicVabHeader && * ( _levelOad->MidiMusicVabHeader ))
				_theSound->addVabHeader( _levelOad->MidiMusicVabHeader, _levelOad->MidiMusicVabBody );
			if ( _levelOad->SoundEffectsVabHeader && * ( _levelOad->SoundEffectsVabHeader ))
				_theSound->addSfxHeader( _levelOad->SoundEffectsVabHeader, _levelOad->SoundEffectsVabBody );
		#endif
	}
	else
	{
		DBSTREAM2( cdebug << " object is templated " << std::endl; )
	}
}

//==============================================================================

void
Level::ConstructStartupData( SObjectStartupData& startupData, _ObjectOnDisk* objdata, int index )
{
	if ( objdata->pathIndex != PATH_NULL )
	{
// 	assert( 0 <= objdata->pathIndex && objdata->pathIndex < _numPaths );

		int32 * pathArray = ( int32 * )( (char * )_levelData + ( _levelData->pathsOffset ));
		int index = objdata->pathIndex;

		_PathOnDisk * pathData = ( _PathOnDisk * )( (( char * )_levelData ) + pathArray[index] );
//		_PathOnDiskEntry * entryArray = ( _PathOnDiskEntry * )( pathData + 1 );

//		startupData.pathData = entryArray;
		startupData.path = pathData;
	}
	else
		startupData.path = NULL;

	startupData.objectData = objdata;
	startupData.commonBlock = _commonBlock;
	startupData.idxCreator = 0;
	startupData.roomNum = ObjectIsInWhichRoom( index, _levelData );
	startupData.memory = _memory;
    startupData.currentTime = LevelClock();
    startupData.messagePortMemPool = theGame->MessagePortMemPool();
    startupData.mailboxes = &_mailboxes;

    //assert(ValidPtr(((char*)_levelData)  + _levelData->channelsOffset ));
    //startupData.channelArray = (int32*) ((char*)_levelData)  + _levelData->channelsOffset );
    //assert(ValidPtr(startupData.channelArray));

    startupData.levelOnDisk = &GetLevelData();

//	_joystickOutputFile = new (HALLmalloc) ofstream("test.txt");			// Used to output a joystick record file
}

//==============================================================================

Scalar
Level::EvalScript( const void* script, int objectIndex )
{
	ValidatePtr(script);
   Scalar data = _interpreter->RunScript(script,objectIndex);
	return data;
}


//==============================================================================

PRoomObjectListCheckFunc checkList[5] = 
{
   static_cast<PRoomObjectListCheckFunc>(&Actor::CanCollide),
   static_cast<PRoomObjectListCheckFunc>(&Actor::CanRender),
   static_cast<PRoomObjectListCheckFunc>(&Actor::CanUpdate),
   static_cast<PRoomObjectListCheckFunc>(&Actor::IsLight),
   static_cast<PRoomObjectListCheckFunc>(&Actor::IsActivationBox)
};

//==============================================================================

WorldFoundryMailboxesManager::WorldFoundryMailboxesManager(BaseObjectList& objects, Mailboxes& mailboxes) :
 _objects(objects),
 _mailboxes(mailboxes)
{

}

WorldFoundryMailboxesManager::~WorldFoundryMailboxesManager()
{

}


Mailboxes& 
WorldFoundryMailboxesManager::LookupMailboxes(int objectIndex)
{
    DBSTREAM1(cmailbox << "wfmbm: index = " << objectIndex << std::endl; )
    if(objectIndex)
    {
        BaseObject* obj = _objects[objectIndex];
        assert(ValidPtr(obj));
        return obj->GetMailboxes();
    }
    return _mailboxes;
}

//==============================================================================

Level::Level
(
	_DiskFile* diskFile
	,ViewPort& viewPort
    ,VideoMemory& videoMemory
    ,Mailboxes* parentMailboxes
)
  :
	_viewPort( viewPort ),
	_theLevelRooms( NULL ),
	_theActiveRooms( NULL ),
	_levelFile( diskFile ),
	_hardwareInput1( NULL ),
	_hardwareInput2( NULL ),
	_hardwareInput3( NULL ),
	_hardwareInput4( NULL ),
#if defined(JOYSTICK_RECORDER)
	_joystickPlaybackInput( NULL ),
	_joystickOutputFile( NULL ),
#endif
	_done( false ),
	_director( NULL ),
	_camera( NULL ),
	_mainCharacter( NULL ),
	_idealMainCharacter( NULL ),
	_numToBeRemovedObjects( 0 ),
	_camShotMailBox( 0 ),
    _scratchMailboxes(EMAILBOX_SCRATCH_START,EMAILBOX_SCRATCH_MAX-EMAILBOX_SCRATCH_START,parentMailboxes),
    _mailboxes(*this, &_scratchMailboxes),
    _mailboxesManager(this->_actors,_mailboxes)
{
	DBSTREAM1( cflow << "Level::Level:" << std::endl; )

#if 0                   // kts used to check levelcon.h sizes
    std::cout << "sizeof(_CollisionRectOnDisk) = " << sizeof(_CollisionRectOnDisk) << std::endl;
    std::cout << "sizeof(wf_euler) = " << sizeof(wf_euler) << std::endl;
    std::cout << "sizeof(_ObjectOnDisk) = " << sizeof(_ObjectOnDisk) << std::endl;
    std::cout << "sizeof(_RoomOnDiskEntry) = " << sizeof(_RoomOnDiskEntry) << std::endl;
    std::cout << "sizeof(_RoomOnDisk) = " << sizeof(_RoomOnDisk) << std::endl;
    std::cout << "sizeof(_PathOnDisk) = " << sizeof(_PathOnDisk) << std::endl;
    std::cout << "sizeof(_ChannelOnDiskEntry) = " << sizeof(_ChannelOnDiskEntry) << std::endl;
    std::cout << "sizeof(_ChannelOnDisk) = " << sizeof(_ChannelOnDisk) << std::endl;
    std::cout << "sizeof(_LevelOnDisk) = " << sizeof(_LevelOnDisk) << std::endl;
    std::cout << "LINEAR_COMPRESSION = " << LINEAR_COMPRESSION << std::endl;
    std::cout << "CONSTANT_COMPRESSION = " << CONSTANT_COMPRESSION << std::endl;
    std::cout << "RLE_COMPRESSION = " << RLE_COMPRESSION << std::endl;
    std::cout << "AMBIENT_LIGHT = " << AMBIENT_LIGHT << std::endl;
    std::cout << "DIRECTIONAL_LIGHT = " << DIRECTIONAL_LIGHT << std::endl;
    std::cout << "LEVEL_VERSION = " << LEVEL_VERSION << std::endl;

    std::cout << "offsetof(wf_euler.a) = " << offsetof(wf_euler,a) << std::endl;
    std::cout << "offsetof(wf_euler.b) = " << offsetof(wf_euler,b) << std::endl;
    std::cout << "offsetof(wf_euler.c) = " << offsetof(wf_euler,c) << std::endl;
    std::cout << "offsetof(wf_euler.order) = " << offsetof(wf_euler,order) << std::endl;

    std::cout << "offsetof(_ObjectOnDisk.type     ) = " << offsetof(_ObjectOnDisk,type     ) << std::endl;
    std::cout << "offsetof(_ObjectOnDisk.x        ) = " << offsetof(_ObjectOnDisk,x        ) << std::endl;
    std::cout << "offsetof(_ObjectOnDisk.y        ) = " << offsetof(_ObjectOnDisk,y        ) << std::endl;
    std::cout << "offsetof(_ObjectOnDisk.z        ) = " << offsetof(_ObjectOnDisk,z        ) << std::endl;
    std::cout << "offsetof(_ObjectOnDisk.x_scale  ) = " << offsetof(_ObjectOnDisk,x_scale  ) << std::endl;
    std::cout << "offsetof(_ObjectOnDisk.y_scale  ) = " << offsetof(_ObjectOnDisk,y_scale  ) << std::endl;
    std::cout << "offsetof(_ObjectOnDisk.z_scale  ) = " << offsetof(_ObjectOnDisk,z_scale  ) << std::endl;
    std::cout << "offsetof(_ObjectOnDisk.rotation ) = " << offsetof(_ObjectOnDisk,rotation ) << std::endl;
    std::cout << "offsetof(_ObjectOnDisk.coarse   ) = " << offsetof(_ObjectOnDisk,coarse   ) << std::endl;
    std::cout << "offsetof(_ObjectOnDisk.oadFlags ) = " << offsetof(_ObjectOnDisk,oadFlags ) << std::endl;
    std::cout << "offsetof(_ObjectOnDisk.pathIndex) = " << offsetof(_ObjectOnDisk,pathIndex) << std::endl;
    std::cout << "offsetof(_ObjectOnDisk.OADSize  ) = " << offsetof(_ObjectOnDisk,OADSize  ) << std::endl;
#endif

	assert( ValidPtr( diskFile ) );
	theLevel = this;
#if defined PHYSICS_ENGINE_ODE
   odeWorld.SetSpace(ode::dHashSpaceCreate(0));
#endif

#if defined(JOYSTICK_RECORDER)
	_joystickOutputFile = new (HALLmalloc) std::ofstream("joystick.out");
#endif

	// load the RAM configuration
	char memoryConfigurationBytes[ DiskFileCD::_SECTOR_SIZE ];
	struct levelMemoryConfiguration* plmc = (struct levelMemoryConfiguration*)memoryConfigurationBytes;
	assert( ValidPtr( plmc ) );

	assert( ValidPtr( _levelFile ) );
	// kts assumes diskFile is seeked to begining of the level iff chunk on disk
	// so this seek is done in game
	_levelFile->ReadBytes( memoryConfigurationBytes, DiskFileCD::_SECTOR_SIZE );
	assert( plmc->tagRam == IFFTAG('R','A','M','\0') );
	assert( plmc->tagObjects == IFFTAG('O','B','J','D') );
	assert( plmc->tagPerm == IFFTAG('P','E','R','M') );
	assert( plmc->tagRoom == IFFTAG('R','O','O','M') );
	assert( plmc->tagFlags == IFFTAG('F','L','A','G') );

	gDoomStick = plmc->doomStickFlag;
	gBungeeCam = plmc->bungeeCamFlag;

//	_memory = new (HALLmalloc) DMalloc(HALLmalloc,340000,MEMORY_NAMED("Level DMalloc"));
	Memory* newMemory = new (HALLmalloc) DMalloc(HALLmalloc,plmc->cbObjectsDRam,MEMORY_NAMED("Level DMalloc"));
	assert( ValidPtr( newMemory ) );
	_memory.Set(newMemory,&HALLmalloc);

	int index;

#if 0	//msvc
#if defined( TASKER )
	// set up the game's message port
	_port = MessagePortNewTask( "GAME" );
	VALIDATEITEM( _port );
#endif	// TASKER
#endif

	DBSTREAM1( cflow << "Level::Level: setting up sound device " << std::endl; )
	_theSoundDevice = new (HALLmalloc) SoundDevice;
	assert( _theSoundDevice );

	// loading screen

	DBSTREAM1( cflow << "Level::Level: setting up joysticks " << std::endl; )

#if defined(JOYSTICK_RECORDER)
	if (bJoyPlayback)
	{
		_joystickPlaybackInput = new (HALLmalloc) InputScript();
 		_hardwareInput1 = new (HALLmalloc) QInputDigital(_joystickPlaybackInput,HALLmalloc);
	}
	else
#endif	// JOYSTICK_RECORDER
		_hardwareInput1 = new (HALLmalloc) QInputDigital( new (HALLmalloc) InputJoystick(EJW_JOYSTICK1), HALLmalloc );

	assert( ValidPtr( _hardwareInput1 ) );

	_hardwareInput2 = new (HALLmalloc)QInputDigital( new (HALLmalloc)InputJoystick(EJW_JOYSTICK2), HALLmalloc );
	assert( ValidPtr( _hardwareInput2 ) );

	DBSTREAM1( cflow <<"Level::Level: loading level data" << std::endl; )
	assert(ValidPtr(_levelFile));

	_theLevelRooms = new (HALLmalloc) LevelRooms;
	assert(ValidPtr(_theLevelRooms));

   _roomCallbacks = new LevelRoomCallbacks(this);
   assert(ValidPtr(_roomCallbacks));

   _assetCallbackRoom = new AssetCallbackRoom(GetLevelRooms());
   assert(ValidPtr(_assetCallbackRoom));

	_theAssetManager = new (HALLmalloc) AssetManager( plmc->cbPerm, plmc->cbRoom,  videoMemory, *_levelFile, *_memory, *_assetCallbackRoom );
	ValidatePtr(_theAssetManager);
	LoadLevelData( );
	assert( ValidPtr( _levelData ) );
	printf("Level::Level: leveldata= %p, object count  = %ld\n",_levelData,_levelData->objectCount);
	DBSTREAM1( cprogress << "_levelData->versionNum = " << _levelData->versionNum << " and LEVEL_VERSION = " << LEVEL_VERSION << std::endl; )
	AssertMsg( _levelData->versionNum == LEVEL_VERSION, "Level version number doesn't match--probably need to rebuild level files" );

	assert( ValidPtr( (void*)( (char*)_levelData + ( _levelData->commonDataOffset )) ) );
	assert( _levelData->commonDataLength >= 0 );
   _commonBlock = new (HALLmalloc) CommonBlock((void*)( (char*)_levelData + ( _levelData->commonDataOffset )),_levelData->commonDataLength);
   _commonBlock->Validate();

   DBSTREAM1( cprogress << "theLevel = " << theLevel << ", commonBlock = " << *_commonBlock << std::endl; )

	DBSTREAM1( cprogress << "Level Loaded: Object Count: " << _levelData->objectCount <<
		", Path Count " << 	_levelData->pathCount <<
		", Channel Count: " << _levelData->channelCount <<
		", Room Count " << _levelData->roomCount <<
		", Common Data size " << _levelData->commonDataLength << std::endl; )

	initLevelOad();
	initModels();
	_theAssetManager->LoadPermanents();

	DBSTREAM1( cflow << "Level::Level: read in objects" << std::endl; )
	DBSTREAM1( cprogress << "Reading objects" << std::endl; )
	int32* objArray = ( int32* )( (char* )_levelData + ( _levelData->objectsOffset ));

	DBSTREAM3( cnull << "Number of temp objects = " << _levelOad->NumberOfTemporaryObjects << std::endl; )
   _actors.SetMax(_levelData->objectCount + _levelOad->NumberOfTemporaryObjects);
	// kts clear array
	for( int actIndex=0; actIndex<_levelData->objectCount + _levelOad->NumberOfTemporaryObjects; ++actIndex )
		_actors[actIndex] = NULL;

	_templateObjects = new (HALLmalloc)( SObjectStartupData * [_levelData->objectCount] );            // array of template object pointers, in levelcon order
	assert( ValidPtr( _templateObjects ) );

	objArray[0] = 0;

	for ( index = 0; index < _levelData->objectCount; ++index )
		_templateObjects[index] = NULL;                                         // kts be sure to null all non-used entries
	_numTemplateObjects = _levelData->objectCount;

	// Find and collect all template objects
	for ( index = 1; index < _levelData->objectCount; ++index )
	{
		DBSTREAM2( cdebug << "New Actor: index " << index << ", Level data = " << ( void * )_levelData << ", objArray[" << index << "] = " << objArray[index] << std::endl; )
		_ObjectOnDisk * objdata = ( _ObjectOnDisk * )( (( char * )_levelData ) + objArray[index] );

		_Actor* _oadData = (_Actor*)( objdata + 1 );
		assert( ValidPtr( _oadData ) );

		if ( _oadData->TemplateObject )
		{
			// if object is templated and not constructed upon startup, add to list
			// of OAD's to hold onto to be used when objects of this type come into existence
			DBSTREAM2( cdebug << " object is templated " << std::endl; )
			SObjectStartupData* elem = new (HALLmalloc) ( SObjectStartupData );
			assert( ValidPtr( elem ) );
			ConstructStartupData( *elem, objdata, index );
			elem->idxActor = 0;			// Currently unassigned

			int objectDataSize = sizeof( _ObjectOnDisk ) + objdata->OADSize;
			assert( objectDataSize > 0 );
			elem->objectData = ( _ObjectOnDisk * ) new (HALLmalloc)( char[objectDataSize] );
			assert( ValidPtr( elem->objectData ) );
			memcpy( elem->objectData, objdata, objectDataSize );

         elem->levelOnDisk = &GetLevelData();
         //assert(ValidPtr(((char*)_levelData)  + _levelData->channelsOffset ));
         //elem->channelArray = (int32*) ((char*)_levelData)  + _levelData->channelsOffset );
         //assert(ValidPtr(elem->channelArray));

			_templateObjects[index] = elem;
		}
	}

	for ( index = 1; index < _levelData->objectCount; ++index )
	{
		DBSTREAM1( cflow <<"Level::Level: reading object #" << index << std::endl; )
		DBSTREAM2( cdebug << "New Actor: index " << index << ", Level data = " << ( void * )_levelData << ", objArray[" << index << "] = " << objArray[index] << std::endl; )
		_ObjectOnDisk * objdata = ( _ObjectOnDisk * )( (( char * )_levelData ) + objArray[index] );

		SObjectStartupData startupData;
		ConstructStartupData( startupData, objdata, index );

		constructObject( startupData, index );

		DBSTREAM2( cdebug << "Level::Level: Done" << std::endl; )
	}

	// make sure that we have a camera
	// fail if no Camera object in level data
	AssertMsg( ValidPtr( _camera ), "No camera in level" );

	_theLevelRooms->InitRooms(_levelData->roomCount,_levelData,checkList, _actors, _roomCallbacks,GetMemory(),GetLevelOAD());
	_theActiveRooms = new (HALLmalloc) ActiveRooms(HALLmalloc,*_theAssetManager,*_theLevelRooms);
   assert(ValidPtr(_theActiveRooms));
	_theActiveRooms->Construct(_levelData->roomCount);
//	DBSTREAM1( cstats << "midi header = " << _levelOad->MidiMusicVabHeader << std::endl; )

	{ // load sound effects
	assert( ValidPtr( _theSoundDevice ) );
	assert( ValidPtr( _levelOad ) );
	(void)_levelOad->sfx127;		// causes compiler error if number of entries shrinks

	int32* pSoundEffectsBank = &( _levelOad->sfx0 );
	for ( int idxSoundEffect=0; idxSoundEffect<128; ++idxSoundEffect )
	{
		assert( ValidPtr( pSoundEffectsBank ) );
		if ( pSoundEffectsBank[ idxSoundEffect ] )
		{
			binistream binis = GetAssetManager().GetAssetStream( pSoundEffectsBank[ idxSoundEffect ]);
			assert( binis.good() );
			_sfx[ idxSoundEffect ] = _theSoundDevice->CreateSoundBuffer( binis );
			assert( ValidPtr( _sfx[ idxSoundEffect ] ) );
		}
		else
			_sfx[ idxSoundEffect ] = NULL;
	}
	}

   // now call reset on all actors in all rooms
   for(int roomIndex=0;roomIndex<_theLevelRooms->NumberOfRooms();roomIndex++)     // kts: there should be a Room iterator
   {
      const Room& room = _theLevelRooms->GetRoom( roomIndex );
      BaseObjectIteratorWrapper iter = room.ListIter(ROOM_OBJECT_LIST_UPDATE);
   
      while(!iter.Empty())
      {
         Actor* actor = dynamic_cast<Actor*>(&(*iter));
         assert(ValidPtr(actor));   
         actor->reset();
         ++iter;
      }
   }

	DBSTREAM1( cprogress << "Done loading level data" << std::endl; )
	assert( ValidPtr( mainCharacter() ) );

	DBSTREAM1( cprogress << "Doing reset" << std::endl; )
	reset();

	DBSTREAM1( cprogress << "common block stuff" << std::endl; )
	DBSTREAM1( cflow << "Level::level: common block stuff" << std::endl; )
	{
		_LevelObj* pActorData = _levelOad;
		assert( ValidPtr( pActorData ) );

		int commonDataOffset = pActorData->commonPageOffset;
		assert( (commonDataOffset & 3) == 0 );
		_Common* commonPage = (_Common*)_commonBlock->GetBlockPtr(commonDataOffset);
		assert( ValidPtr( commonPage ) );

		if ( commonPage->Script != -1 )
		{	// Run the script for the level object [if any] after all objects are constructed and only once
			const void* pScript = _commonBlock->GetBlockPtr(commonPage->Script);
         EvalScript(pScript,0);
		}
	}

   _interpreter = ScriptInterpreterFactory(_mailboxesManager,*_memory);
   assert(ValidPtr(_interpreter));

	DBSTREAM1( cprogress << "Done Loading Level" << std::endl; )
	DBSTREAM1( casset << *_theAssetManager << std::endl; )
}

//==============================================================================
// Level destructor: this shuts down a level

Level::~Level()
{
#if defined( DESIGNER_CHEATS ) && defined( WRITER )
	saveTextureBuffer( "textures.tga" );
#endif

	for ( int idxSoundEffect=0; idxSoundEffect<128; ++idxSoundEffect )
		delete _sfx[ idxSoundEffect ];

#if defined( MIDI_MUSIC )
	DELETE_CLASS( _theSound );
#endif

#if defined(JOYSTICK_RECORDER)
	MEMORY_DELETE(HALLmalloc, _joystickOutputFile, std::ofstream );
#endif
	ValidatePtr(_theActiveRooms);
	MEMORY_DELETE(HALLmalloc,_theActiveRooms,ActiveRooms);

	ValidatePtr(_theLevelRooms);
	MEMORY_DELETE(HALLmalloc,_theLevelRooms,LevelRooms);

	DBSTREAM1( cprogress << "~Level:: deleting template objects" << std::endl; )
	for ( int idxActor = 0; idxActor < _numTemplateObjects; ++idxActor )
	{
		if ( _templateObjects[idxActor] != 0 )
		{
			HALLmalloc.Free(_templateObjects[idxActor]->objectData);
			HALLmalloc.Free(_templateObjects[idxActor]);
		}
	}
	assert( ValidPtr( _templateObjects ) );
	HALLmalloc.Free(_templateObjects, sizeof(SObjectStartupData*) * _levelData->objectCount);

	DBSTREAM1( cprogress << "~Level:: deleting actors" << std::endl; )
	for ( int actorIndex = 0; actorIndex < _actors.Size(); actorIndex++ )
	{
		if ( _actors[actorIndex] != NULL )
		{
			DBSTREAM4( cprogress << "  Deleting actor of type " << _actors[actorIndex]->kind() << std::endl; )
			MEMORY_DELETE((*_memory),_actors[actorIndex],Actor);
		}
	}

	DBSTREAM1( cprogress << "~Level:: deleting mailboxes" << std::endl; )
	//assert( ValidPtr( _scratchMailboxes ) );

   // kts kludge since _actors doesn't get destructed yet, but it allocated its memory at this stage in the constructor
   // so this just resets the LMalloc pointer
   // HALLmalloc.Free(((char*)(_scratchMailboxes)+sizeof(Scalar)*(_numScratchMailboxes+EMAILBOX_SCRATCH_SYSTEM_MAX-EMAILBOX_SCRATCH_SYSTEM_START)));
	//HALLmalloc.Free(_scratchMailboxes,sizeof(Scalar)*(_numScratchMailboxes+EMAILBOX_SCRATCH_SYSTEM_MAX-EMAILBOX_SCRATCH_SYSTEM_START));

	DBSTREAM1( cprogress << "~Level:: deleting leveldata" << std::endl; )
	HALLmalloc.Free(_levelOnDiskMemory);

	MEMORY_DELETE(HALLmalloc,_commonBlock,CommonBlock);
	MEMORY_DELETE(HALLmalloc,_theAssetManager,AssetManager);

	MEMORY_DELETE(HALLmalloc,_hardwareInput2,QInputDigital);
	MEMORY_DELETE(HALLmalloc,_hardwareInput1,QInputDigital);
#if defined(JOYSTICK_RECORDER)
	if (bJoyPlayback)
		MEMORY_DELETE(HALLmalloc,_joystickPlaybackInput,InputScript);
#endif

   delete _roomCallbacks;
   delete _assetCallbackRoom;

	theLevel = NULL;
	DBSTREAM1( cprogress << "~Level:: finished" << std::endl; )
}

//==============================================================================
// This routine loops through all of the actors and runs their update code

class ActorStartFrame
{
public:
	inline void operator() (BaseObject& bo)
	{
      Actor* actor = dynamic_cast<Actor*>(&bo);
		assert(ValidPtr(actor));
		actor->StartFrame();
	}
};

//==============================================================================
// this updates all of the actors in a level, this should run once per frame
// this is where most of the work of the game loop is done

void
Level::update(Scalar deltaTime)
{
	Validate();
	DBSTREAM2( cflow <<"Level::update:" << std::endl; )

	DBSTREAM2( cflow <<"Level::update: reading clock" << std::endl; )
	// Fake clock updates, make game think the frame rate is a fixed value

//	deltaTime = _MAX( deltaTime, SCALAR_CONSTANT(0.1) );
	if ( deltaTime > SCALAR_CONSTANT(0.1) )  // never drop logical frame rate below 10 hz
		deltaTime = SCALAR_CONSTANT(0.1);

#pragma message( __FILE__ ": replace with bool( FakeFrameRate ):: need bool() operator in Scalar" )
	Scalar newtime = levelClock() + ( FakeFrameRate.AsBool() ? FakeFrameRate : deltaTime );

#if 0	//kts crude frame rate printing
	static int counter=0;
	if(!(counter % 100))
		std::cout << "delta time = " << (Scalar::one / deltaTime).WholePart() << std::endl;
	counter++;
#endif

#if defined(JOYSTICK_RECORDER)
// If we're playing back a joystick file, use its values to drive the clock and input
 if ( bJoyPlayback && joystickInputFile->good())
 {
 	int32 theButtons;
 	char joystickBuffer[80];
 	ASSERTIONS( Vector3 cthugPos = mainCharacter()->GetPhysicalAttributes().Position(); )
 	joystickInputFile->getline(joystickBuffer, 79, '\n');
   if(joystickInputFile->eof())
   {
      DBSTREAM1( cerror << "---ran out of joystick input---" << std::endl; )
      sys_exit(1);
   }
#if defined(SCALAR_TYPE_FIXED)
 	int32 xPos, yPos, zPos;
 	int numScanned = sscanf(joystickBuffer, "%lx %lx %lx %lx %lx", (int32*)&newtime, &theButtons, &xPos, &yPos, &zPos);
   AssertMsg(numScanned == 5, "problem scanning joystick input file, line = " << joystickBuffer);
 	AssertMsg( cthugPos.X().AsLong() == xPos, "Something changed since this joystick file was recorded!, cthugPos.X = " << std::hex << cthugPos.X().AsLong() << ", stored X=" << std::hex << xPos );
 	AssertMsg( cthugPos.Y().AsLong() == yPos, "Something changed since this joystick file was recorded!, cthugPos.Y = " << std::hex << cthugPos.Y().AsLong() << ", stored Y=" << std::hex << yPos );
 	AssertMsg( cthugPos.Z().AsLong() == zPos, "Something changed since this joystick file was recorded!, cthugPos.Z = " << std::hex << cthugPos.Z().AsLong() << ", stored Z=" << std::hex << zPos );
 	_joystickPlaybackInput->setButtons(theButtons);
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
   FLOAT_TYPE temptime;
 	FLOAT_TYPE xPos, yPos, zPos;
#if defined(SCALAR_TYPE_DOUBLE)
 	int numScanned = sscanf(joystickBuffer, "%lf %lx %lf %lf %lf", &temptime, &theButtons, &xPos, &yPos, &zPos);
#else
 	int numScanned = sscanf(joystickBuffer, "%f %lx %f %f %f", &temptime, &theButtons, &xPos, &yPos, &zPos);
#endif
   newtime = Scalar(temptime);
   AssertMsg(numScanned == 5, "problem scanning joystick input file, line = " << joystickBuffer);
 	AssertMsg( cthugPos.X() == xPos, "Something changed since this joystick file was recorded!, cthugPos.X = " << cthugPos.X() << ", stored X=" << xPos );
 	AssertMsg( cthugPos.Y() == yPos, "Something changed since this joystick file was recorded!, cthugPos.Y = " << cthugPos.Y() << ", stored Y=" << yPos );
 	AssertMsg( cthugPos.Z() == zPos, "Something changed since this joystick file was recorded!, cthugPos.Z = " << cthugPos.Z() << ", stored Z=" << zPos );
 	_joystickPlaybackInput->setButtons(theButtons);

#else
#error SCALAR TYPE not defined
#endif

 }
#endif

	DBSTREAM2( cflow <<"Level::update: done reading clock" << std::endl; )

	// If _nWallClock is zero, this is the first frame of the game, and we must do some tap dancing to
	// deal the the discontinuity in _deltaClock values.
	levelClock.Tick( newtime );

	DBSTREAM2( cflow << "_nWallClock = " << levelClock() << std::endl; )

	DBSTREAM2( cflow << "reading joysticks" << std::endl; )
	if(_hardwareInput1)							// update all hardware input readers
		_hardwareInput1->update();
	if(_hardwareInput2)
		_hardwareInput2->update();
	if(_hardwareInput3)
		_hardwareInput3->update();
	if(_hardwareInput4)
		_hardwareInput4->update();

#if defined(JOYSTICK_RECORDER)
	DBSTREAM2( cflow << "dumping joystick recorder data" << std::endl; )
	// dump timestamps and joystick bits to the log file
	_joystickOutputFile->width(8);
	_joystickOutputFile->fill('0');


// kts clearly we need serialization functions, then these sorts of ifdefs can go away
#if defined(SCALAR_TYPE_FIXED)
	*_joystickOutputFile << std::hex << levelClock().AsLong() << " ";
	_joystickOutputFile->width(8);
	_joystickOutputFile->fill('0');
	*_joystickOutputFile << (long)(_hardwareInput1->arePressed()) << " ";
	_joystickOutputFile->width(8);
	_joystickOutputFile->fill('0');
	*_joystickOutputFile << mainCharacter()->GetPhysicalAttributes().Position().X().AsLong() << " ";
	_joystickOutputFile->width(8);
	_joystickOutputFile->fill('0');
	*_joystickOutputFile << mainCharacter()->GetPhysicalAttributes().Position().Y().AsLong() << " ";
	_joystickOutputFile->width(8);
	_joystickOutputFile->fill('0');
	*_joystickOutputFile << mainCharacter()->GetPhysicalAttributes().Position().Z().AsLong() << std::endl;
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
	*_joystickOutputFile << levelClock() << " ";
	_joystickOutputFile->width(8);
	_joystickOutputFile->fill('0');
	*_joystickOutputFile << (long)(_hardwareInput1->arePressed()) << " ";
	_joystickOutputFile->width(8);
	_joystickOutputFile->fill('0');
	*_joystickOutputFile << mainCharacter()->GetPhysicalAttributes().Position().X()<< " ";
	_joystickOutputFile->width(8);
	_joystickOutputFile->fill('0');
	*_joystickOutputFile << mainCharacter()->GetPhysicalAttributes().Position().Y()<< " ";
	_joystickOutputFile->width(8);
	_joystickOutputFile->fill('0');
	*_joystickOutputFile << mainCharacter()->GetPhysicalAttributes().Position().Z()<< std::endl;
#else
#error SCALAR TYPE not defined
#endif

#endif

#if defined( __PSX__)
	extern void ViewVideoMemory();
 	if ( _hardwareInput1->justPressed( EJ_BUTTONF_E | EJ_BUTTONF_F | EJ_BUTTONF_G | EJ_BUTTONF_H) == ( EJ_BUTTONF_E | EJ_BUTTONF_F | EJ_BUTTONF_G | EJ_BUTTONF_H) )
		ViewVideoMemory();
#endif

#if DO_DEBUGGING_INFO
	// If WALL_CLOCK_BREAKPOINT_VALUE is set using the debugger, break here
	// when that time value is reached.
	if ( (WALL_CLOCK_BREAKPOINT_VALUE != Scalar::zero) && (WALL_CLOCK_BREAKPOINT_VALUE <= levelClock()) )
	{
		breakpoint();
		WALL_CLOCK_BREAKPOINT_VALUE = Scalar::zero;	// don't trigger again next frame
	}
#endif
	updateMainCharacter();

	// make sure any pending room request gets fulfilled
	assert( ValidPtr( _theActiveRooms ) );
	_theActiveRooms->WaitRoomLoad( false );

	DBSTREAM2( cflow << "Level::update: updating current room selection" << std::endl; )
	assert(ValidPtr(_camera));
	assert(ValidPtr(_camera->GetWatchObject()));
	_theActiveRooms->UpdateRoom(_camera->GetWatchObject());

	DBSTREAM1( ccollision << std::endl << "*** FRAME BOUNDARY ***	Wall clock = " << levelClock() << std::dec << std::endl << std::endl; )

	DBSTREAM2( cflow << "Level::StartFrame:" << std::endl; )
	Validate();
	for_each(GetActiveRooms().GetObjectIter(ROOM_OBJECT_LIST_UPDATE),ActorStartFrame());
	DBSTREAM2( cflow << "Level::StartFrame: done" << std::endl; )

	Validate();
    PredictPosition(GetActiveRooms().GetObjectIter(ROOM_OBJECT_LIST_UPDATE), LevelClock());
	DBSTREAM2( cflow << "Level::update: detecting collisions" << std::endl; )
	Validate();
	ValidatePtr(_theActiveRooms);
 	_theActiveRooms->DetectCollision(levelClock);
	Validate();
	DBSTREAM3 (cflow << "Level::UpdateActors:" << std::endl; )
    UpdatePhysics(GetActiveRooms().GetObjectIter(ROOM_OBJECT_LIST_UPDATE), LevelClock());

	DBSTREAM2( cflow << "Level::update: room contents" << std::endl; )
	Validate();
	updateRoomContents();
	// FIX - manually update director until we get priorities working in updates
	// right now it is not a renderAndUpdate object
	if ( _director )
	{
		DBSTREAM2( cflow << "Level::update: update director" << std::endl; )
		_director->predictPosition(LevelClock());
		_director->update();
	}

	DBSTREAM2( cflow << "Level::update: remove pending objects" << std::endl; )
	// execute pending deletetions
	removePendingObjects();

	DBSTREAM2( cflow << "Level::update: update sound" << std::endl; )
	updateSound();

	DBSTREAM2( cflow << "Level::update: done" << std::endl; )
	Validate();
}

//==============================================================================

void
Level::updateSound()
{
#if defined( MIDI_MUSIC )
	_theSound->updateSound();
	SetMailbox( EMAILBOX_MIDI, 0 );
#endif
}

//==============================================================================
// check for objects which have moved out of the rooms they are in
// if so, move to the new containing room
// PERF - only check for objects which moved

// kts put in Room class

void
Level::updateRoomContents()
{
	DBSTREAM2( cflow << "Level::updateRoomContents:" << std::endl; )
	Validate();
	ValidatePtr(_theActiveRooms);
	ActiveRoomsIter roomIter(_theActiveRooms->GetMutableIter());

	while(!roomIter.Empty())
	{
		(*roomIter).UpdateRoomContents(ROOM_OBJECT_LIST_UPDATE,GetMutableLevelRooms());
		++roomIter;
	}

	Validate();
	DBSTREAM2(cflow << "Level::updateRoomContents: done" << std::endl; )
}

//==============================================================================
// this removes ( but does not necessarily delete ) all of the objects which
// flagged themselves for removal this frame
// ( this should go away - KTS )

void
Level::removePendingObjects()
{
	Validate();
	for ( int toBeRemObjIndex = 0; toBeRemObjIndex < _numToBeRemovedObjects; ++toBeRemObjIndex )
	{
		int32 idxActor = _toBeRemovedObjects[toBeRemObjIndex];
		AssertMsg( idxActor > 0, "Tried to remove pending object #" << idxActor << std::endl );
		DBSTREAM2( clevel << " Removing actor #" << idxActor << " from _toBeRemovedObjects[" << toBeRemObjIndex << "]" << std::endl; )

      BaseObject* bo = GetObject(idxActor);
      assert(ValidPtr(bo));
      Actor* actor = dynamic_cast<Actor*>(bo);
      assert(ValidPtr(actor));
		actor->spawnPoof();
		DBSTREAM2( clevel << " removeactor " << idxActor << " from room" << std::endl; )
		ValidatePtr(_theLevelRooms);
		_theLevelRooms->RemoveObjectFromRoom( idxActor );

//		bool isTemp = idxActor >= _numActors;
//		DBSTREAM2( clevel << " delete if temp" << std::endl; )

		// delete the Actor
		DBSTREAM2( clevel << " deleting " << actor << std::endl; )
		MEMORY_DELETE( *_memory, actor, Actor );
		//DELETE_CLASS( actor );

		// remove from _actors[] array
		assert( idxActor > 0 );
		assert( ValidPtr( _actors[ idxActor ] ) );
		_actors[ idxActor ] = NULL;

		// remove from remove list
		DBSTREAM2( clevel << " nulling entry " << actor << std::endl; )
		_toBeRemovedObjects[toBeRemObjIndex] = 0;
	}
	_numToBeRemovedObjects = 0;
	Validate();
}

//==============================================================================

#if defined(USE_TEST_CAMERA)

#pragma message ("KTS: test code: remove this function")
static void
AdjustCameraParameters(Vector3& position,Euler& rotation)
{
	u_long	padd = PadRead(1);

	padd >>= 16;
	if (padd & PADRup)
		rotation.SetA(rotation.GetA() + Angle(Angle::Degree(SCALAR_CONSTANT(1))));
	if (padd & PADRdown)
		rotation.SetA(rotation.GetA() - Angle(Angle::Degree(SCALAR_CONSTANT(1))));
	if (padd & PADRleft)
		rotation.SetB(rotation.GetB() - Angle(Angle::Degree(SCALAR_CONSTANT(1))));
	if (padd & PADRright)
		rotation.SetB(rotation.GetB() + Angle(Angle::Degree(SCALAR_CONSTANT(1))));
	if (padd & PADR1)
		rotation.SetC(rotation.GetC() - Angle(Angle::Degree(SCALAR_CONSTANT(1))));
	if (padd & PADR2)
		rotation.SetC(rotation.GetC() + Angle(Angle::Degree(SCALAR_CONSTANT(1))));

	if (padd & PADLup)
		position.SetY(position.Y() + SCALAR_CONSTANT(0.5));
	if (padd & PADLdown)
		position.SetY(position.Y() - SCALAR_CONSTANT(0.5));
	if (padd & PADLleft)
		position.SetX(position.X() - SCALAR_CONSTANT(0.5));
	if (padd & PADLright)
		position.SetX(position.X() + SCALAR_CONSTANT(0.5));

	if (padd & PADL1)
		position.SetZ(position.Z() - SCALAR_CONSTANT(0.5));
	if (padd & PADL2)
		position.SetZ(position.Z() + SCALAR_CONSTANT(0.5));

	cscreen << "camera position:" << std::endl << position << std::endl;
	cscreen << "camera rotation:" << std::endl << rotation << std::endl;
}
#endif

//==============================================================================
// this routine loops through all of the actors, adding them to brender's render
// list for this frame, and then tells brender to render it

void
Level::RenderScene()
{
	DBSTREAM1( cdebug << "Level::RenderScene" << std::endl; )
	Validate();
	_theActiveRooms->Validate();

	_viewPort.Validate();
	_viewPort.Clear();
	_theActiveRooms->Validate();
	_viewPort.Validate();

#if defined(USE_TEST_CAMERA)
	cscreen << "--- using test camera ---" << std::endl;
	static RenderCamera testCamera(_viewPort);
	static Vector3 cameraPosition(SCALAR_CONSTANT(-7.5),SCALAR_CONSTANT(20),SCALAR_CONSTANT(0.5));
	static Euler cameraRotation(Angle::Degree(SCALAR_CONSTANT(270)),Angle::Degree(SCALAR_CONSTANT(180)),0);

	// animate positions
	AdjustCameraParameters(cameraPosition,cameraRotation);

	Matrix34 camRot(Euler::zero-cameraRotation,Vector3::zero);
	Matrix34 camPos(Vector3::zero-cameraPosition);

	Matrix34 camMat = camPos * camRot;
	testCamera.SetPosition(camMat);
#endif

	_theActiveRooms->Validate();
	ActiveRoomsIterConst roomIter(_theActiveRooms->GetIter());
	while(!roomIter.Empty())
	{
		DBSTREAM3( cdebug << "----- room " << *roomIter << " -----" << std::endl; )
				// in case there are none in this room
		_camera->GetRenderCamera().SetAmbientColor( Color::black );
		_camera->GetRenderCamera().SetDirectionalLight(0,Vector3::unitX,Color48(PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0)));
		_camera->GetRenderCamera().SetDirectionalLight(1,Vector3::unitX,Color48(PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0)));
		_camera->GetRenderCamera().SetDirectionalLight(2,Vector3::unitX,Color48(PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0)));
//		_camera->GetRenderCamera().SetFog(Color(0,0,0),SCALAR_CONSTANT(10),SCALAR_CONSTANT(30));

		Validate();
		const Room * cur_room = &(*roomIter);
		ValidatePtr( cur_room );
		cur_room->Validate();

		// kts set up lights for this room
      BaseObjectIteratorWrapper lightIter = cur_room->ListIter(ROOM_OBJECT_LIST_LIGHT);

		int directionalLightIndex = 0;
		int ambientLightIndex = 0;
		while(!lightIter.Empty())
		{
         BaseObject& bo = *lightIter;
	  		const Actor* actor = dynamic_cast<Actor*>(&bo);
			assert(ValidPtr(actor));
			assert(actor->kind() == Actor::Light_KIND);
			const Light* light = (const Light*)actor;

			if(light->Type() == DIRECTIONAL_LIGHT)
			{
				assert(directionalLightIndex < 3);
				light->Set(*_camera,directionalLightIndex);
				directionalLightIndex++;
			}
			else
			{
				assert(light->Type() == AMBIENT_LIGHT);
				assert(ambientLightIndex < 1);
				light->Set(*_camera,-1);
				ambientLightIndex++;
			}
			++lightIter;
		}

#if SW_DBSTREAM
		if(directionalLightIndex == 0 && ambientLightIndex == 0)
			std::cerr << "room " << *roomIter << " has no lights, gonna be hard to see!" << std::endl;
#endif

		_camera->RenderBegin();
		BaseObjectIteratorWrapper poIter = cur_room->ListIter(ROOM_OBJECT_LIST_RENDER);
		while( !poIter.Empty() )
		{
	  		Actor* const actor = dynamic_cast<Actor*>(&(*poIter));
			assert(ValidPtr(actor));

			if ( actor && actor->isVisible() )
			{
#if defined(USE_TEST_CAMERA)
				actor->GetRenderActor().Render(testCamera,*actor);
#else
				_camera->Render(*actor,LevelClock());
#endif
			}
			++poIter;
	 	}
		_camera->RenderEnd();
		++roomIter;
	}
	_viewPort.Render();
	Validate();
}

//==============================================================================
// this is supposed to reset the level to its initial state

void
Level::reset( )
{
	DBSTREAM1( cflow << "Level::reset()" << std::endl; )

	int i;
	_numToBeRemovedObjects = 0;
	if ( NULL != _camera )
		_camera->reset();

	//initMailboxes();

	DBSTREAM1( cflow << "Level::reset(): temp objects" << std::endl; )
   // temp objects go immediatly after the level objects in the array, so to iterate the temp objects start at objectCount
	for ( i = _levelData->objectCount; i < _actors.Size(); i++ )
	{
		if ( _actors[i] != NULL )
			MEMORY_DELETE( *_memory, _actors[i], Actor );
		_actors[i] =NULL;
	}

	DBSTREAM1( cflow << "Level::reset(): initActiveRoom" << std::endl; )
	// load the rooms where the main character is
	PhysicalObject* main_char = mainCharacter();
	assert( ValidPtr( main_char ) );

   int32 roomIndex = ObjectIsInWhichRoom( GetObjectIndex(main_char),  _levelData );
	_theActiveRooms->InitActiveRoom( roomIndex, GetLevelRooms() );

#if defined( MIDI_MUSIC )
	DBSTREAM1( cflow << "Level::reset(): sound" << std::endl; )
	_theSound->reset();
#endif

	// Comented this out, because the clock code needs to be able to deal with the discontinuity in wall clock values at startup
	levelClock.reset();
	DBSTREAM1( cflow << "Level::reset() done" << std::endl; )
}

//==============================================================================

#pragma message ("KTS: remove this in release")

//==============================================================================

int
Level::GetObjectIndex( const BaseObject* object )
{
	assert( ValidPtr( object ) );
	for( int objectIndex=1;objectIndex<_actors.Size();objectIndex++ )
	{
		if( object == GetObject( objectIndex ))
			return objectIndex;
	}
	return 0;
}

//==============================================================================
// this adds a temporary actor to the level, and places it into the correct room

void
Level::AddObject( BaseObject* object, const Vector3& posStartAt )
{
	assert(ValidPtr(object));
	AssertMsg( GetObjectIndex( object ) == 0, "Attempting to AddObject() twice on same object : " << object );

	bool added = false;
	// add to temporary object list
	int idxTempObject;
	for ( idxTempObject = _levelData->objectCount; idxTempObject < _actors.Size(); ++idxTempObject )
	{
		if ( !_actors[idxTempObject] )
		{
			_actors[idxTempObject] = object;
         Actor* actor = dynamic_cast<Actor*>(object);
			actor->SetActorIndex( idxTempObject );
			added = true;
         assert(idxTempObject == GetObjectIndex( object ));
         assert(actor->GetActorIndex() > 0);
         assert(actor->GetActorIndex() == idxTempObject);
         // FIX - should we really be setting these things here?
         // there may be a better place to set the actor's initial position.
         actor->setCurrentPos( posStartAt );
         PhysicalAttributes& physAttrib = actor->GetWritablePhysicalAttributes();
         physAttrib.SetPredictedPosition( posStartAt );

         ValidatePtr(_theLevelRooms);
         _theLevelRooms->AddObjectToRoom( idxTempObject );
         DBSTREAM2( clevel << "AddObject: added object #" << idxTempObject << std::endl;)
			break;
		}
	}

	AssertMsg( added,"Too many temporary objects - out of allocated space\n" );
}

//==============================================================================
//  mark this as an object which wishes to be removed at the end of the frame
void
Level::SetPendingRemove( const BaseObject* object )
{
	assert( ValidPtr( object ) );
	AssertMsg( object->kind() != BaseObject::StatPlat_KIND, "Cannot remove a statplat" );

   if ( object == (PhysicalObject*)camera() )
   {
      DBSTREAM1( cwarn << "refusing to remove camera object!!" << std::endl; )
      return;
   }

   const Actor* actor = dynamic_cast<const Actor*>(object);
	assert( ValidPtr( actor ) );

	int32 idxActor = actor->GetActorIndex();
	assert(idxActor > 0);
	assert(idxActor == GetObjectIndex(actor));

	DBSTREAM2( clevel << "SetPendingRemove: actor = " << *actor << std::endl; )
	bool found = false;
	assert( _numToBeRemovedObjects < 99 );

	// check if already set to be removed
	for ( int i = 0; i < _numToBeRemovedObjects; i++ )
	{
		if ( _toBeRemovedObjects[i] == idxActor )
		{
			DBSTREAM2( clevel << " already placed Actor " << idxActor << " into _toBeRemovedObjects[" << i << "]" << std::endl; )
			found = true;
			break;
		}
	}
	if ( !found )
	{
		DBSTREAM2( clevel << " Placing Actor " << GetObjectIndex( actor ) << " into _toBeRemovedObjects[" << _numToBeRemovedObjects << "]" << std::endl; )
		_toBeRemovedObjects[_numToBeRemovedObjects] = idxActor;
		_numToBeRemovedObjects++;
	}
}

//==============================================================================
// Level::loadLevelData: retreive level data from storage device into memory
//==============================================================================

void
Level::LoadLevelData()
{
	DBSTREAM1( cflow <<"Level::loadLevelData:" << std::endl; )
	ValidatePtr(_theAssetManager);
	DBSTREAM2( cflow <<"Level::loadLevelData:reading toc from position " << _levelFile->FilePos() << std::endl; )
	_theAssetManager->LoadTOC(*_levelFile,_levelFile->FilePos());
	DBSTREAM2( cflow <<"Level::loadLevelData:done reading toc" << std::endl; )

	const DiskTOC::TOCEntry& asmaptocEntry = _theAssetManager->GetTOCEntry(AssetManager::TOC_ASMAP_INDEX);
   DBSTREAM2(clevel << "Level::loadLevelData: TOCEntry for assets: offset: " << asmaptocEntry._offsetInDiskFile << ", size = " << asmaptocEntry._size << std::endl;)
	_levelFile->SeekForward(asmaptocEntry._offsetInDiskFile);
   DBSTREAM2(clevel << "asmaptoc: seek to " << asmaptocEntry._offsetInDiskFile << std::endl; )
	// now read asset map into memory

	const int MAX_ASMP_SIZE = DiskFileCD::_SECTOR_SIZE * 4;  // kts abritrary
	char* mapMem = new ( HALScratchLmalloc ) char[MAX_ASMP_SIZE];
	assert(ValidPtr(mapMem));
	DBSTREAM2( cflow <<"Level::loadLevelData:reading map" << std::endl; )
	_levelFile->ReadBytes(mapMem,DiskFileCD::_SECTOR_SIZE);
	int mapStreamSize;
	{
//		binistream mapStream((void*)mapMem,DiskFileCD::_SECTOR_SIZE);

//		IFFChunkIter mapChunkIter(mapStream);
		mapStreamSize = *((long*) (mapMem+4));
		mapStreamSize += DiskFileCD::_SECTOR_SIZE - (mapStreamSize % DiskFileCD::_SECTOR_SIZE);
		if(mapStreamSize > DiskFileCD::_SECTOR_SIZE)
		{			                        // need to read more sectors
			_levelFile->SeekForward((asmaptocEntry._offsetInDiskFile)+DiskFileCD::_SECTOR_SIZE);
			_levelFile->ReadBytes(mapMem+DiskFileCD::_SECTOR_SIZE,mapStreamSize-DiskFileCD::_SECTOR_SIZE);
		}
	}
	DBSTREAM2( cflow <<"Level::loadLevelData:done reading map" << std::endl; )

	binistream mapStream((void*)mapMem,mapStreamSize);

   _theAssetManager->ReadAssetMap(mapStream);
	HALScratchLmalloc.Free(mapMem,MAX_ASMP_SIZE);

	const DiskTOC::TOCEntry& tocEntry = _theAssetManager->GetTOCEntry(AssetManager::TOC_LEVEL_INDEX);
	_levelFile->SeekForward(tocEntry._offsetInDiskFile);
   DBSTREAM2(clevel << "tocentry: seek to " << tocEntry._offsetInDiskFile << std::endl; )
	_levelOnDiskMemory = new (HALLmalloc)  char[tocEntry._size];
	assert( ValidPtr( _levelOnDiskMemory ) );
	assert(tocEntry._size % DiskFileCD::_SECTOR_SIZE == 0);
	_levelFile->ReadBytes( _levelOnDiskMemory, tocEntry._size);

	struct LVLCHUNKHDR
	{
		int32 lvlTag;
		int32 lvlSize;
	};

	ASSERTIONS( LVLCHUNKHDR& lvlhdr = *((LVLCHUNKHDR*)_levelOnDiskMemory); )
	assert(lvlhdr.lvlTag == IFFTAG('L','V','L','\0'));
	// ok, now get the offset of every chunk left in the file

//	_theAssetManager->LoadTOC(*_levelFile,roomCount);
	_levelData = ( _LevelOnDisk * )(((char*)_levelOnDiskMemory)+sizeof(LVLCHUNKHDR));

	ASSERTIONS( int roomCount = _levelData->roomCount; )
	RangeCheckExclusive(0,roomCount,200);		// 200 is arbitrary
	DBSTREAM1( cflow <<"Level::loadLevelData:done" << std::endl; )
}

//==============================================================================

void
Level::WriteSystemMailbox( int boxnum, Scalar value )
{
    RangeCheck(EMAILBOX_GLOBAL_SYSTEM_START,boxnum,EMAILBOX_GLOBAL_SYSTEM_MAX);

	DBSTREAM2( cmailbox << "setting mailbox " << boxnum << " to " << value << std::endl; )

    // KTS wire up system registers here
    switch(boxnum)
    {
        case EMAILBOX_END_OF_LEVEL:				// 0= not done, 1 = done
        {
            if ( value.AsBool() )
            {
                _done = true;
                break;
            }
        }

        case EMAILBOX_CAMSHOT:
            _camShotMailBox = value.WholePart();
            break;

        case EMAILBOX_CAMROLL:
            _camRollMailBox = value.WholePart();
            break;

        case EMAILBOX_MIDI:
        {
            // TODO: Actually means something once we have .sep files
#if defined( SOUND ) && defined( __PSX__ )
            int nSong = value.WholePart();
            extern short seq;	// midi music
            if ( nSong )
                SsSeqPlay( seq, SSPLAY_PLAY, SSPLAY_INFINITY );
            else
                SsSeqStop( seq );
#endif
            break;
        }

        default:
            AssertMsg( 0, "Attempted to write to mailbox " << boxnum );			// invalid system mailbox
            break;
    }
    return;
}

//==============================================================================

Scalar
Level::ReadSystemMailbox( int boxnum ) const
{
    RangeCheck(EMAILBOX_GLOBAL_SYSTEM_START,boxnum,EMAILBOX_GLOBAL_SYSTEM_MAX);

    switch(boxnum)
    {
        case EMAILBOX_CAMSHOT:
            return Scalar( _camShotMailBox, 0 );

//			case EMAILBOX_STUPID_PF_CAMSHOT:
//				return(_stupidPFCamShotMailBox);

        case EMAILBOX_CAMROLL:
            return Scalar( _camRollMailBox, 0 );

        case EMAILBOX_TIME:
            return levelClock();

        case EMAILBOX_DELTA_TIME:
            return LevelClock().Delta();

    case EMAILBOX_HARDWARE_JOYSTICK1:
        assert( ValidPtr( _hardwareInput1 ) );
        return Scalar( _hardwareInput1->arePressed(), 0 );

    case EMAILBOX_HARDWARE_JOYSTICK1_RAW:
        assert( ValidPtr( _hardwareInput1 ) );
        return Scalar( _hardwareInput1->arePressedRaw(), 0 );

    case EMAILBOX_HARDWARE_JOYSTICK1_RAW_JUSTPRESSED:
        assert( ValidPtr( _hardwareInput1 ) );
        return Scalar( _hardwareInput1->justPressedRaw(), 0 );

    case EMAILBOX_HARDWARE_JOYSTICK2:
        assert( ValidPtr( _hardwareInput2 ) );
        return Scalar( _hardwareInput2->arePressed(), 0 );

    case EMAILBOX_HARDWARE_JOYSTICK2_RAW:
        assert( ValidPtr( _hardwareInput2 ) );
        return Scalar( _hardwareInput2->arePressedRaw(), 0 );

    case EMAILBOX_HARDWARE_JOYSTICK2_RAW_JUSTPRESSED:
        assert( ValidPtr( _hardwareInput2 ) );
        return Scalar( _hardwareInput2->justPressedRaw(), 0 );

    case EMAILBOX_HARDWARE_JOYSTICK3:
        assert( ValidPtr( _hardwareInput3 ) );
        return Scalar( _hardwareInput3->arePressed(), 0 );

    case EMAILBOX_HARDWARE_JOYSTICK3_RAW:
        assert( ValidPtr( _hardwareInput3 ) );
        return Scalar( _hardwareInput3->arePressedRaw(), 0 );

    case EMAILBOX_HARDWARE_JOYSTICK3_RAW_JUSTPRESSED:
        assert( ValidPtr( _hardwareInput3 ) );
        return Scalar( _hardwareInput3->justPressedRaw(), 0 );

    case EMAILBOX_HARDWARE_JOYSTICK4:
        assert( ValidPtr( _hardwareInput4 ) );
        return Scalar( _hardwareInput4->arePressed(), 0 );

    case EMAILBOX_HARDWARE_JOYSTICK4_RAW:
        assert( ValidPtr( _hardwareInput4 ) );
        return Scalar( _hardwareInput4->arePressedRaw(), 0 );

    case EMAILBOX_HARDWARE_JOYSTICK4_RAW_JUSTPRESSED:
        assert( ValidPtr( _hardwareInput4 ) );
        return Scalar( _hardwareInput4->justPressedRaw(), 0 );

    default:
        if ( boxnum < EMAILBOX_GLOBAL_SYSTEM_MAX )
            AssertMsg( 0, *this << ":GetMailboxes().ReadMailbox(): Write-only global system mailbox " << boxnum << std::endl );
        else
            AssertMsg( 0, *this << ":GetMailboxes().ReadMailbox(): Unknown global system mailbox " << boxnum << std::endl );
        return Scalar::zero;
    }

    assert(0);
	return(Scalar::zero);
}

//=============================================================================

static INLINE _Common*
ObjectGetOADCommon( const SObjectStartupData* startupData )
{
	assert( ValidPtr( startupData ) );
	assert( ValidPtr( startupData->objectData ) );

	int32* pOADData = ObjectGetOAD( startupData );
	assert( ValidPtr( pOADData ) );

	_Actor* pActorData = (_Actor*)pOADData;
	assert( ValidPtr( pActorData ) );
   return (_Common*)startupData->commonBlock->GetBlockPtr(pActorData->commonPageOffset);
}

//============================================================================
//	This function is a wrapper for ConstructTemplateObject() (in oas/objects.c), which does
//	a collision check before constructing a template object.  If the constructed object
//	will appear inside of another object, its "Poof" object is constructed instead.
//	(If the Poof object also collides with something, *its* poof object is constructed,
//	etc, until something is constructed which won't receive a COLLISION message.)
//	If we hit the end of the "Poof chain" first, return NULL.

// automatically generated by the oas process
Actor* ConstructTemplateObject( int32 type, const SObjectStartupData* startData );


typedef int32 collisionInteraction;
extern collisionInteraction collisionInteractionTable[Actor::MAX_OBJECT_TYPES][Actor::MAX_OBJECT_TYPES];

//-----------------------------------------------------------------------------

Actor*
SafelyConstructTemplateObject(int32 objectToGenerate,
							  int32 parentObjectIndex,
							  Vector3& startPos,
							  Vector3& startVel
                       )
{
	DBSTREAM1( cflow << "Level::SafelyConstructTemplateObject()" << std::endl; )
	assert(objectToGenerate > 0);
	assert(parentObjectIndex > 0);
	SObjectStartupData* startupData = (SObjectStartupData*)theLevel->FindTemplateObjectData(objectToGenerate);
	assert( ValidPtr( startupData ) );

#pragma message ("KTS " __FILE__ ": kludge, should check what mass will be or something")
	if(startupData->objectData->type != Actor::Explode_KIND)
	{
		// Construct a ColSpace for the new object and check collisions
#if 1
        Vector3 vect1( Scalar::FromFixed32( startupData->objectData->coarse.minX ), 
                       Scalar::FromFixed32( startupData->objectData->coarse.minY ), 
                       Scalar::FromFixed32( startupData->objectData->coarse.minZ )  
                      );

        Vector3 vect2( Scalar::FromFixed32( startupData->objectData->coarse.maxX ),
                       Scalar::FromFixed32( startupData->objectData->coarse.maxY ),
                       Scalar::FromFixed32( startupData->objectData->coarse.maxZ ) 
                     );
		ColSpace templateColSpace( ColBox( 
                                           vect1,
                                           vect2
                                         )
                                 );
#else
// kts 8/4/99 for some reason gcc 2.95 doesn't like this, investigate why
		ColSpace templateColSpace( ColBox( Vector3( Scalar::FromFixed32( startupData->objectData->coarse.minX ),
								 					Scalar::FromFixed32( startupData->objectData->coarse.minY ),
													Scalar::FromFixed32( startupData->objectData->coarse.minZ ) 
                                                  ),
								           Vector3( Scalar::FromFixed32( startupData->objectData->coarse.maxX ),
													Scalar::FromFixed32( startupData->objectData->coarse.maxY ),
													Scalar::FromFixed32( startupData->objectData->coarse.maxZ ) 
                                                  ) 
                                         ) 
                                 );
#endif

		templateColSpace.Expand( startVel * theLevel->LevelClock().Delta());

      Room* containingRoom = theLevel->GetLevelRooms().FindContainingRoom(templateColSpace, startPos);
      if(containingRoom)
      {
         BaseObjectIteratorWrapper colIter = containingRoom->ListIter(ROOM_OBJECT_LIST_COLLIDE);

         while ( !colIter.Empty() )
         {
            PhysicalObject* colObject = dynamic_cast<PhysicalObject*>(&(*colIter));
            assert(ValidPtr(colObject));

            if ( collisionInteractionTable[startupData->objectData->type][colObject->kind()] )
            {
               if ( templateColSpace.CheckCollision(startPos, colObject->GetPhysicalAttributes().GetColSpace(), colObject->GetPhysicalAttributes().Position()) )
               {
                  DBSTREAM3(
                     cdebug << "Can't spawn that object, something's in the way." << std::endl;
                     cdebug << "Template ColBox:" << std::endl;
                     cdebug << "Min (" << templateColSpace.Min(startPos) << ")" << std::endl;
                     cdebug << "Max (" << templateColSpace.Max(startPos) << ")" << std::endl;
                     cdebug << "Colliding Actor: " << *colObject << std::endl << colObject->GetPhysicalAttributes() << std::endl;
                     cdebug << "  Kind = " << colObject->kind() << std::endl;
                  )
                  _Common* commonData = ObjectGetOADCommon( startupData );
                  if (commonData->Poof != -1)
                  {
   //						SObjectStartupData* poofStartupData = (SObjectStartupData*)theLevel->FindTemplateObjectData(commonData->Poof);
                     return (SafelyConstructTemplateObject( commonData->Poof, parentObjectIndex, startPos, startVel));
                  }
                  else
                     return (Actor*)NULL;
               }
            }
            ++colIter;
         }
      }
      else
      {
         AssertMsg(containingRoom,"trying to spawn an object outside of all rooms");
         return (Actor*)NULL;
      }

	}
	startupData->idxCreator = parentObjectIndex;
	Actor* retVal = ConstructTemplateObject( startupData->objectData->type, startupData );
	assert(ValidPtr(retVal));
	startupData->idxCreator = 0;				// kts insure no one else uses it
	DBSTREAM1( cflow << "Level::SafelyConstructTemplateObject():done" << std::endl; )
	return retVal;
}

//-----------------------------------------------------------------------------

Actor*
Level::ConstructTemplateObject(int32 templateObjectIndex, int32 parentObjectIndex, Vector3 position, Vector3 velocity)  // = index of template object to construct
{
	Actor* createdObject = SafelyConstructTemplateObject(templateObjectIndex, parentObjectIndex, position, velocity);
	if(createdObject)
	{
      DBSTREAM1(clevel << "object " << *createdObject << " created" << std::endl; )
		assert(ValidPtr(createdObject));

		createdObject->setCurrentPos(position);
		createdObject->setSpeed(velocity);

		// kts now bind geometry
#pragma message ("KTS " __FILE__ ": need to make a dynamic memory allocator for temporary objects")
		createdObject->BindAssets(GetAssetManager().GetAssetSlot(VideoMemory::PERMANENT_SLOT).GetSlotMemory());
	}
	return createdObject;
}

//==============================================================================

BaseObject*
Level::GetObject( int idxObject ) const
{
	RangeCheck(0,idxObject,_actors.Size());

   if(_actors[idxObject])
   {
      BaseObject* po = dynamic_cast<BaseObject*>(_actors[idxObject]);
      assert(ValidPtr(po));
      return po;
   }
   else
      return NULL;
}

//==============================================================================

AssetCallbackRoom::AssetCallbackRoom(const LevelRooms& rooms) 
: _rooms(rooms)
{
   
}

int
AssetCallbackRoom::GetSlotIndex(int roomIndex)
{
	return _rooms.GetSlotIndex(roomIndex);
}

//==============================================================================

LevelRoomCallbacks::LevelRoomCallbacks(Level* level)
{
   assert(ValidPtr(level));
   _level = level;
   Validate();
}

LevelRoomCallbacks::~LevelRoomCallbacks()
{
   Validate();
}

void 
LevelRoomCallbacks::SetPendingRemove( const BaseObject* object )
{
   Validate();
   assert(ValidPtr(object));
   object->Validate();
   _level->SetPendingRemove(object);
}


void 
LevelRoomCallbacks::_Validate() const
{
   assert(ValidPtr(_level));
}

//==============================================================================

int32
ObjectIsInWhichRoom( int32 idxObject, const _LevelOnDisk* _levelData )
{
	int nRoom = -1;

	assert(idxObject > 0);
	assert( ValidPtr( _levelData ) );

	// FIX - this is inefficient!!!  Fix level converter to output objects in room order.
	int32 * roomArray = ( int32 * )( (char * )_levelData + ( _levelData->roomsOffset ));
	assert( ValidPtr( roomArray ) );

	for( int roomnum = 0; roomnum < _levelData->roomCount; roomnum++ )
		{
		_RoomOnDisk * roomData = ( _RoomOnDisk * )( (( char * )_levelData ) + roomArray[roomnum] );
		assert( ValidPtr( roomData ) );

		// check objects in the room
		_RoomOnDiskEntry * entryArray = ( _RoomOnDiskEntry * )( (char * )roomData + sizeof( _RoomOnDisk ));
		assert( ValidPtr( entryArray ) );
		for( int entry = 0; entry < roomData->count; entry++ )
			{
			if ( entryArray[entry].object == idxObject )
				{
				nRoom = roomnum;
				break;
				}
			}
		}
	return nRoom;
}

//==============================================================================


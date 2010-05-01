//==============================================================================
// game.cc: 
// Copyright ( c ) 1994,1995,1996,1997,1998,1999,2001,2002,2003 World Foundry Group  
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
// Description: The WFGame class provides the application framework for the game.
//==============================================================================

// if defined, loads meta-script (and all levels) from cd.iff, otherwise, loads level1.iff
#define DO_CD_IFF

//#define DO_PROFILE

#include "level.hp"
#include "game.hp"
#include "oas/matte.ht"
#include "gamestrm.hp"
#include "camera.hp"
#include "oas/matte.ht"
#include "matte.hp"
#include "mailbox.hp"
#if TCL
#include <scripting/tcl.hp>
#endif
#include <gfx/vmem.hp>
#include <cpplib/libstrm.hp>

#if defined( __PSX__ ) && defined( DO_PROFILE )
#	include <profile/sampprof.hp>
#endif

#if defined( __PSX__ )
extern "C"
{
#	include "libetc.h"
};
#endif

//==============================================================================

Level* theLevel;

// Simulated actor for the "meta-script" run
//const WFGame::LevelActor WFGame::_ActorLevel;

//==============================================================================

WFGame::WFGame( const int nStartingLevel )
	:
	// Memory pool for allocating MsgPort messages
	_msgPortMemPool( MemPoolConstruct( sizeof( SMsg ), MSGPORTPOOLSIZE, HALLmalloc ) ),
	_overrideLevelNum( nStartingLevel )
{
	DBSTREAM1( cprogress << "WFGame::WFGame" << std::endl; )

	assert(ValidPtr(_msgPortMemPool));

	extern int _halWindowXPos;
	extern int _halWindowYPos;
	extern int _halWindowWidth;
	extern int _halWindowHeight;
	_display = new (HALLmalloc) Display(10,_halWindowXPos, _halWindowYPos, _halWindowWidth, _halWindowHeight, HALLmalloc
#if defined(DO_STEREOGRAM)
	,true
#endif
	);
	assert(ValidPtr(_display));

	_videoMemory = new (HALLmalloc) VideoMemory(*_display);
    assert(ValidPtr(_videoMemory));

//	_viewPort = new (HALLmalloc) ViewPort( *_display, 3001, Scalar( 320, 0 ), Scalar( 240, 0 ), HALLmalloc, 750 );
	_viewPort = new (HALLmalloc) ViewPort( *_display, 3001, Scalar( 320, 0 ), Scalar( 240, 0 ), HALLmalloc, 2000 );
	assert( ValidPtr( _viewPort ) );
	DBSTREAM3( cprogress << "WFGame::WFGame done" << std::endl; )
#if defined( DO_CD_IFF )
	DBSTREAM1( std::cout <<"Opening cd.iff" << std::endl; )
	_gameFile = ConstructDiskFile( "cd.iff", HALLmalloc );
	assert( ValidPtr( _gameFile ) );
#endif
}

//==============================================================================

WFGame::~WFGame()
{
	DBSTREAM1( cprogress << "WFGame::~WFGame" << std::endl; )

#if defined(DO_CD_IFF)
	DBSTREAM1( std::cout <<"closing cd.iff" << std::endl; )
	assert(ValidPtr(_gameFile));
	MEMORY_DELETE(HALLmalloc,_gameFile,_DiskFile);
#endif

//#ifdef DO_PROFILE
//	DBSTREAM1( cprogress << "Shutting down profiling" << std::endl; )
//	DELETE_CLASS( testProfile );
//#endif

	MemPoolDestruct( _msgPortMemPool );

	MEMORY_DELETE(HALLmalloc,_viewPort,ViewPort);
	MEMORY_DELETE(HALLmalloc,_videoMemory,VideoMemory);
	MEMORY_DELETE(HALLmalloc,_display,Display);
	DBSTREAM1( cprogress << "game destructor finished" << std::endl; )
}

//==============================================================================

struct CHUNKHDR
{
	int32 tag;
	int32 size;
};

//-----------------------------------------------------------------------------

void
WFGame::RunGameScript()				// runs the whole game, returns when game (really) over (quit to OS)
{
	DBSTREAM2( cflow << "WFGame::RunGameScript" << std::endl; )
#if defined(DO_CD_IFF)
	assert(ValidPtr(_gameFile));
	_gameFile->SeekRandom(0);
	// read game file header
	_gameTOC.LoadTOC(*_gameFile,_gameFile->FilePos());
	AssertMsg(_gameTOC.GetName() == IFFTAG('G','A','M','E'),"gametoc name = " << (void*)_gameTOC.GetName());
#define GAMEFILE_SCRIPT 0
#define GAMEFILE_LEVELSTART 1
	const DiskTOC::TOCEntry& tocScriptEntry = _gameTOC.GetTOCEntry(GAMEFILE_SCRIPT);
	_gameFile->SeekForward(tocScriptEntry._offsetInDiskFile);
	assert(tocScriptEntry._size % DiskFileCD::_SECTOR_SIZE == 0);
	char* streamBuffer = new (HALScratchLmalloc) char[tocScriptEntry._size];
	_gameFile->ReadBytes( streamBuffer, tocScriptEntry._size);
 	CHUNKHDR& chdr = *((CHUNKHDR*)streamBuffer);
	assert(chdr.tag = IFFTAG('S','H','E','L'));

	char* scriptMemory = new (HALLmalloc) char[chdr.size];
	assert(ValidPtr(scriptMemory));
	memcpy(scriptMemory,streamBuffer+sizeof(CHUNKHDR),chdr.size);
	HALScratchLmalloc.Free(streamBuffer,tocScriptEntry._size);

	const void* _pScript = (const void*)scriptMemory;
	assert( ValidPtr( _pScript ) );
#else
#error need to write a new static script
	const uint8 pMetaScript[] =
	{
		0x06,0x00,0x00,0x00,0x01,0x00,0x2A,0x01,0x03,0x00,0x00,0x00,
	 	0x01,0x00,0x2C,0x01,0x01,0x00,0x37,0x01,0x00,0x00,0x88,0x13,
		0x00,0x00,0x01,0x00,0x53,0x68,0x65,0x6C,0x6C,0x00,0x00,0x00
	}; //            ^ This is the level number
	const void* _pScript = (const void*)pMetaScript;
#endif
	
	DBSTREAM2( cflow << "WFGame::RunGameScript:construct MetaScript" << std::endl; )
	assert( ValidPtr( _pScript ) );
   GameMailboxes mailboxes(*this);
   SingleMailboxesManager mailboxesManager(mailboxes);
   ScriptInterpreter* interpreter = ScriptInterpreterFactory(mailboxesManager, HALLmalloc);
   assert(ValidPtr(interpreter));
	DBSTREAM1( cprogress << "meta script interpreter created" << std::endl; )

	char* pScript = (char*)_pScript;
	for ( ;; )
	{
		DBSTREAM2( cflow << "WFGame::RunGameScript::loop top" << std::endl; )
      //Scalar data = 
      interpreter->RunScript(pScript,0);
  		//  cerr << "EvalScript(" << szScript << ")=" << data << endl;
  		//return data;
      
		assert(_desiredLevelNum >= 0);
		assert(_desiredLevelNum < 9999);
		DBSTREAM3( cprogress << "Loading level " << _desiredLevelNum << std::endl; )
#if defined(DO_CD_IFF)

      if(_overrideLevelNum != -1)
         _desiredLevelNum  = _overrideLevelNum;

		const DiskTOC::TOCEntry& tocLevelEntry = _gameTOC.GetTOCEntry(GAMEFILE_LEVELSTART+_desiredLevelNum);
//		std::cout << "seeking to level at offset " << tocLevelEntry._offsetInDiskFile+DiskFileCD::_SECTOR_SIZE << std::endl;
		_gameFile->SeekRandom(tocLevelEntry._offsetInDiskFile+DiskFileCD::_SECTOR_SIZE);
		RunLevel(_gameFile);
#else
		char szLevelName[ _MAX_PATH ];
		sprintf( szLevelName, "level%d.iff", _desiredLevelNum );
		_DiskFile* diskFile = CreateDiskFile(szLevelName, HALLmalloc);
		assert(ValidPtr(diskFile));
		RunLevel(diskFile);
		assert(ValidPtr(diskFile));
		MEMORY_DELETE( HALLmalloc, diskFile,DiskFile);
#endif
	}
#if defined(DO_CD_IFF)
	HALLmalloc.Free((void*)pScript);
#endif
}

//==============================================================================

#if defined(__PSX__) && defined(DO_PROFILE)
extern bool bProfileMainLoop;
extern bool bProfileMemLoad;
#endif

//-----------------------------------------------------------------------------

void
WFGame::RunLevel(_DiskFile* levelFile)
{
    DBSTREAM3( cprogress << "WFGame::RunLevel (sizeof level = " << sizeof(Level) << std::endl; )
    GameMailboxes gmb(*this);
    Level* _curLevel = new (HALLmalloc) Level( levelFile, *_viewPort, *_videoMemory, &gmb );
	DBSTREAM3( cprogress << "new level at address " << _curLevel << ", sizeof " << sizeof(Level) << std::endl; )
	assert( ValidPtr(_curLevel));
	DBSTREAM3( cprogress << "WFGame::loadLevel done" << std::endl; )
#if defined(__PSX__) && defined(DO_PROFILE)
    if ( bProfileMemLoad )
		sys_exit( -1 );
#endif

	Scalar deltaTime = Scalar::zero;
	bool _bContinue = true;
	DBSTREAM1 ( cprogress << "Entering main game loop\n"; );
#if defined(__PSX__ ) && defined(DO_PROFILE)
	profileSample sampler;
	if ( bProfileMainLoop )
		sampler.start();
#endif

#if defined(DO_SLOW_STEREOGRAM)
	InitVSyncCallback(*_display);
#endif
	while ( !_curLevel->done() && _bContinue )
	{
		assert(HALScratchLmalloc.Empty());
		DBSTREAM1( cframeinfo << char(12) << std::endl << "Frame Info:" << std::endl; )
		DBSTREAM2( cflow << "Top of WFGame::update" << std::endl; )

#if defined( __PSX__ ) && defined(DO_PROFILE)
  		static int frameCount;
  		frameCount++;

  		if(bProfileMainLoop && frameCount > 20*5)
			_bContinue = false;
#endif

#if defined(DESIGNER_CHEATS)
		if ( PIGSUserAborted() )
		{
			DBSTREAM1( cwarn << "Level Aborted" << std::endl; )
			_bContinue = false;
			while(PIGSUserAborted())
				;						// wait for buttons to be released
		}
#endif

		assert( ValidPtr(_curLevel ));
		_curLevel->Validate();
		DBSTREAM2( cflow << "WFGame::update: curLevel->Update" << std::endl; )
		_curLevel->update(deltaTime);
		DBSTREAM2( cflow << "WFGame::update: render scene" << std::endl; )

		if(_curLevel->camera()->ValidView())
		{
			_display->RenderBegin();
#if defined(DO_SLOW_STEREOGRAM)
			int localOffset = vsyncCallbackInterface.orderTableOffset ^ 2;
			int secondLocalOffset = localOffset+1;
			vsyncCallbackInterface.constructLock = localOffset;
			if(localOffset == vsyncCallbackInterface.renderLock)
			{
				secondLocalOffset = localOffset;
				localOffset = secondLocalOffset+1;
			}

			vsyncCallbackInterface.constructLock = localOffset;
			_display->SetConstructionOrderTableIndex(localOffset);
			assert(localOffset != vsyncCallbackInterface.renderLock);
			if(localOffset == vsyncCallbackInterface.renderLock)
				printf(__FILE__ ":%d: renderLock falied\n",__LINE__);
			_display->ClearConstructionOrderTable();
			_curLevel->RenderScene();
			assert(localOffset != vsyncCallbackInterface.renderLock);

			while(secondLocalOffset == vsyncCallbackInterface.renderLock)
				;				        // lock waiting for renderer to catch up

			vsyncCallbackInterface.constructLock = secondLocalOffset;
			assert(secondLocalOffset != vsyncCallbackInterface.renderLock);
			if(secondLocalOffset == vsyncCallbackInterface.renderLock)
				printf(__FILE__ ":%d: renderLock falied\n",__LINE__);
			_display->SetConstructionOrderTableIndex(secondLocalOffset);
			_display->ClearConstructionOrderTable();
			_curLevel->RenderScene();
			assert(secondLocalOffset != vsyncCallbackInterface.renderLock);

			vsyncCallbackInterface.constructLock = -1;
			vsyncCallbackInterface.orderTableOffset ^= 2;
			vsyncCallbackInterface.pagesDirty[0] = 1;
			vsyncCallbackInterface.pagesDirty[1] = 1;
#else
			_curLevel->RenderScene();
#endif
			_display->RenderEnd();
		}
#if DO_ASSERTIONS
		else
			AssertMsg(_curLevel->LevelClock().Current() < SCALAR_CONSTANT(10),"No Valid View after 10 seconds");
#endif

		DBSTREAM2( cflow << "WFGame::update: page flip" << std::endl; )
#if defined(DO_SLOW_STEREOGRAM)
		deltaTime = VSyncCallBackReadVSyncCount();
#else
		deltaTime = _display->PageFlip();
#endif
#if defined(__PSX__)
//		cscreen << "deltaTime = " << deltaTime << ", frame rate = " << Scalar::one / deltaTime << std::endl;
#endif
		DBSTREAM2( cflow << "WFGame::update: done" << std::endl; )

		assert(HALScratchLmalloc.Empty());		// make sure everyone remembered to free their scratch memory
	}
	_display->PageFlip();			    // insure no pending ordertable renderings
	_display->PageFlip();
#if defined(DO_SLOW_STEREOGRAM)
	UnInitVSyncCallback();
#endif

#if defined(__PSX__ ) && defined(DO_PROFILE)
	if ( bProfileMainLoop )
	{
		sampler.stop();
		sampler.save( "mainloop.smp" );
		printf("sampler done\n");
		sys_exit(0);
	}
#endif

#pragma message ("KTS: write code to handle lives and restarting same level, etc.")
	DBSTREAM1( std::cout << ", _bContinue = " << _bContinue << ", _curLevel->done() = " << _curLevel->done() << std::endl; )

	MEMORY_DELETE(HALLmalloc,_curLevel,Level);
	_curLevel = NULL;
}

//==============================================================================

Scalar
WFGame::ReadSystemMailbox( int boxnum ) const
{
    RangeCheck(EMAILBOX_PERSISTENT_START,boxnum,EMAILBOX_PERSISTENT_MAX);

	switch(boxnum)
	{
		case EMAILBOX_LEVEL_TO_RUN:
			return Scalar( _desiredLevelNum, 0 );

		default:
			assert(0);
	};

	assert(0);
	return Scalar::zero;
}

//=============================================================================

void
WFGame::WriteSystemMailbox( int boxnum, Scalar value )
{
    RangeCheck(EMAILBOX_PERSISTENT_SYSTEM_START,boxnum,EMAILBOX_PERSISTENT_SYSTEM_MAX);

    switch(boxnum)
    {
        case EMAILBOX_LEVEL_TO_RUN:
            _desiredLevelNum = value.WholePart();
            DBSTREAM1( cscripting << "set desired level to " << _desiredLevelNum << std::endl; )
            break;
        default:
            AssertMsg(0, "WFGame::WriteSystemMailbox( " << boxnum << ", " << value << " )" );
    }
}

//=============================================================================

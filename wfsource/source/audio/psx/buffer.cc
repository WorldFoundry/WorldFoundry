// audio/psx/buffer.cc

#include <audio/buffer.hp>

SoundBuffer::SoundBuffer( binistream& binis ) :
	_binis( binis )
{
}


SoundBuffer::~SoundBuffer()
{
}


void
SoundBuffer::play() const
{
}


#if 0

// old code moved here from actor.cc
// still works -- just needs to be fitted to class

#if defined( __PSX__ ) && defined( SOUND )
struct VoiceInfo
{
	bool bAllocated;
	int32 priority;
};
extern VoiceInfo tblAllocatedVoice[];

const MIN_VOICE = 16;
const MAX_VOICE = 24;

int
findVoice( int32 priority )
{
	DBSTREAM1( cdebug << "checking for unallocated voice or with priority less than " << priority << endl; )

	{ // bridge until I replace the sound/MIDI subsystems
	char status[ MAX_VOICE ];
	SpuGetAllKeysStatus( status );
	for ( int i=MIN_VOICE; i<MAX_VOICE; ++i )
	{
		tblAllocatedVoice[ i ].bAllocated = status[ i ];
		DBSTREAM2( cscripting << "status[ " << i << " ] = " <<
			int32( status[ i ] ) << "  priority = " <<
			tblAllocatedVoice[ i ].priority << endl; )
	}
	}

	for ( int i=MIN_VOICE; i<MAX_VOICE; ++i )
	{
		if ( !tblAllocatedVoice[ i ].bAllocated )
		{
			tblAllocatedVoice[ i ].bAllocated = true;
			tblAllocatedVoice[ i ].priority = priority;
			return i;
		}
	}

	// couldn't find an unallocated one; search for one with a smaller priority
	static VoiceInfo top = { false, INT_MAX };
	VoiceInfo* minPriority = &top;
	for ( int i=MIN_VOICE; i<MAX_VOICE; ++i )
	{
		//DBSTREAM2( printf( "priority=%d  priority[%d]=%d\n", priority, i, tblAllocatedVoice[i].priority ); )
		if ( tblAllocatedVoice[ i ].priority < priority )
		{
			//DBSTREAM3( printf( "minPriority=%d  priority[%d]=%d\n", minPriority->priority, i, tblAllocatedVoice[i].priority ); )
			if ( tblAllocatedVoice[ i ].priority < minPriority->priority )
			{
				//DBSTREAM3( printf( "minPriority = %d\n", i ); )
				minPriority = &tblAllocatedVoice[ i ];
			}
		}
	}

	if ( minPriority == &top )
		return -1;
	else
	{
		assert( minPriority->bAllocated );	// should already be allocated; don't bother rewriting same data
		minPriority->priority = priority;
		return minPriority - tblAllocatedVoice;
	}
}



#pragma message( "Need to pass in camera parameter for 3D effects" )
short
Actor::PlaySoundEffect( int program )
{
	short voxAvailable = findVoice( 0 );
	if ( voxAvailable != -1 )
	{
		extern short vabSfx;
		extern short vabMusic;

		const Camera* pCamera = theLevel->camera();
		assert( ValidPtr( pCamera ) );

		Scalar dist = GetDistanceTo( pCamera );
		dist = dist.Abs();
		extern Scalar soundYon;

		//if ( dist <= soundYon )
		{
			Scalar percentage = Scalar::one - ( dist / soundYon );
			short lvol = ( SCALAR_CONSTANT( 127.0 ) * percentage ).WholePart();
			short rvol = ( SCALAR_CONSTANT( 127.0 ) * percentage ).WholePart();
			lvol = rvol = 127;

			//printf( "playing sfx #%d on voice #%d  lvol=%d rvol=%d\n", program, voxAvailable, lvol, rvol );

			short error = SsUtKeyOffV( voxAvailable );
			assert( error != -1 );
			short note = 10 + ( rand() % 200 );
			//printf( "note = %d\n", note );
//			short idVoice = SsUtKeyOnV( voxAvailable, vabSfx,
			short idVoice = SsUtKeyOnV( voxAvailable, vabMusic,
                                14,
				0,	//TONENO,
				program == 0 ? 190 : 10,
				0,	//FINE,
				lvol, rvol );
			assert( idVoice != -1 );
			assert( idVoice == voxAvailable );
		}
	}

	return voxAvailable;
}


#endif
#endif

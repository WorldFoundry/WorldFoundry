//==============================================================================
// ccycle.cc
// By William B. Norris IV
// Copyright ( c ) 1997,1999,2001,2002 World Foundry Group.  
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
//============================================================================

#include "ccycle.hp"

//==============================================================================

ColourCycle::ColourCycle(Clock& clock) : _clock(clock)
{
	//_pal = NULL;
}

//==============================================================================

ColourCycle::ColourCycle( Clock& clock, int start, int end, Scalar speed )	:	
   _clock(clock)
{
	assert( start >= 0 );
	_start = start;
	assert( end >= 0 );
	_end = end;
	AssertMsg( speed != Scalar::zero, "A colour cycle with a speed of 0.0 is invalid" );
	_speed = speed;
	_timeToChange = _clock() + _speed;
	//assert( pal );
	//_pal = pal;

	_start = 13;
	_end = 0;
	_speed = SCALAR_CONSTANT(-0.3);

	if ( _speed > Scalar::zero )
		assert( _start < _end );
	else
		assert( _start > _end );
}


ColourCycle::~ColourCycle()
{
}


#if defined(RENDERER_BRENDER)
void
ColourCycle::ApplyTime( Scalar /*time*/, VisualPixelmap* pixmap )
{
	assert( theLevel );
	assert( pixmap );

	// TODO: optimize cycle if a lot of loops
//	while ( theLevel->getWallClock() <= _timeToChange )
	{
		Cycle( pixmap );
//		_timeToChange += _speed;
	}
}
#endif

#if defined(RENDERER_BRENDER)
void
ColourCycle::Cycle( VisualPixelmap* pal )
{
	static int _loop = 0;

#if 0
	{
	assert( pal );
	VisualPixelmap* _texture_buffer = pal;

	char szFilename[ _MAX_PATH ];
	sprintf( szFilename, "cc%03d.tga", _loop++ );

	DBSTREAM1( cprogress << "Encoding the texture buffer as \"" << szFilename << "\"" << std::endl; )
	ValidatePtr( _texture_buffer );
	binstd::ostream binos( szFilename );
	AssertMsg( binos.good(), "Could not open \"" << szFilename << "\" for writing" );
	_texture_buffer->Encode( binos, VisualPixelmap::TGA );
	}
#endif

	assert( pal );

	int16 _y = 0;

//	pal = colourCycleVRAM;
//	_y = 240*2;

	// calculate at startup
	int16 dx;
	int16 sx;
	int16 nColours;
	int16 patch;
	int16 save;

	assert( _speed != Scalar::zero );
	if ( _speed > Scalar::zero )
	{
		assert( _start < _end );
		sx = _start;
		dx = _start + 1;
		nColours = _end - _start;
		patch = sx;
		save = sx + nColours;
	}
	else
	{
		assert( _start > _end );
		dx = _end;
		sx = _end + 1;
		nColours = _start - _end;
		patch = dx + nColours;
		save = dx;
	}

	DBSTREAM1( cprogress << "sx=" << sx << " dx=" << dx << " nColours=" << nColours << std::endl; )

	uint16 firstColour = XBrPixelmapPixelGet( pal->BrPixelmapPtr(), save, _y );
	pal->CopyRectangle( dx, _y, *pal, sx, _y, nColours, 1 );
	pal->Plot( patch, _y, firstColour );

#if 0	//( DESIGNER_CHEATS ) && defined( WRITER )
	assert( theLevel );
	char szFilename[ _MAX_PATH ];
	sprintf( szFilename, "cc%03d.tga", _loop++ );
	theLevel->saveTextureBuffer( szFilename );
#endif

	if ( _loop == nColours )
		sys_exit( 0 );
}

#endif			//defined(RENDERER_BRENDER)

//==============================================================================
//  path.cc
//  Copyright (c) 1997,1999,2000,2002,2003 World Foundry Group.
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
//
//  Description: The Path class encapsulates a stream of position and rotation
//				 values, parameterized by time.  Note that a Path doesn't know
//				 anything about how you choose to move along it (which direction,
//				 what to do at the end, etc.):  These are up to the PathHandler
//				 assigned to your Actor (see MovePath.cc).
//
//  Original Author: Ann Mei Chang
//  Fixed by:		William B. Norris IV (no original code remains)
//	Idealogical purification by: Phil Torre (no original concepts remain)
//
//==============================================================================

#define _PATH_CC

#include "path.hp"
#include <anim/channel.hp>

//==============================================================================

extern Channel* ConstructChannelObject( _ChannelOnDisk*, Memory& memory );	// defined in source/anim/channel.cc

Path::Path( const _PathOnDisk* pathData, Memory& newMemory, const _LevelOnDisk * levelData) : _memory(newMemory)
{
	assert( pathData );

	assert(ValidPtr(levelData));
	assert(ValidPtr((char*)levelData + levelData->channelsOffset));
	int32* channelArray = ( int32 * )( (char * )levelData + ( levelData->channelsOffset ));
	assert(ValidPtr(channelArray));

	DBSTREAM4( cstats << "pathData " << pathData << std::endl; )

	// Set follow object position and rotation (zero if absolute path)
	_base.SetX( Scalar( Scalar::FromFixed32(pathData->base.x) ) );
	_base.SetY( Scalar( Scalar::FromFixed32(pathData->base.y) ) );
	_base.SetZ( Scalar( Scalar::FromFixed32(pathData->base.z) ) );

	_baseRot.SetA(Angle::Revolution(Scalar(0,pathData->base.rot.a)));
	_baseRot.SetB(Angle::Revolution(Scalar(0,pathData->base.rot.b)));
	_baseRot.SetC(Angle::Revolution(Scalar(0,pathData->base.rot.c)));

	_endTime = Scalar::zero;

	_ChannelOnDisk* channelData = (_ChannelOnDisk*)( (( char * )levelData ) + channelArray[pathData->PositionXChannel] );
	assert(ValidPtr(channelData));
	_positionXChannel = ConstructChannelObject( channelData, _memory );
	if (_positionXChannel->EndTime() > _endTime)
		_endTime = _positionXChannel->EndTime();

	channelData = (_ChannelOnDisk*)( (( char * )levelData ) + channelArray[pathData->PositionYChannel] );
	assert(ValidPtr(channelData));
	_positionYChannel = ConstructChannelObject( channelData, _memory );
	if (_positionYChannel->EndTime() > _endTime)
		_endTime = _positionYChannel->EndTime();

	channelData = (_ChannelOnDisk*)( (( char * )levelData ) + channelArray[pathData->PositionZChannel] );
	assert(ValidPtr(channelData));
	_positionZChannel = ConstructChannelObject( channelData, _memory );
	if (_positionZChannel->EndTime() > _endTime)
		_endTime = _positionZChannel->EndTime();

	channelData = (_ChannelOnDisk*)( (( char * )levelData ) + channelArray[pathData->RotationAChannel] );
	assert(ValidPtr(channelData));
	_rotationAChannel = ConstructChannelObject( channelData, _memory );
	if (_rotationAChannel->EndTime() > _endTime)
		_endTime = _rotationAChannel->EndTime();

	channelData = (_ChannelOnDisk*)( (( char * )levelData ) + channelArray[pathData->RotationBChannel] );
	assert(ValidPtr(channelData));
	_rotationBChannel = ConstructChannelObject( channelData, _memory );
	if (_rotationBChannel->EndTime() > _endTime)
		_endTime = _rotationBChannel->EndTime();

	channelData = (_ChannelOnDisk*)( (( char * )levelData ) + channelArray[pathData->RotationCChannel] );
	assert(ValidPtr(channelData));
	_rotationCChannel = ConstructChannelObject( channelData, _memory );
	if (_rotationCChannel->EndTime() > _endTime)
		_endTime = _rotationCChannel->EndTime();
}

//==============================================================================

Path::~Path()
{
	assert(_positionXChannel);
	MEMORY_DELETE( _memory, _positionXChannel, Channel);

	assert(_positionYChannel);
	MEMORY_DELETE( _memory,_positionYChannel, Channel);

	assert(_positionZChannel);
	MEMORY_DELETE( _memory,_positionZChannel, Channel);

	assert(_rotationAChannel);
	MEMORY_DELETE( _memory,_rotationAChannel, Channel);

	assert(_rotationBChannel);
	MEMORY_DELETE( _memory,_rotationBChannel, Channel);

	assert(_rotationCChannel);
	MEMORY_DELETE( _memory,_rotationCChannel, Channel);
}

//==============================================================================

Vector3
Path::Position( const Scalar time )
{
	Vector3 position;

	position.SetX( Scalar::FromFixed32(_positionXChannel->Value(time)) );
	position.SetY( Scalar::FromFixed32(_positionYChannel->Value(time)) );
	position.SetZ( Scalar::FromFixed32(_positionZChannel->Value(time)) );

	position += _base;	// Add offset for relative path or zero for absolute path
	return position;
}

//=============================================================================

Euler
Path::Rotation( const Scalar time )
{
	Euler rotation;

//	rotation.SetA( Angle::Radian(_rotationAChannel->Value(time, SCALAR_CONSTANT(PI * 2.0).AsLong())) );
//	rotation.SetB( Angle::Radian(_rotationBChannel->Value(time, SCALAR_CONSTANT(PI * 2.0).AsLong())) );
//	rotation.SetC( Angle::Radian(_rotationCChannel->Value(time, SCALAR_CONSTANT(PI * 2.0).AsLong())) );


   // kts why is channel rotation data stored as Radians?
	rotation.SetA( Angle::Radian( Scalar::FromFixed32( _rotationAChannel->Value(time, long(SCALAR_ONE_LS * PI * 2.0)))) );
	rotation.SetB( Angle::Radian( Scalar::FromFixed32( _rotationBChannel->Value(time, long(SCALAR_ONE_LS * PI * 2.0)))) );
	rotation.SetC( Angle::Radian( Scalar::FromFixed32( _rotationCChannel->Value(time, long(SCALAR_ONE_LS * PI * 2.0)))) );

	rotation += _baseRot;	// Add offset for relative path or zero for absolute path
	return rotation;
}

//=============================================================================


//============================================================================
// path.cc:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
// By Kevin T. Seghetti
//============================================================================
/* Documentation:

	Abstract:
			in memory representation of level path data
	History:
			Created 06-14-95 05:42pm Kevin T. Seghetti

	Class Hierarchy:
			none

	Dependancies:
		PIGS, BRender, STDIO
	Restrictions:

	Example:
*/
//============================================================================

#include "global.hpp"
#include "path.hpp"
#include "channel.hpp"
#include "../lib/iffwrite.hp"

//============================================================================

Path::Path()
{
	assert(0);	// make sure this never gets called, and then take it out.
}

//============================================================================
// relative path
Path::Path(const Point3& basePos, const Quat& baseRot) :
	basePosition(basePos), baseRotation(baseRot)
{
	positionXChannel.SetName( "position.x" );
	positionYChannel.SetName( "position.y" );
	positionZChannel.SetName( "position.z" );
	rotationAChannel.SetName( "rotation.a" );
	rotationBChannel.SetName( "rotation.b" );
	rotationCChannel.SetName( "rotation.c" );
}

//============================================================================

Path::~Path()
{
}

//============================================================================

void
Path::AddPositionKey(TimeValue time, Point3& newPosition)
{
	positionXChannel.AddKey( ChannelEntry( time, newPosition.x - basePosition.x ) );
	positionYChannel.AddKey( ChannelEntry( time, newPosition.y - basePosition.y ) );
	positionZChannel.AddKey( ChannelEntry( time, newPosition.z - basePosition.z ) );
}

//============================================================================

#if 0
void
Path::AddRotationKey(TimeValue time, Euler& newRotation)
{
//	cout << "Adding path rotation key: (" << angles[0] << ", " << angles[1] << ", " << angles[2] << ") at time " << time / 4800.0 << endl;
	rotationAChannel.AddKey( ChannelEntry( time, newRotation.a ) );
	rotationBChannel.AddKey( ChannelEntry( time, newRotation.b ) );
	rotationCChannel.AddKey( ChannelEntry( time, newRotation.c ) );
}
#endif

//==============================================================================

void
Path::AddRotationKey(TimeValue time, Quat& newRotation)
{
	float angles[3];
	float twoPI = 2 * PI;

	QuatToEuler(newRotation - baseRotation, angles);

	// Normalize angles to be between 0 and 2PI
	if (angles[0] < 0)
		angles[0] += twoPI;
	if (angles[0] > twoPI)
		angles[0] -= twoPI;

	if (angles[1] < 0)
		angles[1] += twoPI;
	if (angles[1] > twoPI)
		angles[1] -= twoPI;

	if (angles[2] < 0)
		angles[2] += twoPI;
	if (angles[2] > twoPI)
		angles[2] -= twoPI;

	//DBSTREAM3( cdebug << "Adding path rotation key: (" << angles[0] << ", " << angles[1] << ", " << angles[2] << ") at time " << time / 4800.0 << endl; )
	rotationAChannel.AddKey( ChannelEntry( time, angles[0] ) );
	rotationBChannel.AddKey( ChannelEntry( time, angles[1] ) );
	rotationCChannel.AddKey( ChannelEntry( time, angles[2] ) );

//	rotationAChannel.AddKey( ChannelEntry( time, WF_FLOAT_TO_SCALAR(angles[0]) ) );
//	rotationBChannel.AddKey( ChannelEntry( time, WF_FLOAT_TO_SCALAR(angles[1]) ) );
//	rotationCChannel.AddKey( ChannelEntry( time, WF_FLOAT_TO_SCALAR(angles[2]) ) );

}

//============================================================================

void
Path::AddPositionOffset(const Point3& offset)
{
	assert( 0 );
//	positionXChannel.AddConstantOffset( offset.x
#if 0
	if (positionXChannel != CHANNEL_NULL)
		theLevel->GetChannel(positionXChannel).AddConstantOffset(WF_FLOAT_TO_SCALAR(offset.x));
	if (positionYChannel != CHANNEL_NULL)
		theLevel->GetChannel(positionYChannel).AddConstantOffset(WF_FLOAT_TO_SCALAR(offset.y));
	if (positionZChannel != CHANNEL_NULL)
		theLevel->GetChannel(positionZChannel).AddConstantOffset(WF_FLOAT_TO_SCALAR(offset.z));
#endif
}

//============================================================================

Point3
Path::GetPosition(TimeValue time) const
{
	Point3 returnValue(basePosition);

	assert( 0 );

#if 0
	if (positionXChannel != CHANNEL_NULL)
		returnValue.x = WF_SCALAR_TO_FLOAT(theLevel->GetChannel(positionXChannel).Read(time));
	if (positionYChannel != CHANNEL_NULL)
		returnValue.y = WF_SCALAR_TO_FLOAT(theLevel->GetChannel(positionYChannel).Read(time));
	if (positionZChannel != CHANNEL_NULL)
		returnValue.z = WF_SCALAR_TO_FLOAT(theLevel->GetChannel(positionZChannel).Read(time));
#endif

	return returnValue;
}

//============================================================================
// Printing

#if 0
ostream& operator<<(ostream& s, const Path &path)
{
	s << "Path Dump" << endl;
//	s << "X = " << path._x << "  Y = " << path._y << "  Z = " << path._z << endl;
	return s;
}
#endif

//============================================================================

_IffWriter& 
Path::_print( _IffWriter& _iff )
{
	_iff.enterChunk( ID( "PATH" ) );

#pragma message( "TODO: Implement IffWriter << Point3" )

		_iff.enterChunk( ID( "BPOS" ) );
			struct size_specifier ss = { 1, 15, 16 };
			_iff << Fixed( ss, basePosition.x );
			_iff << Fixed( ss, basePosition.y );
			_iff << Fixed( ss, basePosition.z );
			_iff << Comment( "basePosition" );
		_iff.exitChunk();

		_iff.enterChunk( ID( "BROT" ) );
			_iff << Fixed( ss, baseRotation.x );
			_iff << Fixed( ss, baseRotation.y );
			_iff << Fixed( ss, baseRotation.z );
			_iff << Fixed( ss, baseRotation.w );
			_iff << Comment( "baseRotation" );
		_iff.exitChunk();

		_iff << positionXChannel;
		_iff << positionYChannel;
		_iff << positionZChannel;

		_iff << rotationAChannel;
		_iff << rotationBChannel;
		_iff << rotationCChannel;

	_iff.exitChunk();

	return _iff;
}

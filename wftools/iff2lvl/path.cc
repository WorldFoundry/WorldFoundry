//============================================================================
// path.cc:
// Copyright(c) 1995-2001 World Foundry Group
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

#include "global.hp"
//#include <stdio.h>
//#include <string.h>

#include "hdump.hp"
//#include <stl/algo.h>

//#include "pigtool.h"
#include "path.hp"
#include <oas/levelcon.h>		// included from velocity\source
#include "level.hp"
#include "channel.hp"

//============================================================================

//extern Interface* gMaxInterface;
extern QLevel* theLevel;

//============================================================================

QPath::QPath() :
	positionXChannel(CHANNEL_NULL), positionYChannel(CHANNEL_NULL), positionZChannel(CHANNEL_NULL),
	rotationAChannel(CHANNEL_NULL), rotationBChannel(CHANNEL_NULL), rotationCChannel(CHANNEL_NULL)
{
	assert(0);	// make sure this never gets called, and then take it out.
	basePosition = Point3(0,0,0);
	baseRotation.Identity();
}

//============================================================================
// relative path
QPath::QPath(const Point3& basePos, const Quat& baseRot) :
	basePosition(basePos), baseRotation(baseRot)
{
	positionXChannel = theLevel->NewChannel();
	positionYChannel = theLevel->NewChannel();
	positionZChannel = theLevel->NewChannel();
	rotationAChannel = theLevel->NewChannel();
	rotationBChannel = theLevel->NewChannel();
	rotationCChannel = theLevel->NewChannel();
}

//============================================================================

void
QPath::SetXChannel(const Channel& channel)
{
	theLevel->GetChannel(positionXChannel) = channel;
}

//============================================================================

void
QPath::SetYChannel(const Channel& channel)
{
	theLevel->GetChannel(positionYChannel) = channel;
}

//============================================================================

void
QPath::SetZChannel(const Channel& channel)
{
	theLevel->GetChannel(positionZChannel) = channel;
}

//============================================================================

void
QPath::SetAChannel(const Channel& channel)
{
	theLevel->GetChannel(rotationAChannel) = channel;
}

//============================================================================

void
QPath::SetBChannel(const Channel& channel)
{
	theLevel->GetChannel(rotationBChannel) = channel;
}

//============================================================================

void
QPath::SetCChannel(const Channel& channel)
{
	theLevel->GetChannel(rotationCChannel) = channel;
}

//============================================================================

QPath::~QPath()
{
}

//============================================================================

//void
//QPath::Add(const Point6& newPoint, TimeValue time)
//{
//	keyFramePoints.push_back(newPoint);
//	keyFrameTimes.push_back(time);

//	int32 channelIndex

//	if (
//	theLevel->GetChannel(positionXChannel).AddKey(time, newPoint.X());
//	theLevel->GetChannel(positionYChannel).AddKey(time, newPoint.Y());
//	theLevel->GetChannel(positionZChannel).AddKey(time, newPoint.Z());
//	theLevel->GetChannel(positionYChannel).AddKey(time, newPoint.A()));

//}

//============================================================================

void
QPath::AddPositionKey(TimeValue time, Point3& newPosition)
{
	theLevel->GetChannel(positionXChannel).AddKey(ChannelEntry(time, WF_FLOAT_TO_SCALAR(newPosition.x - basePosition.x)));
	theLevel->GetChannel(positionYChannel).AddKey(ChannelEntry(time, WF_FLOAT_TO_SCALAR(newPosition.y - basePosition.y)));
	theLevel->GetChannel(positionZChannel).AddKey(ChannelEntry(time, WF_FLOAT_TO_SCALAR(newPosition.z - basePosition.z)));
}

//============================================================================

void
QPath::AddRotationKey(TimeValue time, Quat& newRotation)
{
	float angles[3];
	float twoPI = 2 * PI;

#pragma message ("KTS " __FILE__ ": finish this")
	//QuatToEuler(newRotation - baseRotation, angles);
	assert(0);

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

	DBSTREAM3( cdebug << "Adding path rotation key: (" << angles[0] << ", " << angles[1] << ", " << angles[2] << ") at time " << time / 4800.0 << endl; )
	theLevel->GetChannel(rotationAChannel).AddKey(ChannelEntry(time, WF_FLOAT_TO_SCALAR(angles[0])));
	theLevel->GetChannel(rotationBChannel).AddKey(ChannelEntry(time, WF_FLOAT_TO_SCALAR(angles[1])));
	theLevel->GetChannel(rotationCChannel).AddKey(ChannelEntry(time, WF_FLOAT_TO_SCALAR(angles[2])));
}

//============================================================================

void
QPath::AddPositionOffset(const Point3& offset)
{
	if (positionXChannel != CHANNEL_NULL)
		theLevel->GetChannel(positionXChannel).AddConstantOffset(WF_FLOAT_TO_SCALAR(offset.x));
	if (positionYChannel != CHANNEL_NULL)
		theLevel->GetChannel(positionYChannel).AddConstantOffset(WF_FLOAT_TO_SCALAR(offset.y));
	if (positionZChannel != CHANNEL_NULL)
		theLevel->GetChannel(positionZChannel).AddConstantOffset(WF_FLOAT_TO_SCALAR(offset.z));
}

//============================================================================

Point3
QPath::GetPosition(TimeValue time) const
{
	Point3 returnValue(basePosition);

	if (positionXChannel != CHANNEL_NULL)
		returnValue.x = WF_SCALAR_TO_FLOAT(theLevel->GetChannel(positionXChannel).Read(time));
	if (positionYChannel != CHANNEL_NULL)
		returnValue.y = WF_SCALAR_TO_FLOAT(theLevel->GetChannel(positionYChannel).Read(time));
	if (positionZChannel != CHANNEL_NULL)
		returnValue.z = WF_SCALAR_TO_FLOAT(theLevel->GetChannel(positionZChannel).Read(time));

	return returnValue;
}

//============================================================================

//Quat GetRotation(TimeValue time) const;

//============================================================================
// It would seem no one actually calls this.

//int32
//QPath::GetTimeIndex(TimeValue time) const
//{
//	vector<int32>::const_iterator where;
//	where = find(keyFrameTimes.begin(),keyFrameTimes.end(),time);

//	int32 index = where - keyFrameTimes.begin();

//	assert(index < Size());
//	return index;
//}

//============================================================================

//void
//QPath::SetPoint(const Point6& newPoint,int32 index)
//{
//	assert(index < Size());
//	keyFramePoints[index] = newPoint;
//	assert(keyFramePoints.size() == keyFrameTimes.size());
//}

//============================================================================

//const Point6&
//QPath::GetPoint(int32 index) const
//{
//	assert(index < Size());
//	return keyFramePoints[index];
//}

//============================================================================

//const int
//QPath::Size(void) const		// Returns number of keyframes in this path
//{
//	AssertMessageBox(keyFramePoints.size() == keyFrameTimes.size(), "Possible cause: object with hierarchical path another object doesn't have a path" );
//	return (keyFramePoints.size());
//}

//============================================================================
size_t
QPath::SizeOfOnDisk(void)
{
	return sizeof(_PathOnDisk);
}

//============================================================================

void
QPath::Save(ostream& lvl)
{
//	assert(keyFramePoints.size() == keyFrameTimes.size());
	DBSTREAM3( cdebug << "QPath::Save" << endl; )
//	DBSTREAM3( cdebug << "QPath::Save: contains <" << keyFramePoints.size() << "> paths" << endl; )

	_PathOnDisk* diskPath = (_PathOnDisk*)new char[SizeOfOnDisk()];
	assert(diskPath);

	diskPath->base.x = WF_FLOAT_TO_SCALAR(basePosition.x);
	diskPath->base.y = WF_FLOAT_TO_SCALAR(basePosition.y);
	diskPath->base.z = WF_FLOAT_TO_SCALAR(basePosition.z);
	Euler rotEuler;

	diskPath->base.rot.a = WF_FLOAT_TO_SCALAR(rotEuler.a);
	diskPath->base.rot.b = WF_FLOAT_TO_SCALAR(rotEuler.b);
	diskPath->base.rot.c = WF_FLOAT_TO_SCALAR(rotEuler.c);

	diskPath->PositionXChannel = positionXChannel;
	diskPath->PositionYChannel = positionYChannel;
	diskPath->PositionZChannel = positionZChannel;
	diskPath->RotationAChannel = rotationAChannel;
	diskPath->RotationBChannel = rotationBChannel;
	diskPath->RotationCChannel = rotationCChannel;

	lvl.write( (const char*)diskPath, SizeOfOnDisk() );
}

//============================================================================
// Printing

ostream& operator<<(ostream& s, const QPath &path)
{
	s << "QPath Dump" << endl;
//	s << "X = " << path._x << "  Y = " << path._y << "  Z = " << path._z << endl;
	return s;
}

//============================================================================

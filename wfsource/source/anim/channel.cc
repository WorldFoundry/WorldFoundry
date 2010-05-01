//==============================================================================
//	channel.cc:	
//	Copyright 1997,2000,2002 World Foundry Group.  
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
//          Impliments a system for reading channels (streams of parameterized
//				data).  This file contains the game engine side of channels;
//				see levelcon.h and LVLExp for the channel creation stuff.
//
//	Created 9/16/97 by Phil Torre
//
//==============================================================================

#include "channel.hp"
#include <iff/iffread.hp>
#include <memory/memory.hp>
#include <cpplib/libstrm.hp>

//==============================================================================

Channel::Channel()
{
}

//==============================================================================

Channel::~Channel()
{
}

//==============================================================================

LinearChannel::LinearChannel( _ChannelOnDisk* channelOnDisk )
	: _channelOnDisk( channelOnDisk )
//	: endTime( channelOnDisk->endTime ), numKeys( channelOnDisk->numKeys )
{
//	channelData = (ChannelEntry*)(channelOnDisk + 1);
}

//==============================================================================

//LinearChannel::LinearChannel( IFFChunkIter& channelChunkIter )
//{
//	AssertMsg(0, "LinearChannel: Can't construct from IFF data yet.");
//}

//==============================================================================
// Explanation of the 'period' parameter:
// If the channel client knows that this channel stores linear (aperiodic) values,
// use the default parameter of 0.  For periodic values like angles, supply the
// period in whatever units the client uses (i.e., Scalar(6.28) for angles).
// This will prevent the interpolation function from screwing up when the channel
// values cross the "singularity" where the values wrap.
// Oh, yeah:  The client is responsible for clipping "time" to be within the range 0..endTime.
int32
LinearChannel::Value(Scalar time, int32 period)
{
	Scalar endTime( Scalar::FromFixed32(_channelOnDisk->endTime ));
	int numKeys( _channelOnDisk->numKeys );
	ChannelEntry* channelData = (ChannelEntry*)(_channelOnDisk + 1);
   DBSTREAM1( canim << "LinearChannel::Value: numKeys = " << numKeys << ", endTime = " << endTime << std::endl; )

	if (time >= endTime)
		return channelData[numKeys-1].value;
	else if (time <= Scalar::zero)
		return channelData[0].value;

	assert( channelData[0].time == Scalar::zero );
	int indexBelow(-1), indexAbove(-1), index(0);
	ChannelEntry thisEntry;

#pragma message("PERFORMANCE WARNING: For long channels, a faster search is needed here!!!!")
	while ( (indexBelow == -1) || (indexAbove == -1) )
	{
		thisEntry = channelData[index];
		if (Scalar::FromFixed32(thisEntry.time) <= time)
			indexBelow = index;
		if (Scalar::FromFixed32(thisEntry.time) >= time)
			indexAbove = index;
		index++;
	}

   DBSTREAM1( canim << "LinearChannel::Value: indexBelow = " << indexBelow << ", indexAbove = " << indexAbove << std::endl; )

	int32 lowerValue(channelData[indexBelow].value), upperValue(channelData[indexAbove].value);

	// if this channel is periodic, interpolate correctly across the singularity
	if ( labs(lowerValue-upperValue) > (period >> 1) )
	{
		if (lowerValue < upperValue)
			upperValue -= period;
		else
			upperValue += period;
	}

   DBSTREAM1( canim << "LinearChannel::Value: lowerValue = " << lowerValue << ", upperValue = " << upperValue << std::endl; )

	Scalar percentageBetweenKeys = (time - Scalar::FromFixed32(channelData[indexBelow].time)) / Scalar::FromFixed32(channelData[indexAbove].time - channelData[indexBelow].time);
	Scalar partialLength = Scalar::FromFixed32(upperValue - lowerValue) * percentageBetweenKeys;

   Scalar a = SCALAR_CONSTANT(-0.1);
   AssertMsg(a.WholePart() == -1,"a = " << a << ", a.WholePart() = " << a.WholePart() );
   DBSTREAM1(canim << "a = " << a << ", a.WholePart() = " << a.WholePart() << std::endl; )
   DBSTREAM1( canim << "LinearChannel::Value: partialLength = " << partialLength << ", partialLength.WholePart = " << partialLength.WholePart() << ", partialLength.WholePart*LS = " << partialLength.WholePart()*SCALAR_ONE_LS << ", partialLength.AsUnsignedFraction()" << partialLength.AsUnsignedFraction() << std::endl; )

   int32 result = lowerValue + (partialLength.WholePart()*SCALAR_ONE_LS)+partialLength.AsUnsignedFraction();

	// clip the result to lie between 0..period
	if (result > period)
		result -= period;
	else if (result < 0)
		result += period;

   DBSTREAM1( canim << "LinearChannel::Value: percentageBetweenKeys = " << percentageBetweenKeys << ", partialLength = " << partialLength << ",result = " << result << std::endl; )

	return result;
}

//==============================================================================

Scalar
LinearChannel::EndTime()
{
	return Scalar::FromFixed32(_channelOnDisk->endTime);
}

//==============================================================================

ConstantChannel::ConstantChannel( _ChannelOnDisk* channelOnDisk )
	: _channelOnDisk( channelOnDisk )
{
//	value = ((ChannelEntry*)(channelOnDisk + 1))->value;
}

//==============================================================================

//ConstantChannel::ConstantChannel( IFFChunkIter& channelChunkIter )
//{
//	AssertMsg(0, "ConstantChannel: Can't construct from IFF data yet.");
//}

//==============================================================================

int32
ConstantChannel::Value(Scalar, int32 )
{
	return ((ChannelEntry*)(_channelOnDisk + 1))->value;
}

//==============================================================================

Scalar
ConstantChannel::EndTime()
{
	return Scalar::zero;
}

//==============================================================================

FnChannel::FnChannel()
{
}


FnChannel::FnChannel( fnChannel fn, Scalar frameRate ) :
	_frameRate( frameRate ),
	_fn( fn )
{
}


int32
FnChannel::Value( Scalar time, int32 )
{
	Scalar temp = _fn( _frameRate * time );
	return (temp.WholePart()*SCALAR_ONE_LS)+temp.AsUnsignedFraction();
}


Scalar
FnChannel::EndTime()
{
	return Scalar::zero;
}

//==============================================================================
// General purpose channel factory helper

Channel*
ConstructChannelObject( _ChannelOnDisk* channelData, Memory& memory )
{
	switch (channelData->compressorType)
	{
		case LINEAR_COMPRESSION:
			return new (memory) LinearChannel(channelData);

		case CONSTANT_COMPRESSION:
			return new (memory) ConstantChannel(channelData);

		default:
			AssertMsg(0, "Unknown channel type " << channelData->compressorType);
	}
	return NULL;	// Damn picky compiler
	// yes, too picky because it's not smart enough
}

//==============================================================================


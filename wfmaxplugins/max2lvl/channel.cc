//============================================================================
//	channel.hpp:	Encapsulates a parameterized stream of values
//
//	created 9/17/97 by Phil Torre
//
//	Copyright 1997 by Recombinant Limited.  All rights reserved.
//============================================================================

#include "channel.hpp"

#include "global.hpp"
#include <stl/algo.h>
#include <game/levelcon.h>		

extern Interface* gMaxInterface;

//============================================================================

ChannelEntry::ChannelEntry()
{
}

//============================================================================

ChannelEntry::ChannelEntry(TimeValue newTime, int32 newValue)
	: time(newTime), value(newValue)
{
}

//============================================================================

Channel::Channel()
	: compressorType(LINEAR_COMPRESSION), compressed(false)
{
}

//============================================================================

Channel::~Channel()
{
}

//============================================================================

void
Channel::AddKey(ChannelEntry newEntry)
{
	// The keys can be inserted in random order; the Channel will sort
	// itself during compression when SizeOfOnDisk() is called.

#if DEBUG > 0
	// Make sure this key has a unique time
	for (unsigned int entryIndex=0; entryIndex < _entries.size(); entryIndex++)
		assert( _entries[entryIndex].time != newEntry.time );
#endif

	_entries.push_back(newEntry);
	compressed = false;
}

//============================================================================

int32
Channel::NumKeys(void)
{
	return _entries.size();
}

//============================================================================
// Read channel value at this time (interpolated)
int32
Channel::Read(TimeValue time)
{
	// find two keys that "surround" the time value we want, and interpolate between them
//	ChannelEntry firstEntry, secondEntry;

//	for (unsigned int index=0; index < _entries.size(); index++)
//	{
//		if (_entries[index].time >

	if (time == 0)
		return (_entries[0].time);
	else
		AssertMessageBox(0, "Channel::Read() is unimplimented");

	return 0;
}

//============================================================================

size_t
Channel::SizeOfOnDisk(void)
{
	// The channel should compress itself, and then tell you what the resulting size is.
	if (!compressed)
	{
		sort( _entries.begin(), _entries.end() );

		// See if we can make this a CONSTANT channel
		int32 keyZeroValue = _entries[0].value;
		bool isConstant = true;
		for(unsigned int index=1; (index < _entries.size()) && isConstant; index++)
		{
			if ( _entries[index].value != keyZeroValue )
				isConstant = false;
		}

		if (isConstant)
			compressorType = CONSTANT_COMPRESSION;

		switch (compressorType)
		{
			case LINEAR_COMPRESSION:	// nothing to do for this...
				break;

			case CONSTANT_COMPRESSION:
//				while (_entries.size() > 1)
//					_entries.erase( _entries.end() - 1 );
				_entries.erase(_entries.begin()+1, _entries.end());
				assert(_entries.size() == 1);
				break;

			default:
				AssertMessageBox(0, "Unknown compressor type!");
				break;
		}

		compressed = true;
	}

	return ( sizeof(_ChannelOnDisk) + (_entries.size() * sizeof(_ChannelOnDiskEntry)) );
}

//============================================================================
// write this channel to disk
void
Channel::Save(ostream& lvl)
{
#define TICKS_PER_FRAME 4800.0
#pragma message("NOTE: Channel time values are being output in SECONDS rather than FRAMES!")

	assert(compressed);
	DBSTREAM3( cdebug << "Channel::Save" << endl; )
	DBSTREAM3( cdebug << "Channel contains " << _entries.size() << " keys, compression type = " << compressorType << endl; )

	_ChannelOnDisk* thisChannelOnDisk = (_ChannelOnDisk*)new char[SizeOfOnDisk()];
	assert(thisChannelOnDisk);

	thisChannelOnDisk->compressorType = compressorType;
	thisChannelOnDisk->endTime = WF_FLOAT_TO_SCALAR( (_entries.end() - 1)->time / TICKS_PER_FRAME );
	thisChannelOnDisk->numKeys = _entries.size();

	_ChannelOnDiskEntry* entryPtr = (_ChannelOnDiskEntry*)((char*)thisChannelOnDisk + sizeof(_ChannelOnDisk));

	for(unsigned int index=0; index < _entries.size(); index++)
	{
		entryPtr[index].time  = WF_FLOAT_TO_SCALAR( _entries[index].time / TICKS_PER_FRAME );
		entryPtr[index].value = _entries[index].value;
	}

	lvl.write( (char*)thisChannelOnDisk, SizeOfOnDisk() );
}

//============================================================================
// Add a constant to all keys in the channel
void
Channel::AddConstantOffset(int32 offset)
{
	for (unsigned int index=0; index < _entries.size(); index++)
		_entries[index].value += offset;
}

//============================================================================




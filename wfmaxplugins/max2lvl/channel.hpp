//============================================================================
//	channel.hpp:	Encapsulates a parameterized stream of values
//
//	created 9/17/97 by Phil Torre
//
//	Copyright 1997 by Recombinant Limited.  All rights reserved.
//============================================================================


#ifndef _CHANNEL_HP
#define _CHANNEL_HP

//============================================================================

#include "global.hpp"
#include <stl/vector.h>

class ChannelEntry
{
public:
	ChannelEntry();
	ChannelEntry(TimeValue newTime, int32 newValue);

	inline bool operator == (const ChannelEntry& other) const
		{ return ( (time == other.time) && (value == other.value) ); };
    inline bool operator < (const ChannelEntry& other) const
		{ return (time < other.time); };

	TimeValue	time;
	int32		value;
};

class Channel
{
friend class ChannelCompressor;
public:
	Channel();
	~Channel();
	void Save(ostream& lvl);				// write this channel to disk
	size_t SizeOfOnDisk(void);

	void AddKey(ChannelEntry newEntry);			// Add a new entry to this channel
	int32 Read(TimeValue time);                 // Read channel value at this time (interpolated)
	int32 NumKeys(void);                        // How many keys in this channel?
//	void ReadKey(int32 index, TimeValue& time, int32& value);	// Read a specific key
	void AddConstantOffset(int32 offset);		// Add a constant to all keys in the channel

private:
	int32 compressorType;				// which compressor (see levelcon.h)
	vector<ChannelEntry> _entries;
	bool compressed;					// true if this channel is compressed and ready to write out
};

//============================================================================
// Base class for channel compressors

class ChannelCompressor
{
public:
	ChannelCompressor();
	~ChannelCompressor();

	void AddKey(TimeValue time, int32 value);

private:
	Channel& myChannel;		// the Channel that this compressor writes to
};




#endif	// _CHANNEL_HP

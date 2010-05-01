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
//#include <stl/vector.h>
#include <../lib/stl/bstring.h>
#include <max.h>
//#include <pigsys/pigsys.h>
class _IffWriter;

class ChannelEntry
{
public:
	ChannelEntry();
	ChannelEntry(TimeValue newTime, double newValue);

	inline bool operator == (const ChannelEntry& other) const
		{ return ( (time == other.time) && (value == other.value) ); };
    inline bool operator < (const ChannelEntry& other) const
		{ return (time < other.time); };

	TimeValue	time;
	double		value;
};

class Channel
{
friend class ChannelCompressor;
public:
	Channel();
	~Channel();

	void AddKey(ChannelEntry newEntry);			// Add a new entry to this channel
	double Read(TimeValue time) const;            // Read channel value at this time (interpolated)
	int32 NumKeys() const;                       // How many keys in this channel?
	void AddConstantOffset(int32 offset);		// Add a constant to all keys in the channel

	void SetName( const string& name );
	const string& GetName() const;

	enum 	// channel compressor/decompressor types
	{
		LINEAR_COMPRESSION = 0,		// Just stored keys with linear interpolation
		CONSTANT_COMPRESSION,		// A single constant int32 for all time values
		RLE_COMPRESSION             // (Is this the same as just stored keys? Hmm...)
	};

	_IffWriter& _print( _IffWriter& _iff );

	enum
	{
		CHANNEL_NULL = -1				// index of invalid channel 
	};

private:
	int32 compressorType;				// which compressor (see levelcon.h)
	vector<ChannelEntry> _entries;
	bool compressed;					// true if this channel is compressed and ready to write out
	string _name;
};


inline _IffWriter&
operator << ( _IffWriter& iff, Channel& channel )
{
	return channel._print( iff );
}

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

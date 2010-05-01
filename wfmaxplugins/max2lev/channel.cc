//============================================================================
//	channel.hpp:	Encapsulates a parameterized stream of values
//
//	created 9/17/97 by Phil Torre
//
//	Copyright 1997 by Recombinant Limited.  All rights reserved.
//============================================================================

#include "channel.hpp"
#include "../lib/iffwrite.hp"

//============================================================================

ChannelEntry::ChannelEntry()
{
}

//============================================================================

ChannelEntry::ChannelEntry(TimeValue newTime, double newValue)
	: time(newTime), value(newValue)
{
}

//============================================================================

Channel::Channel()
	: compressorType(LINEAR_COMPRESSION), compressed(false), _name( string( "" ) )
{
}

//============================================================================

Channel::~Channel()
{
}

//============================================================================

void
Channel::SetName( const string& name )
{
	_name = name;
}


const string&
Channel::GetName() const
{
	return _name;
}


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
Channel::NumKeys() const
{
	return _entries.size();
}

//============================================================================
// Read channel value at this time (interpolated)
double
Channel::Read(TimeValue time) const
{
	// find two keys that "surround" the time value we want, and interpolate between them
//	ChannelEntry firstEntry, secondEntry;

//	for (unsigned int index=0; index < _entries.size(); index++)
//	{
//		if (_entries[index].time >

	if (time == 0)
		return (_entries[0].time);
	else
		assert( 0 );	//, "Channel::Read() is unimplimented");

	return 0;
}

//============================================================================

_IffWriter& 
Channel::_print( _IffWriter& _iff )
{
#define TICKS_PER_FRAME 4800.0
#pragma message("NOTE: Channel time values are being output in SECONDS rather than FRAMES!")

//?	assert(compressed);

	_iff.enterChunk( ID( "CHAN" ) );

		_iff.enterChunk( ID( "NAME" ) );
			_iff << GetName().c_str();
		_iff.exitChunk();

		_iff.enterChunk( ID( "TYPE" ) );
			_iff << unsigned long( compressorType );
		_iff.exitChunk();

		_iff.enterChunk( ID( "RATE" ) );
			struct size_specifier ss = { 1, 15, 16 };
			_iff << Fixed( ss, (_entries.end() - 1)->time / TICKS_PER_FRAME );
		_iff.exitChunk();

		_iff.enterChunk( ID( "DIM" ) );
			_iff << unsigned long( _entries.size() );
		_iff.exitChunk();

		_iff.enterChunk( ID( "DATA" ) );
			for ( int idx=0; idx<_entries.size(); ++idx )
			{
				_iff << Fixed( ss, _entries[ idx ].time / TICKS_PER_FRAME );
				_iff << Fixed( ss, _entries[ idx ].value );
				_iff << Comment( PrintF( "#%d", idx )() );
			}
		_iff.exitChunk();

	_iff.exitChunk();

	return _iff;
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

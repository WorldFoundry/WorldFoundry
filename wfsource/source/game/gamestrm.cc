//==============================================================================
// gamestrm.cc:
//==============================================================================

#define _GAMESTRM_CC
#include <streams/binstrm.hp>

//==============================================================================
                    
#if SW_DBSTREAM < 1
#error don't build this file if you don't want output streams
#endif

// ------------------------------------------------------------------------
// Standard std::ostream Set
// ------------------------------------------------------------------------

#include <cpplib/stdstrm.hp>

// ------------------------------------------------------------------------

#if defined(__WIN__) || defined(__LINUX__)
#include <cpplib/strmnull.hp>

CREATENULLSTREAM(gamestream_null);				// Create a global instance of the output
//#undef NULL_STREAM
#define NULL_STREAM gamestream_null
#endif
                
// ------------------------------------------------------------------------
// game std::ostreams
// ------------------------------------------------------------------------

#define STREAMENTRY(stream,where,initial,helptext) CREATEANDASSIGNOSTREAM(stream,where);
#define EXTERNSTREAMENTRY(stream,where,initial,helptext)
#include "gamestrm.inc"
#undef STREAMENTRY
#undef EXTERNSTREAMENTRY

// ------------------------------------------------------------------------

void
RedirectGameStream( char * str )
{
	switch( *str )
	{
#define STREAMENTRY(stream,where,initial,helptext) case initial:RedirectStream( stream, str+1 ); break;
#define EXTERNSTREAMENTRY(stream,where,initial,helptext) case initial:RedirectStream( stream, str+1 ); break;
#include "gamestrm.inc"
#undef STREAMENTRY
#undef EXTERNSTREAMENTRY
		default:
			cerror << "Game Error: Unrecognized command line switch \"" <<
				*str << "\"" << std::endl;
			break;
	}
}

//==============================================================================

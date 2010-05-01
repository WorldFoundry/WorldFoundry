//==============================================================================
// libstrm.cc:
//==============================================================================

#define _libstrm_CC

#include <cstdlib>
#include <streams/dbstrm.hp>
#include <pigsys/assert.hp>
#include <cpplib/stdstrm.hp>
#include <cpplib/libstrm.hp>
#include <pigsys/genfh.hp>

#if SW_DBSTREAM == 0
#error should not be included if DBSTREAM not set
#endif

// ------------------------------------------------------------------------

#if defined(__WIN__) || defined(__LINUX__)
#include <cpplib/strmnull.hp>

CREATENULLSTREAM(libstream_null);				// Create a global instance of the output
//#undef NULL_STREAM
#define NULL_STREAM libstream_null
#endif

// ------------------------------------------------------------------------
// Standard ostreams for libraries
// ------------------------------------------------------------------------

#define STREAMENTRY(stream,where,initial,helptext) CREATEANDASSIGNOSTREAM(stream,where);
#include "../libstrm.inc"
#undef STREAMENTRY

// ------------------------------------------------------------------------

void
RedirectLibraryStream( const char * str )
{
	switch( *str )
	{
#define STREAMENTRY(stream,where,initial,helptext) case initial:RedirectStream( stream, str+1 ); break;
#include "../libstrm.inc"
#undef STREAMENTRY
		default:
			cerror << "Game Error: Unrecognized command line switch \"" <<
				*str << "\"" << std::endl;
			break;
	}
}

//==============================================================================

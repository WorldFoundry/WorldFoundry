//==============================================================================
// stdstrm.cc:
//==============================================================================

//#include "anmswtch.hp"
#define _STDSTRM_CC

#include <cstdlib>
#include "global.hp"

#if SW_DBSTREAM == 0
#error should not be included if DBSTREAM==0
#endif

// ------------------------------------------------------------------------
// Standard ostream Set
// ------------------------------------------------------------------------

#include "stdstrm.hp"

// ------------------------------------------------------------------------
// Standard ostreams
// ------------------------------------------------------------------------

// NOTE: cnull cannot be used to initialize global data, since there is no
// guarantee it will be constructed first, must create a local null stream
// to get around this

#if !defined(__PSX__)
#		if defined( __WIN__) || defined(__LINUX__)
#			include "strmnull.hp"
			nullstream stdstream_null;		 		// must be in same file to insure proper order of construction
            CREATEANDASSIGNOSTREAM(cnull,stdstream_null);
#		else
			ostream_withassign cnull( NULL );
#		endif
#endif

CREATEANDASSIGNOSTREAM(cprogress,cout);
CREATEANDASSIGNOSTREAM(cstats,cout);
CREATEANDASSIGNOSTREAM(cdebug,cnull);
CREATEANDASSIGNOSTREAM(cwarn,cout);
CREATEANDASSIGNOSTREAM(cerror,cerr);
CREATEANDASSIGNOSTREAM(cfatal,cerr);
CREATEANDASSIGNOSTREAM(cver,cerr);
CREATEANDASSIGNOSTREAM(cuser,cnull);

// ------------------------------------------------------------------------
// Private Globals
// ------------------------------------------------------------------------

static const int gMaxStreams = 10;
static int gNumStreams = 0;
static ostream * gStreams[gMaxStreams];
static char gStreamNames[gMaxStreams][_MAX_PATH];

// ------------------------------------------------------------------------
// Function Definitions
// ------------------------------------------------------------------------

ostream *
_FindOpenStream( const char * fname )
{
	DBSTREAM1( cdebug << "_FindOpenStream( " << fname << " )"; )
	for( int i = 0; i < gNumStreams; i++ )
	{
		if( !strcmp( fname, gStreamNames[i] ) )
		{
			DBSTREAM1( cdebug << " = " << (void *)gStreams[i] << endl; )
			return gStreams[i];
		}
	}
	DBSTREAM1( cdebug << " = NULL" << endl; )
	return NULL;
}

// ------------------------------------------------------------------------

void
_RegisterStream( const char * fname, ostream * os )
{
	DBSTREAM1( cdebug << "_RegisterStream( " << fname << ", " << (void *)os << " )" << endl; )
	assert( gNumStreams < gMaxStreams );
	gStreams[gNumStreams] = os;
	assert( fname );
	assert( strlen( fname ) + 1 < _MAX_PATH );
	strcpy( gStreamNames[gNumStreams], fname );
	gNumStreams++;
}

// ------------------------------------------------------------------------

ostream&
_RedirectStream( const char* selector )
{
	switch( *selector )
	{
		case 'n':
			return( cnull );
		case 's':
			return( cout );
		case 'e':
			return( cerr );
#if defined ( __PSX__ )
		case 'c':
			return( cscreen );
		case 'u':
			return( cscreenflush );
#endif
#if DO_DEBUG_FILE_SYSTEM
		case 'f':
		 {
			const char * fname = selector + 1;
			ostream * file = _FindOpenStream( fname );
			if( !file )
			{
				file = new ofstream( fname );
				if( file && file->good() )
				{
					_RegisterStream( fname, file );
					return( *file );
				}
				cerr << "unable to open print file " << selector + 1 << 'n';
			}
			else
				return( *file );
		 }
		 break;
#endif
		default:
			cerror << "Game Error: Unrecognized stream  \"" <<
				*selector << "\"" << endl;
			break;
	}
	return( cnull );
};

// ------------------------------------------------------------------------

void
RedirectStream( ostream_withassign& stream, const char * string )
{
	stream = _RedirectStream( string );
}

// ------------------------------------------------------------------------

void
RedirectStandardStream( const char * str )
{
	switch( *str )
	{
		case 'w':
			RedirectStream( cwarn, str+1 );
			break;

		case 'e':
			RedirectStream( cerror, str+1 );
			break;

		case 'f':
			RedirectStream( cfatal, str+1 );
			break;

		case 's':
			RedirectStream( cstats, str+1 );
			break;

		case 'p':
			RedirectStream( cprogress, str+1 );
			break;

		case 'd':
			RedirectStream( cdebug, str+1 );
			break;

		case 'u':
			RedirectStream( cuser, str+1 );
			break;

		default:
			cerror << "Game Error: Unrecognized standard stream name \"" <<
				*str << "\"" << endl;
			break;
	}
}

//==============================================================================

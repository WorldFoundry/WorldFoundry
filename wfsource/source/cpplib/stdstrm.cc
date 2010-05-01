//==============================================================================
// stdstrm.cc:
//==============================================================================

//#include "anmswtch.hp"
#define _STDSTRM_CC

#include <cstdlib>
#include <streams/dbstrm.hp>
#include <pigsys/genfh.hp>

#if SW_DBSTREAM == 0
#error should not be included if DBSTREAM==0
#endif

// ------------------------------------------------------------------------
// Standard ostream Set
// ------------------------------------------------------------------------

#include <pigsys/assert.hp>
#include <cpplib/stdstrm.hp>

// ------------------------------------------------------------------------
// Standard ostreams
// ------------------------------------------------------------------------

// NOTE: cnull cannot be used to initialize global data, since there is no
// guarantee it will be constructed first, must create a local null stream
// to get around this

#if !defined(__PSX__)
#		if defined( __WIN__) || defined(__LINUX__)
#			include <cpplib/strmnull.hp>
			CREATENULLSTREAM(stdstream_null);		 		// must be in same file to insure proper order of construction
            CREATEANDASSIGNOSTREAM(cnull,stdstream_null);
#		else
			ostream_withassign cnull( NULL );
#		endif
#endif

CREATEANDASSIGNOSTREAM(cstats,std::cout);
CREATEANDASSIGNOSTREAM(cdebug,cnull);
CREATEANDASSIGNOSTREAM(cprogress,cnull);
CREATEANDASSIGNOSTREAM(cwarn,std::cout);
CREATEANDASSIGNOSTREAM(cerror,std::cerr);
CREATEANDASSIGNOSTREAM(cfatal,std::cerr);
CREATEANDASSIGNOSTREAM(cver,std::cerr);
CREATEANDASSIGNOSTREAM(cuser,cnull);

// ------------------------------------------------------------------------
// Private Globals
// ------------------------------------------------------------------------

static const int gMaxStreams = 10;
static int gNumStreams = 0;
static std::ostream * gStreams[gMaxStreams];
static char gStreamNames[gMaxStreams][_MAX_PATH];

// ------------------------------------------------------------------------
// Function Definitions
// ------------------------------------------------------------------------

std::ostream *
_FindOpenStream( const char * fname )
{
	DBSTREAM1( cdebug << "_FindOpenStream( " << fname << " )"; )
	for( int i = 0; i < gNumStreams; i++ )
	{
		if( !strcmp( fname, gStreamNames[i] ) )
		{
			DBSTREAM1( cdebug << " = " << (void *)gStreams[i] << std::endl; )
			return gStreams[i];
		}
	}
	DBSTREAM1( cdebug << " = NULL" << std::endl; )
	return NULL;
}

// ------------------------------------------------------------------------

void
_RegisterStream( const char * fname, std::ostream * os )
{
	DBSTREAM1( cdebug << "_RegisterStream( " << fname << ", " << (void *)os << " )" << std::endl; )
	assert( gNumStreams < gMaxStreams );
	gStreams[gNumStreams] = os;
	assert( fname );
	assert( strlen( fname ) + 1 < _MAX_PATH );
	strcpy( gStreamNames[gNumStreams], fname );
	gNumStreams++;
}

// ------------------------------------------------------------------------

std::ostream&
_RedirectStream( const char* selector )
{
	switch( *selector )
	{
		case 'n':
			return( cnull );
		case 's':
			return( std::cout );
		case 'e':
			return( std::cerr );
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
			std::ostream * file = _FindOpenStream( fname );
			if( !file )
			{
				file = new std::ofstream( fname );
				if( file && file->good() )
				{
					_RegisterStream( fname, file );
					return( *file );
				}
				std::cerr << "unable to open print file " << selector + 1 << 'n';
			}
			else
				return( *file );
		 }
		 break;
#endif
		default:
			cerror << "Game Error: Unrecognized stream  \"" <<
				*selector << "\"" << std::endl;
			break;
	}
	return( cnull );
};

// ------------------------------------------------------------------------

void
RedirectStream( ostream_withassign& stream, const char * string )
{
	ASSIGNOSTREAM(stream, _RedirectStream( string ));
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
				*str << "\"" << std::endl;
			break;
	}
}

//==============================================================================

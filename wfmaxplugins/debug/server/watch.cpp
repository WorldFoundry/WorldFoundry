// Watch.cpp: implementation of the Watch class.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Watch.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Watch::Watch( const char* szWatchName )
{
	assert( szWatchName );
	_szWatchName = strdup( szWatchName );
	assert( _szWatchName );

	_pWatchVal = &_val;
	_val = 0.0;
	_lastVal = 1.0;		// not _val
}


Watch::~Watch()
{
	assert( _szWatchName );
	free( (char*)_szWatchName );
}


const char*
Watch::GetName() const
{
	assert( _szWatchName );
	return _szWatchName;
}


double
Watch::eval() const
{
	return _val;
}


bool
Watch::isChanged() const
{
	return _val != _lastVal;
}


void
Watch::updateWatch()
{
	assert( isChanged() );
	_lastVal = _val;
}


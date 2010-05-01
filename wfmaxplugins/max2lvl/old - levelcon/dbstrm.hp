
#ifndef _DBSTRM_HP
#define _DBSTRM_HP

// ------------------------------------------------------------------------
// Copyright (c) 1996, PF. Magic, Inc.  All Rights Reserved.
//
// This is UNPUBLISHED PROPRIETARY SOURCE CODE of PF. Magic, Inc.;
// The contents of this file may not be disclosed to third parties, copied
// or duplicated in any form, in whole or in part, without the prior
// written permission of PF. Magic, Inc.
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
// Debug Output Stream Macro Definitions
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------

#ifndef SW_DBSTREAM
#error "Please define SW_DBSTREAM to be 0 - 5"
#endif

#if SW_DBSTREAM < 0
#error "SW_DBSTREAM is too small, range: 0 - 5"
#endif

#if SW_DBSTREAM > 5
#error "SW_DBSTREAM is too large, range: 0 - 5"
#endif

// ------------------------------------------------------------------------
// DBSTREAM1: nothing in the game loop (startup and shutdown only)
// DBSTREAM2: game modules only (no generic sub-systems)
// DBSTREAM3: all sub-systems non-looping code (construct, destruct, etc.)
// DBSTREAM4: all sub-systems including loops
// DBSTREAM5: everything

#if ( SW_DBSTREAM >= 1 )
#define DBSTREAM1( x ) x
#else
#define DBSTREAM1( x )
#endif

#if ( SW_DBSTREAM >= 2 )
#define DBSTREAM2( x ) x
#else
#define DBSTREAM2( x )
#endif

#if ( SW_DBSTREAM >= 3 )
#define DBSTREAM3( x ) x
#else
#define DBSTREAM3( x )
#endif

#if ( SW_DBSTREAM >= 4 )
#define DBSTREAM4( x ) x
#else
#define DBSTREAM4( x )
#endif

#if ( SW_DBSTREAM >= 5 )
#define DBSTREAM5( x ) x
#else
#define DBSTREAM5( x )
#endif

// ------------------------------------------------------------------------

#endif /* _DBSTRM_HP */

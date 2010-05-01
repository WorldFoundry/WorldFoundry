//==============================================================================
// stdstrm.hp: various output streams
//==============================================================================

#ifndef _STDSTREAM_HPP_
#define _STDSTREAM_HPP_

//== C++ =======================================================================
#include <iostream.h>

#ifdef __WATCOMC__
#define assignableOStream ostream
#endif

#ifdef _MSC_VER
#define assignableOStream ostream_withassign
#endif

//==============================================================================

#ifdef __BORLANDC__
#define createAssignableOStream _drostream_withassign
#endif

#ifdef __WATCOMC__
#define createAssignableOStream ostream
#endif

#ifdef _MSC_VER
#define createAssignableOStream ostream_withassign
#endif

//==============================================================================

#include <pclib/strmnull.hp>

extern nullstream cnull;  						// null stream
extern assignableOStream cprogress;			// progress reporting (% conplete, tasks completed, etc.)
extern assignableOStream cstats;			// statistics (# of lines compiled, etc)
extern assignableOStream cdebug;			// debugging information (internal information of use only to the tool programmer)

extern assignableOStream cwarn;				// warnings about data being processed (opinions can go here, i.e. excessive # or colors used)
extern assignableOStream cerror;			// errors in data being processed, i.e. too many colors used, or some sort of lossy behaviour
extern assignableOStream cfatal;			// bad data files, etc.  i.e. input file corrupt.
extern assignableOStream cuser;

// You may create your own assignable streams as follows:
// createAssignableOStream ctest(cnull);
// where ctest is the name of your stream, and cnull is the name of the stream
// to assign it to(which may be changed later with RedirectAssignableStream)

//==============================================================================
// RedirectStandardStream takes a 2 character string, the first char indicates
// which stream to redirect, and the second indicates where to send it
// here is a list of standard streams and possible outputs:
//        w=warnings (defaults to cerr)
//        e=errors (defaults to cerr)
//        f=fatal (defaults to cerr)
//        s=statistics (defaults to cnull)
//        p=progress (defaults to cnull)
//        d=debugging  (defaults to cnull)
//          n=null
//          s=standard out
//          e=standard err
//          f=file
// in a standard tool, this should be switched on '-p'
//          Example: -ppn will send the progress stream to null
//============================================================================

void
RedirectStandardStream(char* str);

//==============================================================================

void
RedirectAssignableStream(assignableOStream& stream, char* str);

//==============================================================================

#endif

//==============================================================================

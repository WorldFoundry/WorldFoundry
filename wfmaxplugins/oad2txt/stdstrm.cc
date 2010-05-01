//==============================================================================
// stdstrm.cc: various output streams
//==============================================================================

#include <fstream.h>

#include "global.hpp"
#include "stdstrm.hpp"
#include <pclib/strmnull.hp>

// note: we did it this way to ensure strmnull gets constructed before the
// standard streams

#include <pclib/strmnull.cc>

createAssignableOStream cprogress(cnull);
createAssignableOStream cstats(cnull);
createAssignableOStream cdebug(cnull);
//createAssignableOStream cwarn(clog);
ofstream fout( "c:\\levelcon.warn.txt" );
createAssignableOStream cwarn( fout.rdbuf() );
createAssignableOStream cerror( fout.rdbuf() );
createAssignableOStream cfatal( fout.rdbuf() );
createAssignableOStream cuser( fout.rdbuf() );

//==============================================================================

ostream&
_RedirectStream(char* selector)
{
	switch(*selector)
	 {
		case 'n':
			return(cnull);
		case 's':
			return(cout);
		case 'e':
			return(cerr);
		case 'f':
			ofstream* file;
			file = new ofstream(selector+1);
			if(file && file->good())
				return(*file);
			cerr << "unable to open print file " << selector+1 << 'n';
	 }
	return(cnull);
};

//==============================================================================

void
RedirectAssignableStream(assignableOStream& stream, char* string)
{
	stream = _RedirectStream(string);
	assert(stream.good());
}

//==============================================================================

void
RedirectStandardStream(char* str)
{
	switch(*str)
	 {
		case 'w':
			RedirectAssignableStream(cwarn,str+1);
			break;
		case 'e':
			RedirectAssignableStream(cerror,str+1);
			break;
		case 'f':
			RedirectAssignableStream(cfatal,str+1);
			break;
		case 's':
			RedirectAssignableStream(cstats,str+1);
			break;
		case 'p':
			RedirectAssignableStream(cprogress,str+1);
			break;
		case 'd':
			RedirectAssignableStream(cdebug,str+1);
			break;
	 }
}

//============================================================================

// log.cc

#include <audiofmt/st.h>
#include <audiofmt/version.h>
#include <audiofmt/patchlvl.h>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdarg>
#include <cctype>
#include <cerrno>
#define LASTCHAR        '/'
//#include <getopt.h>

/* export flags */
int verbose = 0;	/* be noisy on stderr */
int summary = 0;	/* just print summary of information */

void
report(char *fmt, ...)
{
	va_list args;

	if (! verbose)
		return;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}


void
warn(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}

void
fail(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");

	extern void cleanup();
	cleanup();

	exit(2);
}

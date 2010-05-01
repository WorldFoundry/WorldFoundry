//=============================================================================
// general.c: simple boring routines for PIGS
//=============================================================================
// Documentation:
//=============================================================================

//	Abstract:
//		Simple string manipulation, etc
//	History:
//			Created	12-15-94 12:32pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:

//	Restrictions:

//	Example:

//=============================================================================
// dependencies

#define _gENERAL_C
#include <hal/general.h>

//#include <cstring>

//=============================================================================
// strncpy sucks, and has several dangerous behaviors, so I wrote my own

// will copy up to count characters from source to dest, and then zero terminate
//  it(count should be the total # of destination bytes availible, including
// the zero termination

void
StringCopyCount(char* dest,const char* source,size_t count)
{
	assert(count > 0);
	while(--count && ((*dest++ = *source++) != 0))
		;
	*dest = '\0';						// ensure 0 terminate
}

//=============================================================================
// test suite

#if TEST_GENERAL

void
GeneralTest(void)
{
	char foo[] = "123456";
	char bar[] = "000000";

	// first test StringCopyCount()
	assert(strlen(foo) == 6);				// ensure I got the test string length right

	StringCopyCount(bar,foo,7);
	assert(strlen(bar) == 6);
	assert(!strcmp(foo,bar));

	StringCopyCount(bar,foo,4);
	assert(strlen(bar) == 3);

	StringCopyCount(bar,foo,10);
	assert(strlen(bar) == 6);
	assert(!strcmp(foo,bar));
}

#endif

//=============================================================================

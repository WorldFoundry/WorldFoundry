#include <stdio.h>
#include <stdlib.h>

void
regerror(char* s)
{
#ifdef ERRAVAIL
	error("regexp: %s", s);
#else
	fprintf(stderr, "regexp(3): %s\n", s);
//	exit(1);
/*
	note:	if the program using regexp(3) wants to handle the regexp
		errors, comment out the above and use this
*/		
	return;	  /* let using program handle errors */
#endif
	/* NOTREACHED */
}

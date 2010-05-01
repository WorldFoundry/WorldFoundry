/*
 * July 5, 1991
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained.
 * Lance Norskog And Sundry Contributors are not responsible for
 * the consequences of using this software.
 */

#include "st.h"
#include "version.h"
#include "patchlvl.h"
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdarg>
#include <cctype>
#include <cerrno>
#define LASTCHAR        '/'
//#include <getopt.h>

/*
 * util.c.
 * Incorporate Jimen Ching's fixes for real library operation: Aug 3, 1994.
 * Redo all work from scratch, unfortunately.
 * Separate out all common variables used by effects & handlers,
 * and utility routines for other main programs to use.
 */


float volume = 1.0;	/* expansion coefficient */
int dovolume = 0;

float amplitude = 1.0;	/* Largest sample so far */

int writing = 0;	/* are we writing to a file? */

/* export flags */
//int verbose = 0;	/* be noisy on stderr */
//int summary = 0;	/* just print summary of information */

#ifdef	DOS
char writebuf[BUFSIZ];	/* output write buffer */
#endif

char *myname;

int soxpreview = 0;	/* preview mode */


#if 0
strcmpcase(const char* s1, const char* s2)
{
	while(*s1 && *s2 && (tolower(*s1) == tolower(*s2)))
		s1++, s2++;
	return *s1 - *s2;
}
#endif


/*
 * Check that we have a known format suffix string.
 */
void
gettype(ft_t formp)
{
	char **list;
	int i;

	if (! formp->filetype)
fail("Must give file type for %s file, either as suffix or with -t option",
formp->filename);
	for(i = 0; formats[i].names; i++) {
		for(list = formats[i].names; *list; list++) {
			char *s1 = *list, *s2 = formp->filetype;
			if (! _stricmp(s1, s2))
				break;	/* not a match */
		}
		if (! *list)
			continue;
		/* Found it! */
		formp->h = &formats[i];
		return;
	}
	if (! _stricmp(formp->filetype, "snd")) {
		verbose = 1;
		report("File type '%s' is used to name several different formats.", formp->filetype);
		report("If the file came from a Macintosh, it is probably");
		report("a .ub file with a sample rate of 11025 (or possibly 5012 or 22050).");
		report("Use the sequence '-t .ub -r 11025 file.snd'");
		report("If it came from a PC, it's probably a Soundtool file.");
		report("Use the sequence '-t .sndt file.snd'");
		report("If it came from a NeXT, it's probably a .au file.");
		fail("Use the sequence '-t .au file.snd'\n");
	}
	fail("File type '%s' of %s file is not known!",
		formp->filetype, formp->filename);
}

/*
 * Check that we have a known effect name.
 */
void
geteffect(eff_t effp)
{
	int i;

	for(i = 0; effects[i].name; i++) {
		char *s1 = effects[i].name, *s2 = effp->name;
		while(*s1 && *s2 && (tolower(*s1) == tolower(*s2)))
			s1++, s2++;
		if (*s1 || *s2)
			continue;	/* not a match */
		/* Found it! */
		effp->h = &effects[i];
		return;
	}
	/* Guido Van Rossum fix */
	fprintf(stderr, "Known effects:");
	for (i = 0; effects[i].name; i++)
		fprintf(stderr, "\t%s\n", effects[i].name);
	fail("\nEffect '%s' is not known!", effp->name);
}


// File format routines
int
copyformat(ft_t ft, ft_t ft2)
{
	int noise = 0, i;
	double factor;

	if (ft2->info.rate == 0) {
		ft2->info.rate = ft->info.rate;
		noise = 1;
	}
	if (ft2->info.size == -1) {
		ft2->info.size = ft->info.size;
		noise = 1;
	}
	if (ft2->info.style == -1) {
		ft2->info.style = ft->info.style;
		noise = 1;
	}
	if (ft2->info.channels == -1) {
		ft2->info.channels = ft->info.channels;
		noise = 1;
	}
	if (ft2->comment == NULL) {
		ft2->comment = ft->comment;
		noise = 1;
	}
	/*
	 * copy loop info, resizing appropriately
	 * it's in samples, so # channels don't matter
	 */
	factor = (double) ft2->info.rate / (double) ft->info.rate;
	for(i = 0; i < NLOOPS; i++) {
		ft2->loops[i].start = ft->loops[i].start * factor;
		ft2->loops[i].length = ft->loops[i].length * factor;
		ft2->loops[i].count = ft->loops[i].count;
		ft2->loops[i].type = ft->loops[i].type;
	}
	/* leave SMPTE # alone since it's absolute */
	ft2->instr = ft->instr;
	return noise;
}


void
cmpformats(ft_t ft, ft_t ft2)
{
}


// check that all settings have been given
void
checkformat(ft_t ft)
{
	if (ft->info.rate == 0)
		fail("Sampling rate for %s file was not given\n", ft->filename);
	if ((ft->info.rate < 100) || (ft->info.rate > 50000))
		fail("Sampling rate %lu for %s file is bogus\n",
			ft->info.rate, ft->filename);
	if (ft->info.size == -1)
		fail("Data size was not given for %s file\nUse one of -b/-w/-l/-f/-d/-D", ft->filename);
	if (ft->info.style == -1 && ft->info.size != cbFLOAT)
		fail("Data style was not given for %s file\nUse one of -s/-u/-U/-A", ft->filename);
	/* it's so common, might as well default */
	if (ft->info.channels == -1)
		ft->info.channels = 1;
	/*	fail("Number of output channels was not given for %s file",
			ft->filename); */
}

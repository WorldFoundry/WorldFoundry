/*
 * July 5, 1991
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained.
 * Lance Norskog And Sundry Contributors are not responsible for
 * the consequences of using this software.
 */

#include <audiofmt/st.h>
#include <audiofmt/version.h>
#include <audiofmt/patchlvl.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdarg>
#include <cctype>
#include <cerrno>
#define LASTCHAR        '/'

//#include <getopt.h>
extern "C" int getopt( int nargc, char** nargv, char* ostr );
extern "C" char *optarg;
extern "C" int optind;


void init();
void doopts( int n, char* args[] );
void usage( const char* opt );
void process();

int filetype(int fd);
void checkeffect( eff_t effp );
void geteffect(eff_t effp);
int flow_effect( int e );

void statistics();


/*
 * SOX main program.
 *
 * Rewrite for new nicer option syntax.  July 13, 1991.
 * Rewrite for separate effects library.  Sep. 15, 1991.
 * Incorporate Jimen Ching's fixes for real library operation: Aug 3, 1994.
 * Rewrite for multiple effects: Aug 24, 1994.
 */

#ifdef	DOS
extern char writebuf[];
#endif

int clipped = 0;		/* Volume change clipping errors */

static long ibufl[BUFSIZ/2];	/* Left/right interleave buffers */
static long ibufr[BUFSIZ/2];
static long obufl[BUFSIZ/2];
static long obufr[BUFSIZ/2];

long volumechange();

void checkeffect( eff_t );

struct soundstream informat, outformat;

ft_t ft;

#define MAXEFF 4
struct effect eff;
struct effect efftab[MAXEFF];	/* table of left/mono channel effects */
struct effect efftabR[MAXEFF];	/* table of right channel effects */
				/* efftab[0] is the input stream */
int neffects;			/* # of effects */
char *ifile, *ofile, *itype, *otype;

int
main( int n, char* args[] )
{
	myname = args[0];
	init();

	ifile = ofile = NULL;

	/* Get input format options */
	ft = &informat;
	doopts(n, args);
	/* Get input file */
	if (optind >= n)
#ifndef	VMS
		usage("No input file?");
#else
		/* I think this is the VMS patch, but am not certain */
		fail("Input file required");
#endif
	ifile = args[optind];
	if (! strcmp(ifile, "-"))
		ft->fp = stdin;
	else if ((ft->fp = fopen(ifile, READBINARY)) == NULL)
		fail("Can't open input file '%s': %s",
			ifile, strerror(errno));
	ft->filename = ifile;
	optind++;

	/* Let -e allow no output file, just do an effect */
	if (optind < n) {
	    if (strcmp(args[optind], "-e")) {
		/* Get output format options */
		ft = &outformat;
		doopts(n, args);
		/* Get output file */
		if (optind >= n)
			usage("No output file?");
		ofile = args[optind];
		ft->filename = ofile;
		/*
		 * There are two choices here:
		 *	1) stomp the old file - normal shell "> file" behavior
		 *	2) fail if the old file already exists - csh mode
		 */
		if (! strcmp(ofile, "-"))
			ft->fp = stdout;
		else {
#ifdef	unix
		 	/*
			 * Remove old file if it's a text file, but
		 	 * preserve Unix /dev/sound files.  I'm not sure
			 * this needs to be here, but it's not hurting
			 * anything.
			 */
			if ((ft->fp = fopen(ofile, WRITEBINARY)) &&
				   (filetype(fileno(ft->fp)) == _S_IFREG)) {
				fclose(ft->fp);
				unlink(ofile);
				creat(ofile, 0666);
				ft->fp = fopen(ofile, WRITEBINARY);
			}
#else
			ft->fp = fopen(ofile, WRITEBINARY);
#endif
			if (ft->fp == NULL)
				fail("Can't open output file '%s': %s",
					ofile, strerror(errno));
#ifdef	DOS
			if (setvbuf (ft->fp,writebuf,_IOFBF,sizeof(writebuf)))
				fail("Can't set write buffer");
#endif
		}
		writing = 1;
	    }
	    optind++;
	}

	/* ??? */
/*
	if ((optind < n) && !writing && !eff.name)
		fail("Can't do an effect without an output file!");
*/

	/* Get effect name */
	if (optind < n) {
		eff.name = args[optind];
		optind++;
		geteffect(&eff);
		(*eff.h->getopts)(&eff, n - optind, &args[optind]);
	} else {
		eff.name = "null";
		geteffect(&eff);
	}

	/*
	 * If we haven't specifically set an output file
	 * don't write a file; we could be doing a summary
	 * or a format check.
	 */
/*
	if (! ofile)
		usage("Must be given an output file name");
*/
	if (! ofile)
		writing = 0;
	/* Check global arguments */
	if (volume <= 0.0)
		fail("Volume must be greater than 0.0");
#if	defined(unix) || defined(AMIGA) || defined(ARM) || defined(_WIN32)
	informat.seekable  = (filetype(_fileno(informat.fp)) == _S_IFREG);
	outformat.seekable = (filetype(_fileno(outformat.fp)) == _S_IFREG);
#else
#if	defined(DOS) || defined(__OS2__)
	informat.seekable  = 1;
	outformat.seekable = 1;
#else
	informat.seekable  = 0;
	outformat.seekable = 0;
#endif
#endif

	/* If file types have not been set with -t, set from file names. */
	if (! informat.filetype) {
		if (informat.filetype = strrchr(ifile, LASTCHAR))
			informat.filetype++;
		else
			informat.filetype = ifile;
		if (informat.filetype = strrchr(informat.filetype, '.'))
			informat.filetype++;
		else /* Default to "auto" */
			informat.filetype = "auto";
	}
	if (writing && ! outformat.filetype) {
		if (outformat.filetype = strrchr(ofile, LASTCHAR))
			outformat.filetype++;
		else
			outformat.filetype = ofile;
		if (outformat.filetype = strrchr(outformat.filetype, '.'))
			outformat.filetype++;
	}
	/* Default the input comment to the filename.
	 * The output comment will be assigned when the informat
	 * structure is copied to the outformat.
	 */
	informat.comment = informat.filename;

	process();
	statistics();

	return 0;
}


void
doopts( int n, char* args[] )
{
	int c;
	char *str;

	while ((c = getopt(n, args, "r:v:t:c:phsuUAbwlfdDxSV")) != -1) {
		switch(c) {
		case 'p':
			soxpreview++;
			break;

		case 'h':
			usage((char *)0);
			/* no return from above */

		case 't':
			if (! ft) usage("-t");
			ft->filetype = optarg;
			if (ft->filetype[0] == '.')
				ft->filetype++;
			break;

		case 'r':
			if (! ft) usage("-r");
			str = optarg;
			if ((! sscanf(str, "%lu", &ft->info.rate)) ||
					(ft->info.rate <= 0))
				fail("-r must be given a positive integer");
			break;
		case 'v':
			if (! ft) usage("-v");
			str = optarg;
			if ((! sscanf(str, "%e", &volume)) ||
					(volume <= 0))
				fail("Volume value '%s' is not a number",
					optarg);
			dovolume = 1;
			break;

		case 'c':
			if (! ft) usage("-c");
			str = optarg;
			if (! sscanf(str, "%d", &ft->info.channels))
				fail("-c must be given a number");
			break;
		case 'b':
			if (! ft) usage("-b");
			ft->info.size = cbBYTE;
			break;
		case 'w':
			if (! ft) usage("-w");
			ft->info.size = cbWORD;
			break;
		case 'l':
			if (! ft) usage("-l");
			ft->info.size = cbLONG;
			break;
		case 'f':
			if (! ft) usage("-f");
			ft->info.size = cbFLOAT;
			break;
		case 'd':
			if (! ft) usage("-d");
			ft->info.size = cbDOUBLE;
			break;
		case 'D':
			if (! ft) usage("-D");
			ft->info.size = cbIEEE;
			break;

		case 's':
			if (! ft) usage("-s");
			ft->info.style = SIGN2;
			break;
		case 'u':
			if (! ft) usage("-u");
			ft->info.style = UNSIGNED;
			break;
		case 'U':
			if (! ft) usage("-U");
			ft->info.style = ULAW;
			break;
		case 'A':
			if (! ft) usage("-A");
			ft->info.style = ALAW;
			break;

		case 'x':
			if (! ft) usage("-x");
			ft->swap = 1;
			break;

/*  stat effect does this ?
		case 'S':
			summary = 1;
			break;
*/
		case 'V':
			verbose = 1;
			break;
		case 'z':
			fprintf(stderr, "SOX Version %d, Patchlevel %d\n",
				VERSION, PATCHLEVEL);
			exit(0);
			break;
		}
	}
}


void
init()
{
	/* init files */
	informat.info.rate      = outformat.info.rate  = 0;
	informat.info.size      = outformat.info.size  = -1;
	informat.info.style     = outformat.info.style = -1;
	informat.info.channels  = outformat.info.channels = -1;
	informat.comment   = outformat.comment = NULL;
	informat.swap      = 0;
	informat.filetype  = outformat.filetype  = (char *) 0;
	informat.fp        = stdin;
	outformat.fp       = stdout;
	informat.filename  = "input";
	outformat.filename = "output";
}


// Process input file -> effect table -> output file one buffer at a time
void
process()
{
	long i, idone, odone, idonel, odonel, idoner, odoner;
	int e, f, havedata, endoffile;

	gettype(&informat);
	if (writing)
		gettype(&outformat);
	/* Read and write starters can change their formats. */
	(* informat.h->startread)(&informat);
	checkformat(&informat);
	if (dovolume)
		report("Volume factor: %f\n", volume);
	report("Input file: using sample rate %lu\n\tsize %s, style %s, %d %s",
		informat.info.rate, sizes[informat.info.size],
		styles[informat.info.style], informat.info.channels,
		(informat.info.channels > 1) ? "channels" : "channel");
	if (informat.comment)
		report("Input file: comment \"%s\"\n",
			informat.comment);
	/* need to check EFF_REPORT */
	if (writing) {
		copyformat(&informat, &outformat);
		(* outformat.h->startwrite)(&outformat);
		checkformat(&outformat);
		cmpformats(&informat, &outformat);
	report(
"Output file: using sample rate %lu\n\tsize %s, style %s, %d %s",
		outformat.info.rate, sizes[outformat.info.size],
		styles[outformat.info.style], outformat.info.channels,
		(outformat.info.channels > 1) ? "channels" : "channel");
		if (outformat.comment)
			report("Output file: comment \"%s\"\n",
				outformat.comment);
	}

	/* Very Important:
	 * Effect fabrication and start is called AFTER files open.
	 * Effect may write out data beforehand, and
	 * some formats don't know their sample rate until now.
	 */
	/* inform effect about signal information */
	eff.ininfo = informat.info;
	eff.outinfo = outformat.info;
	for(i = 0; i < 8; i++) {
	  memcpy(&eff.loops[i], &informat.loops[i], sizeof(struct loopinfo));
	}
	eff.instr = informat.instr;

	/* build efftab */
	checkeffect(&eff);

	/* now rip out null effect */
	for(e = 1; e < neffects; e++)
		if (! strcmp(efftab[e].name, "null")) {
			for(; e < neffects; e++) {
				efftab[e] = efftab[e+1];
				efftabR[e] = efftabR[e+1];
				}
			neffects--;
		}

	for(e = 1; e < neffects; e++) {
		(* efftab[e].h->start)(&efftab[e]);
		if (efftabR[e].name)
			(* efftabR[e].h->start)(&efftabR[e]);
	}
	for(e = 0; e < neffects; e++) {
		efftab[e].obuf = (long *) malloc(BUFSIZ * sizeof(long));
		if (efftabR[e].name)
		    efftabR[e].obuf = (long *) malloc(BUFSIZ * sizeof(long));
	}

	/* read chunk */
	efftab[0].olen = (*informat.h->read)(&informat, efftab[0].obuf, (long) BUFSIZ);
	efftab[0].odone = 0;

	while (efftab[0].olen > 0) {

	/* mark chain as empty */
	for(e = 1; e < neffects; e++)
		efftab[e].odone = efftab[e].olen = 0;

#ifdef later
	/* Do volume before effects or after?  idunno */
	if (dovolume && informat.info.size != cbFLOAT)
		for (i = 0; i < isamp; i++)
			ibuffer[i] = volumechange(ibuffer[i]);
#endif

	do {

	    /* run entire chain BACKWARDS: pull, don't push.*/
	    /* this is because buffering system isn't a nice queueing system */
	    for(e = neffects - 1; e > 0; e--)
		if (flow_effect(e))
			break;

	    if (writing&&(efftab[neffects-1].olen>efftab[neffects-1].odone))
		{
			(* outformat.h->write)(&outformat, efftab[neffects-1].obuf, (long) efftab[neffects-1].olen);
	        efftab[neffects-1].odone = efftab[neffects-1].olen;
	    }

	    /* if stuff still in pipeline */
	    havedata = 0;
	    for(e = 0; e < neffects - 1; e++)
		if (efftab[e].odone < efftab[e].olen) {
			havedata = 1;
			break;
		}
	} while (havedata);

	/* read chunk */
	efftab[0].olen = (*informat.h->read)(&informat,
		efftab[0].obuf, (long) BUFSIZ);
	efftab[0].odone = 0;
	}

	/* Drain the effects out first to last,
	 * pushing residue through subsequent effects */
	/* oh, what a tangled web we weave */
	if (writing) for(f = 1; f < neffects; f++) do {

	    /* keep draining this effect */
	    efftab[f].olen = BUFSIZ;
	    (* efftab[f].h->drain)(&efftab[f],efftab[f].obuf, (long*)&efftab[f].olen);
	    if (efftab[f].olen == 0)
		break;		/* out of do {} while (1) */
	    /* mark chain as empty */
	    for(e = f + 1; e < neffects; e++)
		efftab[e].odone = efftab[e].olen = 0;
	  do {

	    for(e = neffects - 1; e > f; e--)
		if (flow_effect(e))
			break;

	    if (efftab[neffects-1].olen > 0)
		(* outformat.h->write)(&outformat, efftab[neffects-1].obuf,
			(long) efftab[neffects-1].olen);

	    /* if stuff still in pipeline */
	    havedata = 0;
	    for(e = 1; e < neffects - 1; e++)
		if (efftab[e].odone < efftab[e].olen) {
			havedata = 1;
			break;
		}
	  } while (havedata);
	  /* drain() signals end by returning less than full buffer */
	  /* bad XXX */
	  if (efftab[f].olen != BUFSIZ)
		break;		/* out of do {} while (1) */

	} while (1);

#ifdef	oldstuff
		do {
		    if (dosplit) {
			odonel = sizeof(obufl) / sizeof(long);
			(* eff.h->drain)(&eff, obufl, &odonel);
			odoner = sizeof(obufr) / sizeof(long);
			(* eff.h->drain)(&eff2, obufr, &odoner);
			for(i = 0; i < odoner; i++) {
				obuf[i*2] = obufl[i];
				obuf[i*2 + 1] = obufr[i];
			}
			odone = odonel + odoner;
		    } else {
			odone = sizeof(obuf) / sizeof(long);
			(* eff.h->drain)(&eff, obuf, &odone);
		    }
		    if (odone > 0)
			(* outformat.h->write)(&outformat, obuf, (long) odone);
		} while (odone == (sizeof(obuf) / sizeof(long)));
	}
#endif
	/* Very Important:
	 * Effect stop is called BEFORE files close.
	 * Effect may write out more data after.
	 */
	for(e = 1; e < neffects; e++)
	{
		(* efftab[e].h->stop)(&efftab[e]);
		if (efftabR[e].name)
			(* efftabR[e].h->stop)(&efftabR[e]);
	}
	(* informat.h->stopread)(&informat);
	fclose(informat.fp);
	if (writing)
		(* outformat.h->stopwrite)(&outformat);
	if (writing)
		fclose(outformat.fp);
}


int
flow_effect( int e )
{
	long i, idone, odone, idonel, odonel, idoner, odoner;
	long *ibuf, *obuf;

	/* I have no input data ? */
	if (efftab[e-1].odone == efftab[e-1].olen)
		return 0;

	/* hack: if have to run secondary effect */
	if (! efftabR[e].name) {
		idone = efftab[e-1].olen - efftab[e-1].odone;
		odone = BUFSIZ;
		(* efftab[e].h->flow)(&efftab[e], &efftab[e-1].obuf[efftab[e-1].odone], efftab[e].obuf, (int*)&idone, (int*)&odone);
		efftab[e-1].odone += idone;
		efftab[e].odone = 0;
		efftab[e].olen = odone;
	} else {
		idone = efftab[e-1].olen - efftab[e-1].odone;
		odone = BUFSIZ;
		ibuf = &efftab[e-1].obuf[efftab[e-1].odone];
		for(i = 0; i < idone; i += 2) {
			ibufl[i/2] = *ibuf++;
			ibufr[i/2] = *ibuf++;
		}
		/* left */
		idonel = (idone + 1)/2;		/* odd-length logic */
		odonel = odone/2;
		(* efftab[e].h->flow)(&efftab[e], ibufl, obufl, (int*)&idonel, (int*)&odonel);
		/* right */
		idoner = idone/2;		/* odd-length logic */
		odoner = odone/2;
		(* efftabR[e].h->flow)(&efftabR[e], ibufr, obufr, (int*)&idoner, (int*)&odoner);
		obuf = efftab[e].obuf;
		for(i = 0; i < odoner; i++) {
			*obuf++ = obufl[i];
			*obuf++ = obufr[i];
		}
		efftab[e-1].odone += idonel + idoner;
		efftab[e].odone = 0;
		efftab[e].olen = odonel + odoner;
	}
	if (idone == 0)
		fail("Effect took no samples!");

	return 0;
}

#define setin(eff, effname) \
	{eff.name = effname; \
	eff.ininfo.rate = informat.info.rate; \
	eff.ininfo.channels = informat.info.channels; \
	eff.outinfo.rate = informat.info.rate; \
	eff.outinfo.channels = informat.info.channels;}

#define setout(eff, effname) \
	{eff.name = effname; \
	eff.ininfo.rate = outformat.info.rate; \
	eff.ininfo.channels = outformat.info.channels; \
	eff.outinfo.rate = outformat.info.rate; \
	eff.outinfo.channels = outformat.info.channels;}

/*
 * If no effect given, decide what it should be.
 * Smart ruleset for multiple effects in sequence.
 * 	Puts user-specified effect in right place.
 */
void
checkeffect( eff_t effp )
{
	int already = (effp->name != (char *) 0);
	int i, j, oureffect = -1;
	int needchan = 0, needrate = 0;

	if (! writing) {
		neffects = 2;
		efftab[1].name = effp->name;
		if ((informat.info.channels == 2) &&
		   (! (efftab[1].h->flags & EFF_MCHAN)))
			efftabR[1].name = effp->name;
	}

	/* if given effect does these, we don't need to add them */
	needrate = (informat.info.rate != outformat.info.rate) &&
		! (effp->h->flags & EFF_RATE);
	needchan = (informat.info.channels != outformat.info.channels) &&
		! (effp->h->flags & EFF_MCHAN);

	neffects = 1;
	/* effect #0 is the input stream */
	/* inform all effects about all relevant changes */
	for(i = 0; i < MAXEFF; i++) {
		efftab[i].name = efftabR[i].name = (char *) 0;
		/* inform effect about signal information */
		efftab[i].ininfo = informat.info;
		efftabR[i].ininfo = informat.info;
		efftab[i].outinfo = outformat.info;
		efftabR[i].outinfo = outformat.info;
		for(j = 0; j < 8; j++) {
			memcpy(&efftab[i].loops[j],
				&informat.loops[j], sizeof(struct loopinfo));
			memcpy(&efftabR[i].loops[j],
				&informat.loops[j], sizeof(struct loopinfo));
		}
		efftab[i].instr = informat.instr;
		efftabR[i].instr = informat.instr;
	}

	if (soxpreview) {
	    /* to go faster, i suppose rate could come first if downsampling */
	    if (needchan && (informat.info.channels > outformat.info.channels))
		{
	        if (needrate) {
		    neffects = 4;
		    efftab[1].name = "avg";
		    efftab[2].name = "rate";
		    setout(efftab[3], effp->name);
		} else {
		    neffects = 3;
		    efftab[1].name = "avg";
		    setout(efftab[2], effp->name);
		}
	    } else if (needchan &&
		    (informat.info.channels < outformat.info.channels)) {
	        if (needrate) {
		    neffects = 4;
		    efftab[1].name = effp->name;
		    efftab[1].outinfo.rate = informat.info.rate;
		    efftab[1].outinfo.channels = informat.info.channels;
		    efftab[2].name = "rate";
		    efftab[3].name = "avg";
		} else {
		    neffects = 3;
		    efftab[1].name = effp->name;
		    efftab[1].outinfo.channels = informat.info.channels;
		    efftab[2].name = "avg";
		}
	    } else {
	        if (needrate) {
		    neffects = 3;
		    efftab[1].name = effp->name;
		    efftab[1].outinfo.rate = informat.info.rate;
		    efftab[2].name = "rate";
		    if (informat.info.channels == 2)
			    efftabR[2].name = "rate";
		} else {
		    neffects = 2;
		    efftab[1].name = effp->name;
		}
		if ((informat.info.channels == 2) &&
		    (! (effp->h->flags & EFF_MCHAN)))
		        efftabR[1].name = effp->name;
	    }
	} else {	/* not preview mode */
	    /* [ sum to mono,] [ then resample,] then effect */
	    /* not the purest, but much faster */
	    if (needchan &&
			(informat.info.channels > outformat.info.channels)) {
	        if (needrate && (informat.info.rate != outformat.info.rate)) {
		    neffects = 4;
		    efftab[1].name = "avg";
		    efftab[2].name = effp->name;
		    efftab[2].outinfo.rate = informat.info.rate;
		    efftab[2].outinfo.channels = informat.info.channels;
		    efftab[3].name = "resample";
		} else {
		    neffects = 3;
		    efftab[1].name = "avg";
		    efftab[2].name = effp->name;
		    efftab[2].outinfo.rate = informat.info.rate;
		    efftab[2].outinfo.channels = informat.info.channels;
		}
	    } else if (needchan &&
			(informat.info.channels < outformat.info.channels)) {
	        if (needrate) {
		    neffects = 4;
		    efftab[1].name = effp->name;
		    if (! (effp->h->flags & EFF_MCHAN))
			    efftabR[1].name = effp->name;
		    efftab[1].outinfo.rate = informat.info.rate;
		    efftab[1].outinfo.channels = informat.info.channels;
		    efftab[2].name = "resample";
		    efftab[3].name = "avg";
		} else {
		    neffects = 3;
		    efftab[1].name = effp->name;
		    if (! (effp->h->flags & EFF_MCHAN))
			    efftabR[1].name = effp->name;
		    efftab[1].outinfo.channels = informat.info.channels;
		    efftab[2].name = "avg";
		}
	    } else {
	        if (needrate) {
		    neffects = 3;
		    efftab[1].name = effp->name;
		    efftab[1].outinfo.rate = informat.info.rate;
		    efftab[2].name = "resample";
		    if (informat.info.channels == 2)
			    efftabR[2].name = "resample";
		} else {
		    neffects = 2;
		    efftab[1].name = effp->name;
		}
		if ((informat.info.channels == 2) &&
		    (! (effp->h->flags & EFF_MCHAN)))
		        efftabR[1].name = effp->name;
	    }
	}

	for(i = 1; i < neffects; i++) {
		/* pointer comparison OK here */
		/* shallow copy of initialized effect data */
		/* XXX this assumes that effect_getopt() doesn't malloc() */
		if (efftab[i].name == effp->name) {
			memcpy(&efftab[i], &eff, sizeof(struct effect));
			if (efftabR[i].name)
			    memcpy(&efftabR[i], &eff, sizeof(struct effect));
		} else {
			/* set up & give default opts for added effects */
			geteffect(&efftab[i]);
			(* efftab[i].h->getopts)(&efftab[i],0,(char**)0);
			if (efftabR[i].name)
			    memcpy(&efftabR[i], &efftab[i],
				sizeof(struct effect));
		}
	}
}

/* Guido Van Rossum fix */
void
statistics()
{
	if (dovolume && clipped > 0)
		report("Volume change clipped %d samples", clipped);
}

long
volumechange(long y)
{
	double y1;

	y1 = y * volume;
	if (y1 < -2147483647.0) {
		y1 = -2147483647.0;
		clipped++;
	}
	else if (y1 > 2147483647.0) {
		y1 = 2147483647.0;
		clipped++;
	}

	return y1;
}

#if	defined(unix) || defined(AMIGA) || defined(ARM) || defined(_WIN32)
int
filetype(int fd)
{
	struct _stat st;

	_fstat(fd, &st);

	return st.st_mode & _S_IFMT;
}
#endif

char *usagestr =
"[ -V -S -h ] [ fopts ] ifile [ fopts ] ofile [ effect [ effopts ] ]\nfopts: -r rate -v volume -c channels -s/-u/-U/-A -b/-w/-l/-f/-d/-D -x\neffects and effopts: various";


void
usage( const char* opt )
{
#ifndef	DOS
	/* single-threaded machines don't really need this */
	fprintf(stderr, "%s: ", myname);
#endif
	if (opt)
		fprintf(stderr, "%s\n\n", version());
	fprintf(stderr, "Usage: %s\n", usagestr);
	if (opt)
		fprintf(stderr, "Failed at: %s\n", opt);
	exit(1);
}


/* called from util.c:fail */
void
cleanup()
{
	// Close the input file and outputfile before exiting
	if (informat.fp)
		fclose(informat.fp);
	if (outformat.fp)
	{
		fclose(outformat.fp);
		/* remove the output file because we failed, if it's ours. */
		_unlink( outformat.filename );
	}
}

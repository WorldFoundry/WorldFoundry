/*
 * July 5, 1991
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained.
 * Lance Norskog And Sundry Contributors are not responsible for
 * the consequences of using this software.
 */

#include <audiofmt/st.h>

// Sound Tools file format and effect tables.

#define FORMAT( __name__ )	\
	void __name__##startread( ft_t ); \
	long __name__##read( ft_t, long*, long ); \
	void __name__##stopread( ft_t ); \
	void __name__##startwrite( ft_t ); \
	void __name__##write( ft_t, long*, long ); \
	void __name__##stopwrite( ft_t );

// File format handlers.

char *rawnames[] = {
	"raw",
	(char *) 0
};
FORMAT( raw )

char *cdrnames[] = {
	"cdr",
	(char *) 0
};
FORMAT( cdr )

char *vocnames[] = {
	"voc",
	(char *) 0
};
FORMAT( voc )

char *aunames[] = {
	"au",
#ifdef	NeXT
	"snd",
#endif
	(char *) 0
};
FORMAT( au )

char *wvenames[] = {
      "wve",
      (char *) 0
};
FORMAT( wve )


char *aiffnames[] = {
	"aiff",
	"aif",
	(char *) 0
};
FORMAT( aiff )

char *svxnames[] = {
	"8svx",
	"iff",
	(char *) 0
};
FORMAT( svx )

char *hcomnames[] = {
	"hcom",
	(char *) 0
};
FORMAT( hcom )

char *sndtnames[] = {
	"sndt",
#if defined( DOS ) || defined( _WIN32 )
	"snd",
#endif
	(char *) 0
};
FORMAT( sndt )

char *sndrnames[] = {
	"sndr",
	(char *) 0
};
FORMAT( sndr )

char *ubnames[] = {
	"ub",
	"sou",
	"fssd",
#ifdef	MAC
	"snd",
#endif
	(char *) 0
};
FORMAT( ub )

char *sbnames[] = {
	"sb",
	(char *) 0
};
FORMAT( sb )

char *uwnames[] = {
	"uw",
	(char *) 0
};
FORMAT( uw )

char *swnames[] = {
	"sw",
	(char *) 0
};
FORMAT( sw )

char *ulnames[] = {
	"ul",
	(char *) 0
};
FORMAT( ul )


char *alnames[] = {
	"al",
	(char *) 0
};
FORMAT( al )


char *sfnames[] = {
	"sf",
	(char *) 0
};
FORMAT( sf )

char *wavnames[] = {
	"wav",
	(char *) 0
};
FORMAT( wav )

#if	defined(BLASTER) || defined(SBLAST) || defined(LINUXSOUND)
char *sbdspnames[] = {
	"sbdsp",
	(char *) 0
};
FORMAT( sbdsp )
#endif

char *smpnames[] = {
	"smp",
	(char *) 0,
};
FORMAT( smp )

char *maudnames[] = {
        "maud",
        (char *) 0,
};
FORMAT( maud )

char *autonames[] = {
	"auto",
	(char *) 0,
};
FORMAT( auto )
void autostartread(ft_t);
void autostartwrite(ft_t);

char *datnames[] = {
	"dat",
	(char *) 0
};
FORMAT( dat )

FORMAT( null )

format_t formats[] = {
	{autonames, FILE_STEREO,
		autostartread, nullread, nullstopread,	/* Guess from header */
		autostartwrite, nullwrite, nullstopwrite },	/* patched run time */
	{smpnames, FILE_STEREO | FILE_LOOPS,
		smpstartread, smpread, nullstopread,	/* SampleVision sound */
		smpstartwrite, smpwrite, smpstopwrite},	/* Turtle Beach */
	{rawnames, FILE_STEREO,
		rawstartread, rawread, nullstopread, 	/* Raw format */
		rawstartwrite, rawwrite, nullstopwrite},
	/* Raw format that does mono->stereo automatically */
/*
	{raw2names, FILE_STEREO,
		rawstartread, rawread, nothing,
		rawstartwrite, raw2write, nothing},
*/
	{cdrnames, FILE_STEREO,
		cdrstartread, cdrread, cdrstopread,  /* CD-R format */
		cdrstartwrite, cdrwrite, cdrstopwrite},
	{vocnames, FILE_STEREO,
		vocstartread, vocread, vocstopread,  /* Sound Blaster .VOC */
		vocstartwrite, vocwrite, vocstopwrite},
	{aunames, FILE_STEREO,
		austartread, auread, nullstopread, 	/* SPARC .AU w/header */
		austartwrite, auwrite, austopwrite},
	{wvenames, 0,       			/* Psion .wve */
		wvestartread, wveread, nullstopread,
		wvestartwrite, wvewrite, wvestopwrite},
	{ubnames, FILE_STEREO,
		ubstartread, rawread, nullstopread, 	/* unsigned byte raw */
		ubstartwrite, rawwrite, nullstopwrite},	/* Relies on raw */
	{sbnames, FILE_STEREO,
		sbstartread, rawread, nullstopread, 	/* signed byte raw */
		sbstartwrite, rawwrite, nullstopwrite},
	{uwnames, FILE_STEREO,
		uwstartread, rawread, nullstopread, 	/* unsigned word raw */
		uwstartwrite, rawwrite, nullstopwrite},
	{swnames, FILE_STEREO,
		swstartread, rawread, nullstopread, 	/* signed word raw */
		swstartwrite, rawwrite, nullstopwrite},
	{ulnames, FILE_STEREO,
		ulstartread, rawread, nullstopread, 	/* u-law byte raw */
		ulstartwrite, rawwrite, nullstopwrite},
	{alnames, FILE_STEREO,
		alstartread, rawread, nullstopread, 	/* a-law byte raw */
		alstartwrite, rawwrite, nullstopwrite},
	{aiffnames, FILE_STEREO,
		aiffstartread, aiffread, aiffstopread,    /* SGI/Apple AIFF */
		aiffstartwrite, aiffwrite, aiffstopwrite},
	{svxnames, FILE_STEREO,
		svxstartread, svxread, svxstopread,      /* Amiga 8SVX */
		svxstartwrite, svxwrite, svxstopwrite},
        {maudnames, FILE_STEREO,     			/* Amiga MAUD */
		maudstartread, maudread, maudstopread,
		maudstartwrite, maudwrite, maudstopwrite},
	{hcomnames, 0,
		hcomstartread, hcomread, hcomstopread, /* Mac FSSD/HCOM */
		hcomstartwrite, hcomwrite, hcomstopwrite},
	{sfnames, FILE_STEREO,
		sfstartread, rawread, nullstopread, 	/* IRCAM Sound File */
		sfstartwrite, rawwrite, nullstopwrite},	/* Relies on raw */
	{sndtnames, FILE_STEREO,
		sndtstartread, rawread, nullstopread,    /* Sndtool Sound File */
		sndtstartwrite, sndtwrite, sndtstopwrite},
	{sndrnames, FILE_STEREO,
		sndtstartread, rawread, nullstopread,    /* Sounder Sound File */
		sndrstartwrite, rawwrite, nullstopwrite},
	{wavnames, FILE_STEREO,
		wavstartread, wavread, nullstopread, 	/* Windows 3.0 .wav */
		wavstartwrite, wavwrite, wavstopwrite},
#if	defined(BLASTER) || defined(SBLAST) || defined(LINUXSOUND)
	/* 386 Unix sound blaster players.  No more of these, please! */
	{sbdspnames, FILE_STEREO,
		sbdspstartread, sbdspread, sbdspstopread, 	/* /dev/sbdsp */
		sbdspstartwrite, sbdspwrite, sbdspstopwrite},
#endif
	{datnames, 0,
		datstartread, datread, nullstopread, 	/* Text data samples */
		datstartwrite, datwrite, nullstopwrite },
	0
};

/* Effects handlers. */

#define EFFECT( __name__ )	\
	void __name__##_getopts( eff_t effp, int n, char* argv[] ); \
	void __name__##_start( eff_t effp );\
	void __name__##_flow( eff_t effp, long* ibuf, long* obuf, int* isamp, int* osamp );\
	void __name__##_drain( eff_t effp, long* obuf, long* osamp );\
	void __name__##_stop( eff_t effp );

void copy_stop();

EFFECT( copy )
EFFECT( avg )
EFFECT( pred )
EFFECT( stat )
EFFECT( vibro )
EFFECT( band )
EFFECT( lowp )
EFFECT( highp )
EFFECT( echo )
EFFECT( rate )
EFFECT( reverse )
EFFECT( map )
EFFECT( cut )
EFFECT( split )
EFFECT( pick )
EFFECT( resample )
EFFECT( mask )

EFFECT( null )

/*
 * EFF_CHAN means that the number of channels can change.
 * EFF_RATE means that the sample rate can change.
 * The first effect which can handle a data rate change, stereo->mono, etc.
 * is the default handler for that problem.
 *
 * EFF_MCHAN just means that the effect is coded for multiple channels.
 */

effect_t effects[] = {
	{"null", 0, 			/* stand-in, never gets called */
		null_getopts, null_start, null_flow, null_drain, null_stop},
	{"copy", EFF_MCHAN,
		copy_getopts, copy_start, copy_flow, null_drain, null_stop},
	{"avg", EFF_CHAN | EFF_MCHAN,
		avg_getopts, avg_start, avg_flow, null_drain, avg_stop},
	{"split", EFF_CHAN | EFF_MCHAN,
		split_getopts, split_start, split_flow, null_drain,split_stop},
	{"pick", EFF_CHAN | EFF_MCHAN,
		pick_getopts, pick_start, pick_flow, null_drain, pick_stop},
	{"pred", 0,
		pred_getopts, pred_start, pred_flow, null_drain, pred_stop},
	{"stat", EFF_MCHAN | EFF_REPORT,
		stat_getopts, stat_start, stat_flow, null_drain, stat_stop},
	{"vibro", 0,
		vibro_getopts, vibro_start, vibro_flow, null_drain, null_stop},
	{"echo", 0,
		echo_getopts, echo_start, echo_flow, echo_drain, echo_stop},
	{"band", 0,
		band_getopts, band_start, band_flow, null_drain, band_stop},
	{"lowp", 0,
		lowp_getopts, lowp_start, lowp_flow, null_drain, lowp_stop},
	{"highp", 0,
		highp_getopts, highp_start, highp_flow, null_drain,highp_stop},
	{"rate", EFF_RATE,
		rate_getopts, rate_start, rate_flow, null_drain, null_stop},
	{"resample", EFF_RATE,
		resample_getopts, resample_start, resample_flow,
		resample_drain, resample_stop},
	{"reverse", 0,
		reverse_getopts, reverse_start,
		reverse_flow, reverse_drain, reverse_stop},
	{"map", EFF_REPORT,
		map_getopts, map_start, map_flow, null_drain, null_stop},
	{"cut", EFF_MCHAN,
		cut_getopts, cut_start, cut_flow, null_drain, null_stop},
	{"mask", EFF_MCHAN,
		mask_getopts, null_start, mask_flow, null_drain, null_stop},
	0
};


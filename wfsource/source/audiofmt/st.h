/*
 * July 5, 1991
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained.
 * Lance Norskog And Sundry Contributors are not responsible for
 * the consequences of using this software.
 */

// Sound Tools sources header file.

#include <cstdlib>
#include <cstdio>
#include <cstring>

#ifdef AMIGA
#include "amiga.h"
#endif /* AMIGA */

struct soundstream;
typedef struct soundstream *ft_t;

/*
 * Handler structure for each format.
 */

typedef struct format {
	char	**names;	/* file type names */
	int	flags;		/* details about file type */
	void	(*startread)(ft_t);
	long	(*read)( ft_t ft, long* buf, long len );
	void	(*stopread)( ft_t );
	void	(*startwrite)(ft_t);
	void	(*write)( ft_t ft, long* buf, long len );
	void	(*stopwrite)( ft_t );
} format_t;

extern format_t formats[];

/* Signal parameters */

struct  signalinfo {
	long		rate;		/* sampling rate */
	int		size;		/* word length of data */
	int		style;		/* format of sample numbers */
	int		channels;	/* number of sound channels */
};

/* Loop parameters */

struct  loopinfo {
	int		start;		/* first sample */
	int		length;		/* length */
	int		count;		/* number of repeats, 0=forever */
	int		type;		/* 0=no, 1=forward, 2=forward/back */
};

/* Instrument parameters */

/* vague attempt at generic information for sampler-specific info */

struct  instrinfo {
	char 		MIDInote;	/* for unity pitch playback */
	char		MIDIlow, MIDIhi;/* MIDI pitch-bend range */
	char		loopmode;	/* semantics of loop data */
	char		nloops;		/* number of active loops */
	unsigned char	smpte[4];	/* SMPTE offset (hour:min:sec:frame) */
					/* this is a film audio thing */
};


#define MIDI_UNITY 60		/* MIDI note number to play sample at unity */

/* Loop modes */
#define LOOP_NONE          0
#define LOOP_8             1	/* 8 loops: don't know ?? */
#define LOOP_SUSTAIN_DECAY 2	/* AIFF style: one sustain & one decay loop */

/* Pipe parameters */

struct	pipeinfo {
	FILE	*pout;			/* Output file */
	FILE	*pin;			/* Input file */
};

/*
 *  Format information for input and output files.
 */

#define	PRIVSIZE	100

#define NLOOPS		8

struct soundstream {
	struct	signalinfo info;	/* signal specifications */
	struct  instrinfo instr;	/* instrument specification */
	struct  loopinfo loops[NLOOPS];	/* Looping specification */
	bool	swap;			/* do byte- or word-swap */
	bool	seekable;		/* can seek on this file */
	char	*filename;		/* file name */
	char	*filetype;		/* type of file */
	char	*comment;		/* comment string */
	FILE	*fp;			/* File stream pointer */
	format_t *h;			/* format struct for this file */
	double	priv[PRIVSIZE/8];	/* format's private data area */
};

extern struct soundstream informat, outformat;
typedef struct soundstream *ft_t;

/* flags field */
#define FILE_STEREO	1	/* does file format support stereo? */
#define FILE_LOOPS	2	/* does file format support loops? */
#define FILE_INSTR	4	/* does file format support instrument specificications? */

/* Size field */
#define	cbBYTE	1
#define	cbWORD	2
#define	cbLONG	4
#define	cbFLOAT	5
#define cbDOUBLE	6
#define cbIEEE	7		/* cbIEEE 80-bit floats.  Is it necessary? */

/* Style field */
#define UNSIGNED	1	/* unsigned linear: Sound Blaster */
#define SIGN2		2	/* signed linear 2's comp: Mac */
#define	ULAW		3	/* U-law signed logs: US telephony, SPARC */
#define ALAW		4	/* A-law signed logs: non-US telephony */

extern char *sizes[], *styles[];


struct effect_t;
struct effect {
	char		*name;		/* effect name */
	struct signalinfo ininfo;	/* input signal specifications */
	struct loopinfo   loops[8];	/* input loops  specifications */
	struct instrinfo  instr;	/* input instrument  specifications */
	struct signalinfo outinfo;	/* output signal specifications */
	effect_t 	*h;		/* effects driver */
	long		*obuf;		/* output buffer */
	long		odone, olen;	/* consumed, total length */
	double		priv[PRIVSIZE];	/* private area for effect */
};

typedef struct effect *eff_t;


/*
 * Handler structure for each effect.
 */

struct effect;
typedef struct effect_t {
	char	*name;			/* effect name */
	int	flags;			/* this and that */
	void (*getopts)( struct effect* effp, int n, char* argv[] );		// process arguments
	void (*start)( eff_t );		/* start off effect */
	void (*flow)(eff_t effp, long* ibuf, long* obuf, int* isamp, int* osamp);		/* do a buffer */
	void (*drain)( eff_t effp, long* obuf, long* osamp );		/* drain out at end */
	void (*stop)( eff_t effp );		/* finish up effect */
} effect_t;

extern ;effect_t effects[];

#define	EFF_CHAN	1		/* Effect can mix channels up/down */
#define EFF_RATE	2		/* Effect can alter data rate */
#define EFF_MCHAN	4		/* Effect can handle multi-channel */
#define EFF_REPORT	8		/* Effect does nothing */

#if	defined(__STDC__) || defined(ARM)
#define	P1(x) x
#define	P2(x,y) x, y
#define	P3(x,y,z) x, y, z
#define	P4(x,y,z,w) x, y, z, w
#else
#define P1(x)
#define P2(x,y)
#define P3(x,y,z)
#define P4(x,y,z,w)
#endif

/* Utilities to read and write shorts and longs little-endian and big-endian */
unsigned short rlshort(ft_t ft);			/* short little-end */
unsigned short rbshort(ft_t ft);			/* short big-end    */
void wlshort( ft_t ft, unsigned short us );	/* short little-end */
void wbshort(ft_t ft, unsigned short us);	/* short big-end    */
unsigned long  rllong(ft_t ft);			/* long little-end  */
unsigned long  rblong(ft_t ft);			/* long big-end     */
void wllong(ft_t ft, unsigned long ul);	/* long little-end  */
void wblong(ft_t ft, unsigned long ul);	/* long big-end     */
/* Read and write words and longs in "machine format".  Swap if indicated.  */
unsigned short rshort(ft_t ft);
void wshort(ft_t ft, unsigned short us);
unsigned long  rlong(ft_t ft);
void wlong(ft_t ft, unsigned long ul);
float          rfloat(ft_t ft);
void wfloat(ft_t ft, float f);
double         rdouble(ft_t ft);
void wdouble(ft_t ft, double d);

/* Utilities to byte-swap values */
unsigned short swapw(unsigned short us);		/* Swap short */
unsigned long  swapl(unsigned long ul);		/* Swap long */
float  	       swapf(float f);			/* Swap float */
double 	       swapd(double d);			/* Swap double */

#ifdef ARM
double sfloor(double x);   /* Hack our way around the flawed */
double sceil(double x);    /* UnixLib floor ceil functions */
#endif


void report(char *, ...);
void warn(char *, ...);
void fail(char *, ...);

typedef	unsigned int u_i;
typedef	unsigned long u_l;
typedef	unsigned short u_s;

extern float volume;	/* expansion coefficient */
extern int dovolume;

extern float amplitude;	/* Largest sample so far */

extern int writing;	/* are we writing to a file? */

/* export flags */
extern int verbose;	/* be noisy on stderr */
extern int summary;	/* just print summary of information */

extern char *myname;

extern int soxpreview;	/* Preview mode: be fast and ugly */

#define	MAXRATE	50L * 1024			/* maximum sample rate */

#if  defined(unix) || defined (__OS2__)
/* Some wacky processors don't have arithmetic down shift, so do divs */
/* Most compilers will turn this into a shift if they can, don't worry */
#define RIGHT(datum, bits)	((datum) / (1L << bits))
#define LEFT(datum, bits)	((datum) << bits)
#else
/* x86 & 68k PC's have arith shift ops and dumb compilers */
#define RIGHT(datum, bits)	((datum) >> bits)
#define LEFT(datum, bits)	((datum) << bits)
#endif

#ifndef	M_PI
#define M_PI	3.14159265358979323846
#endif

#if	defined(unix) || defined(AMIGA) || defined (__OS2__) \
	|| defined(OS9) || defined(ARM)
#define READBINARY	"r"
#define WRITEBINARY	"w"
#endif
#ifdef	VMS
#define READBINARY      "r", "mbf=16", "ctx=stm"
#define WRITEBINARY     "w", "ctx=stm"
#endif
#if defined(DOS) || defined(_WIN32)
#define READBINARY	"rb"
#define WRITEBINARY	"wb"
#endif

/* Error code reporting */
#ifdef	QNX
#include <cerrno>
#endif

#if defined(unix) || defined(__OS2__)
#include <cerrno>
extern errno;
#endif

const char* version();			/* return version number */

// Considered standard
long rawread(ft_t ft, long* buf, long nsamp);
void rawwrite(ft_t ft, long* buf, long nsamp);

void gettype(ft_t formp);
void checkformat(ft_t ft);
int copyformat(ft_t ft, ft_t ft2);
void cmpformats(ft_t ft, ft_t ft2);

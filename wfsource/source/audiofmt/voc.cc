/*
 * September 8, 1993
 * Copyright 1993 T. Allen Grider - for changes to support block type 9
 * and word sized samples.  Same caveats and disclaimer as below.
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained.
 * Lance Norskog And Sundry Contributors are not responsible for
 * the consequences of using this software.
 */

/*
 * Sound Tools Sound Blaster VOC handler sources.
 *
 * Outstanding problem: the Sound Blaster DMA clock is 8 bits wide,
 * giving spotty resolution above 10khz.  voctartwrite() should check
 * the given output rate and make sure it's +-1% what the SB can
 * actually do.  Other format drivers should do similar checks.
 */

#include <audiofmt/st.h>
#include <cstring>

/* Private data for VOC file */
typedef struct vocstuff {
	long	rest;			/* bytes remaining in current block */
	long	rate;			/* rate code (byte) of this chunk */
	int	silent;			/* sound or silence? */
	long	srate;			/* rate code (byte) of silence */
	int	blockseek;		/* start of current output block */
	long	samples;		/* number of samples output */
	int	size;			/* word length of data */
	int	channels;		/* number of sound channels */
} *vs_t;

#define	VOC_TERM	0
#define	VOC_DATA	1
#define	VOC_CONT	2
#define	VOC_SILENCE	3
#define	VOC_MARKER	4
#define	VOC_TEXT	5
#define	VOC_LOOP	6
#define	VOC_LOOPEND	7
#define VOC_DATA_16	9

#define	min(a, b)	(((a) < (b)) ? (a) : (b))

static void getblock(ft_t);
static void blockstart(ft_t ft);
static void blockstop(ft_t ft);

void
vocstartread(ft_t ft)
{
	char header[20];
	vs_t v = (vs_t) ft->priv;
	int sbseek;

	if (! ft->seekable)
		fail("VOC input file must be a file, not a pipe");
	if (fread(header, 1, 20, ft->fp) != 20)
		fail("unexpected EOF in VOC header");
	if (strncmp(header, "Creative Voice File\032", 19))
		fail("VOC file header incorrect");

	sbseek = rlshort(ft);
	fseek(ft->fp, sbseek, 0);

	v->rate = -1;
	v->rest = 0;
	getblock(ft);
	if (v->rate == -1)
		fail("Input .voc file had no sound!");

	if (v->rate < 256)
	    ft->info.rate = 1000000.0/(256 - v->rate);
	else
	    ft->info.rate = v->rate;
	ft->info.size = v->size;
	ft->info.style = UNSIGNED;
	if (v->size == cbWORD)
	    ft->info.style = SIGN2;
	if (ft->info.channels == -1)
		ft->info.channels = v->channels;
}


long
vocread( ft_t ft, long* buf, long len )
{
	vs_t v = (vs_t) ft->priv;
	int done = 0;

	if (v->rest == 0)
		getblock(ft);
	if (v->rest == 0)
		return 0;

	if (v->silent) {
		/* Fill in silence */
		for(;v->rest && (done < len); v->rest--, done++)
			*buf++ = 0x80000000;
	} else {
		for(;v->rest && (done < len); v->rest--, done++) {
			long l1, l2;
			switch(v->size)
			{
			    case cbBYTE:
				if ((l1 = getc(ft->fp)) == EOF) {
				    fail("VOC input: short file"); /* */
				    v->rest = 0;
				    return 0;
				}
				l1 ^= 0x80;	/* convert to signed */
				*buf++ = LEFT(l1, 24);
				break;
			    case cbWORD:
				l1 = getc(ft->fp);
				l2 = getc(ft->fp);
				if (l1 == EOF || l2 == EOF)
				{
				    fail("VOC input: short file");
				    v->rest = 0;
				    return 0;
				}
				l1 = (l2 << 8) | l1; /* already sign2 */
				*buf++ = LEFT(l1, 16);
				v->rest--;
				break;
			}
		}
	}
	return done;
}

/* nothing to do */
void
vocstopread(ft_t)
{
}


void
vocstartwrite(ft_t ft)
{
	vs_t v = (vs_t) ft->priv;

	if (! ft->seekable)
		fail("Output .voc file must be a file, not a pipe");

	v->samples = 0;

	/* File format name and a ^Z (aborts printing under DOS) */
	(void) fwrite("Creative Voice File\032\032", 1, 20, ft->fp);
	wlshort(ft, 26);			/* size of header */
	wlshort(ft, 0x10a);                     /* major/minor version number */
	wlshort(ft, 0x1129);			/* checksum of version number */

	ft->info.size = cbBYTE;
	ft->info.style = UNSIGNED;
	if (ft->info.channels == -1)
		ft->info.channels = 1;
}


void
vocwrite(ft_t ft, long* buf, long len)
{
	vs_t v = (vs_t) ft->priv;
	unsigned char uc;

	v->rate = 256 - (1000000.0/(float)ft->info.rate);	/* Rate code */
	if (v->samples == 0) {
		/* No silence packing yet. */
		v->silent = 0;
		blockstart(ft);
	}
	v->samples += len;
	while(len--) {
		uc = RIGHT(*buf++, 24);
		uc ^= 0x80;
		putc(uc, ft->fp);
	}
}

void
vocstopwrite(ft_t ft)
{
	blockstop(ft);
}

/* Voc-file handlers */

/* Read next block header, save info, leave position at start of data */
void
getblock(ft_t ft)
{
	vs_t v = (vs_t) ft->priv;
	unsigned char uc, block;
	unsigned long sblen;
	long new_rate;
	int i;

	v->silent = 0;
	while (v->rest == 0) {
		if (feof(ft->fp))
			return;
		block = getc(ft->fp);
		if (block == VOC_TERM)
			return;
		if (feof(ft->fp))
			return;
		uc = getc(ft->fp);
		sblen = uc;
		uc = getc(ft->fp);
		sblen |= ((long) uc) << 8;
		uc = getc(ft->fp);
		sblen |= ((long) uc) << 16;
		switch(block) {
		case VOC_DATA:
			uc = getc(ft->fp);
			if (uc == 0)
			   fail("File %s: Sample rate is zero?");
			if ((v->rate != -1) && (uc != v->rate))
			   fail("File %s: sample rate codes differ: %d != %d",
					v->rate, uc);
			v->rate = uc;
			uc = getc(ft->fp);
			if (uc != 0)
				fail("File %s: only interpret 8-bit data!");
			v->rest = sblen - 2;
			v->size = cbBYTE;
			v->channels = 1;
			return;
		case VOC_DATA_16:
			new_rate = rllong(ft);
			if (new_rate == 0)
			    fail("File %s: Sample rate is zero?");
			if ((v->rate != -1) && (new_rate != v->rate))
			    fail("File %s: sample rate codes differ: %d != %d",
				v->rate, new_rate);
			v->rate = new_rate;
			uc = getc(ft->fp);
			switch (uc)
			{
			    case 8:	v->size = cbBYTE; break;
			    case 16:	v->size = cbWORD; break;
			    default:	fail("Don't understand size %d", uc);
			}
			v->channels = getc(ft->fp);
			getc(ft->fp);	/* unknown1 */
			getc(ft->fp);	/* notused */
			getc(ft->fp);	/* notused */
			getc(ft->fp);	/* notused */
			getc(ft->fp);	/* notused */
			getc(ft->fp);	/* notused */
			v->rest = sblen - 12;
			return;
		case VOC_CONT:
			v->rest = sblen;
			return;
		case VOC_SILENCE:
			{
			unsigned short period;

			period = rlshort(ft);
			uc = getc(ft->fp);
			if (uc == 0)
				fail("File %s: Silence sample rate is zero");
			/*
			 * Some silence-packed files have gratuitously
			 * different sample rate codes in silence.
			 * Adjust period.
			 */
			if ((v->rate != -1) && (uc != v->rate))
				period = (period * (256 - uc))/(256 - v->rate);
			else
				v->rate = uc;
			v->rest = period;
			v->silent = 1;
			return;
			}
		case VOC_MARKER:
			uc = getc(ft->fp);
			uc = getc(ft->fp);
			/* Falling! Falling! */
		case VOC_TEXT:
			{
			int i;
			/* Could add to comment in SF? */
			for(i = 0; i < sblen; i++)
				getc(ft->fp);
			}
			continue;	/* get next block */
		case VOC_LOOP:
		case VOC_LOOPEND:
			report("File %s: skipping repeat loop");
			for(i = 0; i < sblen; i++)
				getc(ft->fp);
			break;
		default:
			report("File %s: skipping unknown block code %d",
				ft->filename, block);
			for(i = 0; i < sblen; i++)
				getc(ft->fp);
		}
	}
}

/* Start an output block. */
static void
blockstart(ft_t ft)
{
	vs_t v = (vs_t) ft->priv;

	v->blockseek = ftell(ft->fp);
	if (v->silent) {
		putc(VOC_SILENCE, ft->fp);	/* Silence block code */
		putc(0, ft->fp);		/* Period length */
		putc(0, ft->fp);		/* Period length */
		putc((int) v->rate, ft->fp);		/* Rate code */
	} else {
		putc(VOC_DATA, ft->fp);		/* Voice Data block code */
		putc(0, ft->fp);		/* block length (for now) */
		putc(0, ft->fp);		/* block length (for now) */
		putc(0, ft->fp);		/* block length (for now) */
		putc((int) v->rate, ft->fp);		/* Rate code */
		putc(0, ft->fp);		/* 8-bit raw data */
	}
}

/* End the current data or silence block. */
static void
blockstop(ft_t ft)
{
	vs_t v = (vs_t) ft->priv;
	long datum;

	putc(0, ft->fp);			/* End of file block code */
	fseek(ft->fp, v->blockseek, 0);		/* seek back to block length */
	fseek(ft->fp, 1, 1);			/* seek forward one */
	if (v->silent) {
		datum = (v->samples) & 0xff;
		putc((int)datum, ft->fp);       /* low byte of length */
		datum = (v->samples >> 8) & 0xff;
		putc((int)datum, ft->fp);  /* high byte of length */
	} else {
		v->samples += 2;		/* adjustment: SBDK pp. 3-5 */
		datum = (v->samples) & 0xff;
		putc((int)datum, ft->fp);       /* low byte of length */
		datum = (v->samples >> 8) & 0xff;
		putc((int)datum, ft->fp);  /* middle byte of length */
		datum = (v->samples >> 16) & 0xff;
		putc((int)datum, ft->fp); /* high byte of length */
	}
}


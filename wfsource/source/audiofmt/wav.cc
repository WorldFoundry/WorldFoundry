/*
 * April 15, 1992
 * Copyright 1992 Rick Richardson
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained.
 * Lance Norskog And Sundry Contributors are not responsible for
 * the consequences of using this software.
 */

/*
 * Windows 3.0 .wav format driver
 */

/*
 * Fixed by various contributors:
 * 1) Little-endian handling
 * 2) Skip other kinds of file data
 * 3) Handle 16-bit formats correctly
 * 4) Not go into infinite loop
 */

/*
 * User options should overide file
 * header :- we assumed user knows what
 * they are doing if they specify options.
 * Enhancements and clean up
 * by Graeme W. Gill, 93/5/17
 */

#include <audiofmt/st.h>
#include <audiofmt/wav.h>

#include <cstring>

/* Private data for .wav file */
typedef struct wavstuff {
	long	samples;
	int	second_header;	/* non-zero on second header write */
} *wav_t;

static char* wav_format_str( unsigned );
static void wavwritehdr(ft_t ft);

/*
 * Do anything required before you start reading samples.
 * Read file header.
 *	Find out sampling rate,
 *	size and style of samples,
 *	mono/stereo/quad.
 */
void
wavstartread(ft_t ft)
{
	wav_t	wav = (wav_t) ft->priv;
	char	magic[4];
        unsigned len;
	int	littlendian = 1;
	char	*endptr;
	char	c;

        /* wave file characteristics */
        unsigned short wFormatTag;              /* data format */
        unsigned short wChannels;               /* number of channels */
        unsigned long  wSamplesPerSecond;       /* samples per second per channel */
        unsigned long  wAvgBytesPerSec;         /* estimate of bytes per second needed */
        unsigned short wBlockAlign;             /* byte alignment of a basic sample block */
        unsigned short wBitsPerSample;          /* bits per sample */
        unsigned long  data_length;             /* length of sound data in bytes */
	unsigned long  bytespersample; 		/* bytes per sample (per channel) */

	endptr = (char *) &littlendian;
	if (!*endptr) ft->swap = 1;

	/* If you need to seek around the input file. */
	if (0 && ! ft->seekable)
		fail("Sorry, .wav input file must be a file, not a pipe");

	if (   fread(magic, 1, 4, ft->fp) != 4
	    || strncmp("RIFF", magic, 4))
		fail("Sorry, not a RIFF file");

	len = rllong(ft);

	if (   fread(magic, 1, 4, ft->fp) != 4
	    || strncmp("WAVE", magic, 4))
		fail("Sorry, not a WAVE file");

	/* Now look for the format chunk */
	for (;;)
	{
		if ( fread(magic, 1, 4, ft->fp) != 4 )
			fail("Sorry, missing fmt spec");
		len = rllong(ft);
		if (strncmp("fmt ", magic, 4) == 0)
			break;	/* Found the format chunk */
		while (len > 0 && !feof(ft->fp))	/* skip to next chunk */
		{
			getc(ft->fp);
			len--;
		}
	}

	if ( len < 16 )
		fail("Sorry, fmt chunk is too short");

	wFormatTag = rlshort(ft);
	switch (wFormatTag)
	{
		case WAVE_FORMAT_UNKNOWN:
		fail("Sorry, this WAV file is in Microsoft Official Unknown format.");
		case WAVE_FORMAT_PCM: 	/* this one, at least, I can handle */
			if (ft->info.style != -1 && ft->info.style != UNSIGNED && ft->info.style != SIGN2)
				warn("User options overiding style read in .wav header");
			break;
		case WAVE_FORMAT_ADPCM:
		fail("Sorry, this WAV file is in Microsoft ADPCM format.");
		case WAVE_FORMAT_ALAW:	/* Think I can handle this */
			if (ft->info.style == -1 || ft->info.style == ALAW)
				ft->info.style = ALAW;
			else
				warn("User options overiding style read in .wav header");
			break;
		case WAVE_FORMAT_MULAW:	/* Think I can handle this */
			if (ft->info.style == -1 || ft->info.style == ULAW)
				ft->info.style = ULAW;
			else
				warn("User options overiding style read in .wav header");
			break;
		case WAVE_FORMAT_OKI_ADPCM:
		fail("Sorry, this WAV file is in OKI ADPCM format.");
		case WAVE_FORMAT_DIGISTD:
		fail("Sorry, this WAV file is in Digistd format.");
		case WAVE_FORMAT_DIGIFIX:
		fail("Sorry, this WAV file is in Digifix format.");
		case IBM_FORMAT_MULAW:
		fail("Sorry, this WAV file is in IBM U-law format.");
		case IBM_FORMAT_ALAW:
		fail("Sorry, this WAV file is in IBM A-law format.");
		case IBM_FORMAT_ADPCM:
		fail("Sorry, this WAV file is in IBM ADPCM format.");
	default:	fail("Sorry, don't understand format");
	}
	wChannels = rlshort(ft);
	/* User options take precedence */
	if (ft->info.channels == -1 || ft->info.channels == wChannels)
		ft->info.channels = wChannels;
	else
		warn("User options overiding channels read in .wav header");
	wSamplesPerSecond = rllong(ft);
	if (ft->info.rate == 0 || ft->info.rate == wSamplesPerSecond)
		ft->info.rate = wSamplesPerSecond;
	else
		warn("User options overiding rate read in .wav header");
	wAvgBytesPerSec = rllong(ft);	/* Average bytes/second */
	wBlockAlign = rlshort(ft);	/* Block align */
	wBitsPerSample =  rlshort(ft);	/* bits per sample per channel */
	bytespersample = (wBitsPerSample + 7)/8;
	switch (bytespersample)
	{
		case 1:
			/* User options take precedence */
			if (ft->info.size == -1 || ft->info.size == cbBYTE)
				ft->info.size = cbBYTE;
			else
				warn("User options overiding size read in .wav header");
			if (ft->info.style == -1 || ft->info.style == UNSIGNED)
				ft->info.style = UNSIGNED;
			else if (ft->info.style != ALAW && ft->info.style != ULAW)
				warn("User options overiding style read in .wav header");
			break;
		case 2:
			if (ft->info.size == -1 || ft->info.size == cbWORD)
				ft->info.size = cbWORD;
			else
				warn("User options overiding size read in .wav header");
			if (ft->info.style == -1 || ft->info.style == SIGN2)
				ft->info.style = SIGN2;
			else
				warn("User options overiding style read in .wav header");
			break;
		case 4:
			if (ft->info.size == -1 || ft->info.size == cbLONG)
				ft->info.size = cbLONG;
			else
				warn("User options overiding size read in .wav header");
			if (ft->info.style == -1 || ft->info.style == SIGN2)
				ft->info.style = SIGN2;
			else
				warn("User options overiding style read in .wav header");
			break;
		default:
			fail("Sorry, don't understand .wav size");
	}
	len -= 16;
	while (len > 0 && !feof(ft->fp))
	{
		getc(ft->fp);
		len--;
	}

	/* Now look for the wave data chunk */
	for (;;)
	{
		if ( fread(magic, 1, 4, ft->fp) != 4 )
			fail("Sorry, missing data chunk");
		len = rllong(ft);
		if (strncmp("data", magic, 4) == 0)
			break;	/* Found the data chunk */
		while (len > 0 && !feof(ft->fp)) /* skip to next chunk */
		{
			getc(ft->fp);
			len--;
		}
	}
	data_length = len;
	wav->samples = data_length/ft->info.size;	/* total samples */

	report("Reading Wave file: %s format, %d channel%s, %d samp/sec",
	        wav_format_str(wFormatTag), wChannels,
	        wChannels == 1 ? "" : "s", wSamplesPerSecond);
	report("        %d byte/sec, %d block align, %d bits/samp, %u data bytes\n",
                wAvgBytesPerSec, wBlockAlign, wBitsPerSample, data_length);
}

/*
 * Read up to len samples from file.
 * Convert to signed longs.
 * Place in buf[].
 * Return number of samples read.
 */

long
wavread(ft_t ft, long* buf, long len)
{
	wav_t	wav = (wav_t) ft->priv;
	int	done;

	if (len > wav->samples) len = wav->samples;
	if (len == 0)
		return 0;
	done = rawread(ft, buf, len);
	if (done == 0)
		warn("Premature EOF on .wav input file");
	wav->samples -= done;
	return done;
}

/*
 * Do anything required when you stop reading samples.
 * Don't close input file!
 */
void
wavstopread(ft_t ft)
{
}

void
wavstartwrite(ft_t ft)
{
	wav_t	wav = (wav_t) ft->priv;
	int	littlendian = 1;
	char	*endptr;

	endptr = (char *) &littlendian;
	if (!*endptr) ft->swap = 1;

	wav->samples = 0;
	wav->second_header = 0;
	if (! ft->seekable)
		warn("Length in output .wav header will wrong since can't seek to fix it");
	wavwritehdr(ft);
}

static void
wavwritehdr(ft_t ft)
{
	wav_t	wav = (wav_t) ft->priv;

        /* wave file characteristics */
        unsigned short wFormatTag;              /* data format */
        unsigned short wChannels;               /* number of channels */
        unsigned long  wSamplesPerSecond;       /* samples per second per channel */
        unsigned long  wAvgBytesPerSec;         /* estimate of bytes per second needed */
        unsigned short wBlockAlign;             /* byte alignment of a basic sample block */
        unsigned short wBitsPerSample;          /* bits per sample */
        unsigned long  data_length;             /* length of sound data in bytes */
	unsigned long  bytespersample; 		/* bytes per sample (per channel) */

	switch (ft->info.size)
	{
		case cbBYTE:
			wBitsPerSample = 8;
			if (ft->info.style == -1 || ft->info.style == UNSIGNED)
				ft->info.style = UNSIGNED;
			else if (!wav->second_header && ft->info.style != ALAW && ft->info.style != ULAW)
				warn("User options overiding style written to .wav header");
			break;
		case cbWORD:
			wBitsPerSample = 16;
			if (ft->info.style == -1 || ft->info.style == SIGN2)
				ft->info.style = SIGN2;
			else if (!wav->second_header)
				warn("User options overiding style written to .wav header");
			break;
		case cbLONG:
			wBitsPerSample = 32;
			if (ft->info.style == -1 || ft->info.style == SIGN2)
				ft->info.style = SIGN2;
			else if (!wav->second_header)
				warn("User options overiding style written to .wav header");
			break;
		default:
			wBitsPerSample = 32;
			if (ft->info.style == -1)
				ft->info.style = SIGN2;
			if (!wav->second_header)
				warn("Warning - writing bad .wav file using %s",sizes[ft->info.size]);
			break;
	}

	switch (ft->info.style)
	{
		case UNSIGNED:
			wFormatTag = WAVE_FORMAT_PCM;
			if (wBitsPerSample != 8 && !wav->second_header)
				warn("Warning - writing bad .wav file using unsigned data and %d bits/sample",wBitsPerSample);
			break;
		case SIGN2:
			wFormatTag = WAVE_FORMAT_PCM;
			if (wBitsPerSample == 8 && !wav->second_header)
				warn("Warning - writing bad .wav file using signed data and %d bits/sample",wBitsPerSample);
			break;
		case ALAW:
			wFormatTag = WAVE_FORMAT_ALAW;
			if (wBitsPerSample != 8 && !wav->second_header)
				warn("Warning - writing bad .wav file using A-law data and %d bits/sample",wBitsPerSample);
			break;
		case ULAW:
			wFormatTag = WAVE_FORMAT_MULAW;
			if (wBitsPerSample != 8 && !wav->second_header)
				warn("Warning - writing bad .wav file using U-law data and %d bits/sample",wBitsPerSample);
			break;
	}


	wSamplesPerSecond = ft->info.rate;
	bytespersample = (wBitsPerSample + 7)/8;
	wAvgBytesPerSec = ft->info.rate * ft->info.channels * bytespersample;
	wChannels = ft->info.channels;
	wBlockAlign = ft->info.channels * bytespersample;
	if (!wav->second_header)	/* use max length value first time */
		data_length = 0x7fffffff - (8+16+12);
	else	/* fixup with real length */
		data_length = bytespersample * wav->samples;

	/* figured out header info, so write it */
	fputs("RIFF", ft->fp);
	wllong(ft, data_length + 8+16+12);	/* Waveform chunk size: FIXUP(4) */
	fputs("WAVE", ft->fp);
	fputs("fmt ", ft->fp);
	wllong(ft, (long)16);		/* fmt chunk size */
	wlshort(ft, wFormatTag);
	wlshort(ft, wChannels);
	wllong(ft, wSamplesPerSecond);
	wllong(ft, wAvgBytesPerSec);
	wlshort(ft, wBlockAlign);
	wlshort(ft, wBitsPerSample);

	fputs("data", ft->fp);
	wllong(ft, data_length);		/* data chunk size: FIXUP(40) */

	if (!wav->second_header) {
		report("Writing Wave file: %s format, %d channel%s, %d samp/sec",
	        	wav_format_str(wFormatTag), wChannels,
	        	wChannels == 1 ? "" : "s", wSamplesPerSecond);
		report("        %d byte/sec, %d block align, %d bits/samp",
	                wAvgBytesPerSec, wBlockAlign, wBitsPerSample);
	} else
		report("Finished writing Wave file, %u data bytes\n",data_length);
}


void
wavwrite(ft_t ft, long* buf, long len)
{
	wav_t	wav = (wav_t) ft->priv;

	wav->samples += len;
	rawwrite(ft, buf, len);
}

void
wavstopwrite(ft_t ft)
{
	/* All samples are already written out. */
	/* If file header needs fixing up, for example it needs the */
 	/* the number of samples in a field, seek back and write them here. */
	if (!ft->seekable)
		return;
	if (fseek(ft->fp, 0L, 0) != 0)
		fail("Sorry, can't rewind output file to rewrite .wav header.");
	((wav_t) ft->priv)->second_header = 1;
	wavwritehdr(ft);
}

/*
 * Return a string corresponding to the wave format type.
 */
static char *
wav_format_str(unsigned wFormatTag)
{
	switch (wFormatTag)
	{
		case WAVE_FORMAT_UNKNOWN:
			return "Microsoft Official Unknown";
		case WAVE_FORMAT_PCM:
			return "Microsoft PCM";
		case WAVE_FORMAT_ADPCM:
			return "Microsoft ADPCM";
		case WAVE_FORMAT_ALAW:
			return "Microsoft A-law";
		case WAVE_FORMAT_MULAW:
			return "Microsoft U-law";
		case WAVE_FORMAT_OKI_ADPCM:
			return "OKI ADPCM format.";
		case WAVE_FORMAT_DIGISTD:
			return "Digistd format.";
		case WAVE_FORMAT_DIGIFIX:
			return "Digifix format.";
		case IBM_FORMAT_MULAW:
			return "IBM U-law format.";
		case IBM_FORMAT_ALAW:
			return "IBM A-law";
                case IBM_FORMAT_ADPCM:
                	return "IBM ADPCM";
		default:
			return "Unknown";
	}
}

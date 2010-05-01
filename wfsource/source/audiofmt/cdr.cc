/*
 * July 5, 1991
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained.
 * Lance Norskog And Sundry Contributors are not responsible for
 * the consequences of using this software.
 */

/*
 * CD-R format handler
 *
 * David Elliott, Sony Microsystems
 *
 * This code automatically handles endianness differences
 */

#include <audiofmt/st.h>

#define SECTORSIZE	(2352 / 2)

/* Private data for SKEL file */
typedef struct cdrstuff {
	int	samples;	/* number of samples written */
} *cdr_t;

/*
 * Do anything required before you start reading samples.
 * Read file header.
 *	Find out sampling rate,
 *	size and style of samples,
 *	mono/stereo/quad.
 */

void
cdrstartread(ft_t ft)
{

	int     littlendian = 1;
	char    *endptr;

	endptr = (char *) &littlendian;
	if (!*endptr) ft->swap = 1;

	ft->info.rate = 44100;
	ft->info.size = cbWORD;
	ft->info.style = SIGN2;
	ft->info.channels = 2;
	ft->comment = NULL;
}

/*
 * Read up to len samples from file.
 * Convert to signed longs.
 * Place in buf[].
 * Return number of samples read.
 */

long
cdrread(ft_t ft, long* buf, long len)
{

	return rawread(ft, buf, len);
}

/*
 * Do anything required when you stop reading samples.
 * Don't close input file!
 */
void
cdrstopread(ft_t ft)
{
}


void
cdrstartwrite(ft_t ft)
{
	cdr_t cdr = (cdr_t) ft->priv;

	int     littlendian = 1;
	char    *endptr;

	endptr = (char *) &littlendian;
	if (!*endptr) ft->swap = 1;

	cdr->samples = 0;

	ft->info.rate = 44100;
	ft->info.size = cbWORD;
	ft->info.style = SIGN2;
	ft->info.channels = 2;

}


void
cdrwrite(ft_t ft, long* buf, long len)
{
	cdr_t cdr = (cdr_t) ft->priv;

	cdr->samples += len;

	rawwrite(ft, buf, len);
}

/*
 * A CD-R file needs to be padded to SECTORSIZE, which is in terms of
 * samples.  We write -32768 for each sample to pad it out.
 */

void
cdrstopwrite(ft_t ft)
{
	cdr_t cdr = (cdr_t) ft->priv;
	int padsamps = SECTORSIZE - (cdr->samples % SECTORSIZE);
	short pad[2];

	pad[0] = 0;

	if (padsamps == SECTORSIZE) {
		return;
	}

	while (padsamps > 0) {
		rawwrite(ft, (long*)pad, 1);
		padsamps--;
	}
}


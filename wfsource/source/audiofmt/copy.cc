/*
 * July 5, 1991
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained.
 * Lance Norskog And Sundry Contributors are not responsible for
 * the consequences of using this software.
 */

/*
 * Sound Tools skeleton effect file.
 */

#include <audiofmt/st.h>
#include <cstring>

// Process options
void
copy_getopts(eff_t effp, int n, char* argv[] )
{
	if (n)
		fail("Copy effect takes no options.");
}

// Start processing
void
copy_start(eff_t effp)
{
	/* nothing to do */
	/* stuff data into delaying effects here */
}

/*
 * Read up to len samples from file.
 * Convert to signed longs.
 * Place in buf[].
 * Return number of samples read.
 */

void
copy_flow(eff_t effp, long* ibuf, long* obuf, int* isamp, int* osamp )
{
	int done;

	done = ((*isamp < *osamp) ? *isamp : *osamp);
	memcpy(obuf, ibuf, done * sizeof(long));
	*isamp = *osamp = done;
//	return done;
}

/*
 * Do anything required when you stop reading samples.
 * Don't close input file!
 */
void
copy_stop(eff_t)
{
	/* nothing to do */
}





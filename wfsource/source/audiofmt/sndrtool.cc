/*
 * Sounder/Sndtool format handler: W V Neisius, February 1992
 *
 * June 28, 93: force output to mono.
 */

#include <audiofmt/st.h>
#include <math.h>

// Private data used by writer
struct sndpriv {
        unsigned long nsamples;
};

#include <cstdio>
#include <cstring>

void sndtwriteheader( ft_t ft,long nsamples );

/*======================================================================*/
/*                         SNDSTARTREAD                                */
/*======================================================================*/

void
sndtstartread(ft_t ft)
{
struct sndpriv *p = (struct sndpriv *) ft->priv;

char buf[97];

long rate;

rate = 0;

/* determine file type */
        /* if first 5 bytes == SOUND then this is probably a sndtool sound */
        /* if first word (16 bits) == 0
         and second word is between 4000 & 25000 then this is sounder sound */
        /* otherwise, its probably raw, not handled here */

if (fread(buf, 1, 2, ft->fp) != 2)
	fail("SND: unexpected EOF");
if (strncmp(buf,"\0\0",2) == 0)
	{
	/* sounder */
	rate = rlshort(ft);
	if (rate < 4000 || rate > 25000 )
		fail ("SND: sample rate out of range");
	fseek(ft->fp,4,SEEK_CUR);
	}
else
	{
	/* sndtool ? */
	fread(&buf[2],1,6,ft->fp);
	if (strncmp(buf,"SOUND",5))
		fail ("SND: unrecognized SND format");
	fseek(ft->fp,12,SEEK_CUR);
	rate = rlshort(ft);
	fseek(ft->fp,6,SEEK_CUR);
	if (fread(buf,1,96,ft->fp) != 96)
		fail ("SND: unexpected EOF in SND header");
	report ("%s",buf);
	}

ft->info.channels = 1;
ft->info.rate = rate;
ft->info.style = UNSIGNED;
ft->info.size = cbBYTE;

}

/*======================================================================*/
/*                         SNDTSTARTWRITE                               */
/*======================================================================*/
void
sndtstartwrite(ft_t ft)
{
struct sndpriv *p = (struct sndpriv *) ft->priv;

/* write header */
ft->info.channels = 1;
ft->info.style = UNSIGNED;
ft->info.size = cbBYTE;
p->nsamples = 0;
sndtwriteheader(ft, 0);

}
/*======================================================================*/
/*                         SNDRSTARTWRITE                               */
/*======================================================================*/
void
sndrstartwrite(ft_t ft)
{
/* write header */
ft->info.channels = 1;
ft->info.style = UNSIGNED;
ft->info.size = cbBYTE;

/* sounder header */
wlshort (ft,0); /* sample size code */
wlshort (ft,(int) ft->info.rate);     /* sample rate */
wlshort (ft,10);        /* volume */
wlshort (ft,4); /* shift */
}

/*======================================================================*/
/*                         SNDTWRITE                                     */
/*======================================================================*/

void
sndtwrite(ft_t ft, long* buf, long len)
{
	struct sndpriv *p = (struct sndpriv *) ft->priv;
	p->nsamples += len;
	rawwrite(ft, buf, len);
}

/*======================================================================*/
/*                         SNDTSTOPWRITE                                */
/*======================================================================*/

void
sndtstopwrite(ft_t ft)
{
struct sndpriv *p = (struct sndpriv *) ft->priv;

/* fixup file sizes in header */
if (fseek(ft->fp, 0L, 0) != 0)
	fail("can't rewind output file to rewrite SND header");
sndtwriteheader(ft, p->nsamples);
}

/*======================================================================*/
/*                         SNDTWRITEHEADER                              */
/*======================================================================*/
void
sndtwriteheader(ft_t ft,long nsamples)
{
char name_buf[97];

/* sndtool header */
fputs ("SOUND",ft->fp); /* magic */
fputc (0x1a,ft->fp);
wlshort (ft,(long)0);  /* hGSound */
wllong (ft,nsamples);
wllong (ft,(long)0);
wllong (ft,nsamples);
wlshort (ft,(int) ft->info.rate);
wlshort (ft,0);
wlshort (ft,10);
wlshort (ft,4);
sprintf (name_buf,"%s - File created by Sound Exchange",ft->filename);
fwrite (name_buf, 1, 96, ft->fp);

}



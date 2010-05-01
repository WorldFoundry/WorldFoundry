/*
 * Psion wve format, based on the au format file. Hacked by
 * Richard Caley (R.Caley@ed.ac.uk)
 */

#include <audiofmt/st.h>
#include <audiofmt/g72x.h>

#include <cstring>

/* Magic numbers used in Psion audio files */
#define PSION_MAGIC     "ALawSoundFile**"
#define PSION_VERSION   ((short)3856)
#define PSION_HDRSIZE	32

struct wvepriv
{
    unsigned int length;
    short padding;
    short repeats;
};

void wvewriteheader( ft_t ft );

void
wvestartread(ft_t ft)
{
	struct wvepriv *p = (struct wvepriv *) ft->priv;
	char magic[16];
	short version;
	unsigned int length;


	/* Sanity check */
	if (sizeof(struct wvepriv) > PRIVSIZE)
		fail(
"struct wvepriv is too big (%d); change PRIVSIZE in st.h and recompile sox",
		     sizeof(struct wvepriv));

	/* Check the magic word */
        fread(magic, 16, 1, ft->fp);
	if (strcmp(magic, PSION_MAGIC)==0) {
		ft->swap = 0;
		report("Found Psion magic word");
	}
	else
		fail("Psion header doesn't start with magic word\nTry the '.al' file type with '-t al -r 8000 filename'");

        version=rshort(ft);

	if (version != PSION_VERSION)
		fail("Wron version in Psion header");

     	p->length=rlong(ft);

	p->padding=rshort(ft);

	p->repeats=rshort(ft);

 	(void)rshort(ft);
 	(void)rshort(ft);
 	(void)rshort(ft);

	ft->info.style = ALAW;
	ft->info.size = cbBYTE;

	ft->info.rate = 8000;

	ft->info.channels = 1;
}

/* When writing, the header is supposed to contain the number of
   data bytes written, unless it is written to a pipe.
   Since we don't know how many bytes will follow until we're done,
   we first write the header with an unspecified number of bytes,
   and at the end we rewind the file and write the header again
   with the right size.  This only works if the file is seekable;
   if it is not, the unspecified size remains in the header
   (this is illegal). */


void
wvestartwrite(ft_t ft)
{
	struct wvepriv *p = (struct wvepriv *) ft->priv;

	p->length = 0;
	if (p->repeats == 0)
	    p->repeats = 1;

	ft->info.style = ALAW;
	ft->info.size = cbBYTE;
	ft->info.rate = 8000;

	wvewriteheader(ft);
}


long
wveread(ft_t ft, long* buf, long samp)
{
	return rawread(ft, buf, samp);
}


void
wvewrite(ft_t ft, long* buf, long samp)
{
	struct wvepriv *p = (struct wvepriv *) ft->priv;
	p->length += samp * ft->info.size;
	rawwrite(ft, buf, samp);
}


void
wvestopwrite(ft_t ft)
{
	struct wvepriv *p = (struct wvepriv *) ft->priv;
	if (!ft->seekable)
		return;
	if (fseek(ft->fp, 0L, 0) != 0)
		fail("Can't rewind output file to rewrite Psion header.");
	wvewriteheader(ft);
}


void
wvewriteheader(ft_t ft)
{

    char magic[16];
    short version;
    short zero;
    struct wvepriv *p = (struct wvepriv *) ft->priv;

    strcpy(magic,PSION_MAGIC);
    version=PSION_VERSION;
    zero=0;

    fwrite(magic, sizeof(magic), 1, ft->fp);

    wshort(ft, version);
    wblong(ft, p->length);
    wshort(ft, p->padding);
    wshort(ft, p->repeats);

    wshort(ft, zero);
    wshort(ft, zero);
    wshort(ft, zero);

}

// nulleff.c

#include <audiofmt/st.h>

void
null_getopts( eff_t effp, int n, char* argv[] )
{
}


void
null_start( eff_t effp )
{
}


void
null_flow( eff_t effp, long* ibuf, long* obuf, int* isamp, int* osamp )
{
}

// dummy drain routine for effects
void
null_drain(eff_t effp, long* obuf, long* osamp)
{
	*osamp = 0;
}


void
null_stop( eff_t effp )
{
}

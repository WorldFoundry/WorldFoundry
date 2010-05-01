// null.c

#include <audiofmt/st.h>

void
nullstartread( ft_t )
{
}

long
nullread( ft_t, long*, long )
{
	return 0;
}

void
nullstopread( ft_t )
{
}

void
nullstartwrite( ft_t )
{
}

void
nullwrite( ft_t, long*, long )
{
}

void
nullstopwrite( ft_t )
{
}

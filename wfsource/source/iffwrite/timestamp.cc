// timestamp.cc

#include <iffwrite/iffwrite.hp>
#include <time.h>

Timestamp::Timestamp()
{
	time_t t = time( NULL );
	pTime = localtime( &t );
	_localtimestamp = mktime( pTime );
}

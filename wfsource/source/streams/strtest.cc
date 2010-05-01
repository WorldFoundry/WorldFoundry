//=============================================================================
// strtest.cc:
//=============================================================================

#include <pigsys/pigsys.hp>
#include <hal/diskfile.hp>

//-----------------------------------------------------------------------------

#if (defined(__WIN__) || defined(__LINUX__) ) && SW_DBSTREAM
#include <cpplib/strmnull.hp>
nullstream cppteststrm_null;				// Create a global instance of the output
CREATEANDASSIGNOSTREAM(cmem,cppteststrm_null);
#endif

//=============================================================================

void
PIGSMain( int argc, char* argv[] )
//main( int argc, char* argv[] )
{
	//ResetCallback();
	//CdInit();


#if 0
	printf( "CdControlB( CdlStop, 0, 0 );\n" );
	CdControlB( CdlStop, 0, 0 );
	printf( "CdSync( 0, 0 );\n" );
	CdSync( 0, 0 );
#endif

}

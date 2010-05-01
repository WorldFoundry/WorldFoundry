//==============================================================================
// scriptingtest.cc:
//==============================================================================

#include <pigsys/pigsys.hp>
#include <math/scalar.hp>
#include <math/vector3.hp>
#include <cpplib/array.hp>

//-----------------------------------------------------------------------------

#if defined(_MSC_VER) && SW_DBSTREAM
#include <cpplib/strmnull.hp>
CREATENULLSTREAM(scriptteststrm_null);				// Create a global instance of the output
CREATEANDASSIGNOSTREAM(cscript,scriptteststrm_null);
#endif

//=============================================================================


void
PIGSMain( int argc, char* argv[] )
{
   std::cout << "script test" << std::endl;                 

}

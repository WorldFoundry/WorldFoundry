// testtime.cc

#include <timer/clock.hp>

void
PIGSMain( int argc, char* argv[] )
{
	Clock clock;

	for ( int i=0; i<100; ++i )
	{
		clock.Tick( clock() + SCALAR_CONSTANT( 1.15 ) );
		cout << clock() << endl;
//		cout << clock.getDelta() << endl;
//		cout << clock.getLastDelta() << endl;
	}
}

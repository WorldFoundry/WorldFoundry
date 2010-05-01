//=============================================================================
// c program for generating test tables used by mathtest.cc
//=============================================================================


#define ENTRIES (360*4)


#include <iostream>
#include <math.h>


#define PI  3.14159265359

inline double
deg2rad(double deg)
{
	double ret = (deg*2*PI)/360;
	return ret;
}


inline double
rad2deg(double rad)
{
	double ret = (rad/(2*PI))*360;
	return ret;
}


int
main()
{
	cout << "// sine table" << endl;
	cout << "Scalar testSineTable[" << ENTRIES << "] = {" << endl;
	int index;
	for(index=0;index<ENTRIES;index++)
	{
		double value = sin(deg2rad(double(index)/4));  // in degrees
		cout << "SCALAR_CONSTANT(" << value << "),		//" << double(index)/4 << ",asin = " << rad2deg(asin(value)) << endl;
	}
	cout << "};" << endl;


	cout << "// cosine table" << endl;
	cout << "Scalar testCosTable[" << ENTRIES << "] = {" << endl;
	for(index=0;index<ENTRIES;index++)
	{
		double value = cos(deg2rad(double(index)/4));
		cout << "SCALAR_CONSTANT(" << value << "),		// " << double(index)/4 << endl;
	}
	cout << "};" << endl;

	return 0;
}

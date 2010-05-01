

#include	"global.hp"
//#include	<stdlib.h>
//#include	<stdio.h>
//#include	<conio.h>
//#include	<math.h>
//#include	<time.h>


float
SafeATan2(float rise, float run)
{
	float result;

	if ((rise < 0.00001) && (rise > -0.00001))
		rise = 0;

	if ((run < 0.00001) && (run > -0.00001))
		run = 0;

//	printf("safeatan2: rise  = %f, run = %f, result = %f\n",rise, run, result);
	result = atan2(rise,run);
	return(result);
}



void
RotationConvert(float x, float y, float z, float r, float* newX, float* newY, float* newZ)
{
	/* used to keep track of out 'north pole' */
	float	x1, y1, z1, r1;
	float	x2, y2, z2, r2;
	float	x3, y3, z3, r3;
	float	x4, y4, z4, r4;
	float	x5, y5, z5, r5;
	float	x6, y6, z6, r6;
	float	x7, y7, z7, r7;
	float	x8, y8, z8, r8;
	/* used to check a random point */
	float	a, b, c;
	float	a1, b1, c1;
	float	a2, b2, c2;
	float	a3, b3, c3;
	float	a4, b4, c4;
	float	a5, b5, c5;
	float	a6, b6, c6;
	float	a7, b7, c7;
	float	a8, b8, c8;
	/* used to track unit vectors */
	float	Xa, Xb, Xc;
	float	Xa1, Xb1, Xc1;
	float	Xa2, Xb2, Xc2;
	float	Xa3, Xb3, Xc3;
	float	Xa4, Xb4, Xc4;
	float	Xa5, Xb5, Xc5;
	float	Xa6, Xb6, Xc6;
	float	Xa7, Xb7, Xc7;
	float	Xa8, Xb8, Xc8;
	float	Ya, Yb, Yc;
	float	Ya1, Yb1, Yc1;
	float	Ya2, Yb2, Yc2;
	float	Ya3, Yb3, Yc3;
	float	Ya4, Yb4, Yc4;
	float	Ya5, Yb5, Yc5;
	float	Ya6, Yb6, Yc6;
	float	Ya7, Yb7, Yc7;
	float	Ya8, Yb8, Yc8;
	float	Za, Zb, Zc;
	float	Za1, Zb1, Zc1;
	float	Za2, Zb2, Zc2;
	float	Za3, Zb3, Zc3;
	float	Za4, Zb4, Zc4;
	float	Za5, Zb5, Zc5;
	float	Za6, Zb6, Zc6;
	float	Za7, Zb7, Zc7;
	float	Za8, Zb8, Zc8;
	/* used to work backwards */
	float	Xr, Yr, Zr;

//	clrscr();
//	randomize();

	DBSTREAM3( cdebug << "RotationConvert:initial values" << std::endl; )

	a = (rand()*2.0)/32767.0 - 1.0;			// invent test point?
	b = (rand()*2.0)/32767.0 - 1.0;
	c = (rand()*2.0)/32767.0 - 1.0;

	Xa = 1;									// three axis aligned vectors
	Xb = 0;
	Xc = 0;

	Ya = 0;
	Yb = 1;
	Yc = 0;

	Za = 0;
	Zb = 0;
	Zc = 1;

	DBSTREAM3( cdebug << "r = " << r << "\tx = " << x << "\ty = " << y << "\tz = " << z << std::endl; )
	DBSTREAM3( cdebug << "		a = " << a << "	b = " << b << "	c = " << c << std::endl; )
	DBSTREAM3( cdebug << "		Xa = " << Xa << "	Xb = " << Xb << "	Xc = " << Xc << std::endl; )
	DBSTREAM3( cdebug << "		Ya = " << Ya << "	Yb = " << Yb << "	Yc = " << Yc << std::endl; )
	DBSTREAM3( cdebug << "		Za = " << Za << "	Zb = " << Zb << "	Zc = " << Zc << std::endl; )

	DBSTREAM3( cdebug << "rotate Z -> Y" << std::endl; )

	if ((z < 0.00001) && (z > -0.00001))
		r1 = 0;
	else
		r1 = atan2(z,y);

	x1 = x;
	y1 = y * cos(r1) + z * sin(r1);
	z1 = z * cos(r1) - y * sin(r1);
	a1 = a;
	b1 = b * cos(r1) + c * sin(r1);
	c1 = c * cos(r1) - b * sin(r1);
	Xa1 = Xa;
	Xb1 = Xb * cos(r1) + Xc * sin(r1);
	Xc1 = Xc * cos(r1) - Xb * sin(r1);
	Ya1 = Ya;
	Yb1 = Yb * cos(r1) + Yc * sin(r1);
	Yc1 = Yc * cos(r1) - Yb * sin(r1);
	Za1 = Za;
	Zb1 = Zb * cos(r1) + Zc * sin(r1);
	Zc1 = Zc * cos(r1) - Zb * sin(r1);
	DBSTREAM3( cdebug << "r1 = " << r1 << "\tx1 = " << x1 << "\ty1 = " << y1 << "\tz1 = " << z1 << std::endl; )
	DBSTREAM3( cdebug << "		a1 = " << a1 << "	b1 = " << b1 << "	c1 = " << c1 << std::endl; )
	DBSTREAM3( cdebug << "		Xa1 = " << Xa1 << "	Xb1 = " << Xb1 << "	Xc1 = " << Xc1 << std::endl; )
	DBSTREAM3( cdebug << "		Ya1 = "	<< Ya1 << " Yb1 = " << Ya1 << "	Yc1 = " << Yc1 << std::endl; )
	DBSTREAM3( cdebug << "		Za1 = " << Za1 << "	Zb1 = " << Zb1 << "	Zc1 = " << Zc1 << std::endl; )

	DBSTREAM3( cdebug << "rotate X -> Y" << std::endl; )
	if ((x1 < 0.00001) && (x1 > -0.00001))
		r2 = 0;
	else
		r2 = atan2(x1,y1);

	x2 = x1 * cos(r2) - y1 * sin(r2);
	y2 = y1 * cos(r2) + x1 * sin(r2);
	z2 = z1;
	a2 = a1 * cos(r2) - b1 * sin(r2);
	b2 = b1 * cos(r2) + a1 * sin(r2);
	c2 = c1;
	Xa2 = Xa1 * cos(r2) - Xb1 * sin(r2);
	Xb2 = Xb1 * cos(r2) + Xa1 * sin(r2);
	Xc2 = Xc1;
	Ya2 = Ya1 * cos(r2) - Yb1 * sin(r2);
	Yb2 = Yb1 * cos(r2) + Ya1 * sin(r2);
	Yc2 = Yc1;
	Za2 = Za1 * cos(r2) - Zb1 * sin(r2);
	Zb2 = Zb1 * cos(r2) + Za1 * sin(r2);
	Zc2 = Zc1;
	DBSTREAM3( cdebug << "r2 = " << r2 << "\tx2 = " << x2 << "\ty2 = " << y2 << "\tz2 = " << z2 << std::endl; )
	DBSTREAM3( cdebug << "		a2 = " << a2 << "	b2 = " << b2 << "	c2 = " << c2 << std::endl; )
	DBSTREAM3( cdebug << "		Xa2 = " << Xa2 << "	Xb2 = " << Xb2 << "	Xc2 = " << Xc2 << std::endl; )
	DBSTREAM3( cdebug << "		Ya2 = " << Ya2 << "	Yb2 = " << Yb2 << "	Yc2 = " << Yc2 << std::endl; )
	DBSTREAM3( cdebug << "		Za2 = " << Za2 << "	Zb2 = " << Zb2 << "	Zc2 = " << Zc2 << std::endl; )

	DBSTREAM3( cdebug << "rotate Z -> X (this is our initial input rotation)" << std::endl; )
	r3 = r;
	x3 = x2 * cos(r3) + z2 * sin(r3);
	y3 = y2;
	z3 = z2 * cos(r3) - x2 * sin(r3);
	a3 = a2 * cos(r3) + c2 * sin(r3);
	b3 = b2;
	c3 = c2 * cos(r3) - a2 * sin(r3);
	Xa3 = Xa2 * cos(r3) + Xc2 * sin(r3);
	Xb3 = Xb2;
	Xc3 = Xc2 * cos(r3) - Xa2 * sin(r3);
	Ya3 = Ya2 * cos(r3) + Yc2 * sin(r3);
	Yb3 = Yb2;
	Yc3 = Yc2 * cos(r3) - Ya2 * sin(r3);
	Za3 = Za2 * cos(r3) + Zc2 * sin(r3);
	Zb3 = Zb2;
	Zc3 = Zc2 * cos(r3) - Za2 * sin(r3);
	DBSTREAM3( cdebug << "r3 = " << r3 << "\tx3 = " << x3 << "\ty3 = " << y3 << "\tz3 = " << z3 << std::endl; )
	DBSTREAM3( cdebug << "		a3 = " << a3 << "	b3 = " << b3 << "	c3 = " << c3 << std::endl; )
	DBSTREAM3( cdebug << "		Xa3 = " << Xa3 << "	Xb3 = " << Xb3 << "	Xc3 = " << Xc3 << std::endl; )
	DBSTREAM3( cdebug << "		Ya3 = " << Xb3 << "	Yb3 = " << Yb3 << "	Yc3 = " << Yc3 << std::endl; )
	DBSTREAM3( cdebug << "		Za3 = " << Xc3 << "	Zb3 = " << Zb3 << "	Zc3 = " << Zc3 << std::endl; )

	DBSTREAM3( cdebug << "reverse rotate X -> Y\n" << std::endl; )
	r4 = -r2;
	x4 = x3 * cos(r4) - y3 * sin(r4);
	y4 = y3 * cos(r4) + x3 * sin(r4);
	z4 = z3;
	a4 = a3 * cos(r4) - b3 * sin(r4);
	b4 = b3 * cos(r4) + a3 * sin(r4);
	c4 = c3;
	Xa4 = Xa3 * cos(r4) - Xb3 * sin(r4);
	Xb4 = Xb3 * cos(r4) + Xa3 * sin(r4);
	Xc4 = Xc3;
	Ya4 = Ya3 * cos(r4) - Yb3 * sin(r4);
	Yb4 = Yb3 * cos(r4) + Ya3 * sin(r4);
	Yc4 = Yc3;
	Za4 = Za3 * cos(r4) - Zb3 * sin(r4);
	Zb4 = Zb3 * cos(r4) + Za3 * sin(r4);
	Zc4 = Zc3;
	DBSTREAM3( cdebug << "r4 = " << r4 << "\tx4 = " << y4 << "\ty4 = " << z4 << std::endl; )
	DBSTREAM3( cdebug << "		a4 = " << a4 << "	b4 = " << b4 << "	c4 = " << c4 << std::endl; )
	DBSTREAM3( cdebug << "		Xa4 = " << Xa4 << "	Xb4 = " << Xb4 << "	Xc4 = " << Xc4 << std::endl; )
	DBSTREAM3( cdebug << "		Ya4 = " << Ya4 << "	Yb4 = " << Yb4 << "	Yc4 = " << Yc4 << std::endl; )
	DBSTREAM3( cdebug << "		Za4 = " << Za4 << "	Zb4 = " << Zb4 << "	Zc4 = " << Zc4 << std::endl; )

	DBSTREAM3( cdebug << "reverse rotate Z -> Y" << std::endl; )
	r5 = -r1;
	x5 = x4;
	y5 = y4 * cos(r5) + z4 * sin(r5);
	z5 = z4 * cos(r5) - y4 * sin(r5);
	a5 = a4;
	b5 = b4 * cos(r5) + c4 * sin(r5);
	c5 = c4 * cos(r5) - b4 * sin(r5);
	Xa5 = Xa4;
	Xb5 = Xb4 * cos(r5) + Xc4 * sin(r5);
	Xc5 = Xc4 * cos(r5) - Xb4 * sin(r5);
	Ya5 = Ya4;
	Yb5 = Yb4 * cos(r5) + Yc4 * sin(r5);
	Yc5 = Yc4 * cos(r5) - Yb4 * sin(r5);
	Za5 = Za4;
	Zb5 = Zb4 * cos(r5) + Zc4 * sin(r5);
	Zc5 = Zc4 * cos(r5) - Zb4 * sin(r5);
	DBSTREAM3( cdebug << "r5 = " << r5 << "\tx5 = " << "\ty5 = " << y5 << "\tz5 = " << z5 << " (= x y z)" << std::endl; )
	DBSTREAM3( cdebug << "		a5 = " << a5 << "	b5 = " << b5 << "	c5 = " << c5 << std::endl; )
	DBSTREAM3( cdebug << "		Xa5 = " << Xa5 << "	Xb5 = " << Xb5 << "	Xc5 = " << Xc5 << std::endl; )
	DBSTREAM3( cdebug << "		Ya5 = " << Ya5 << "	Yb5 = " << Yb5 <<  "	Yc5 = " << Yc5 << std::endl; )
	DBSTREAM3( cdebug << "		Za5 = " << Za5 << "	Zb5 = " << Zb5 << "	Zc5 = " << Zc5 << std::endl; )

	DBSTREAM3( cdebug << "\nWe now have our rotated points." << std::endl; )
	DBSTREAM3( cdebug << "Now let's work backwards to get a X Y Z rotation...\n" << std::endl; )

	DBSTREAM3( cdebug << "Zrotate Y -> X to get unitX into XZ plane" << std::endl; )
//	if ((Xb5 < 0.00001) && (Xb5 > -0.00001))
//	{
//		Zr = 0.0;
//	}
//	else
//	{
//		Zr = GetAngle(Xb5,Xa5);
//	}
	Zr = SafeATan2(Xb5,Xa5);

	r6 = Zr;
	x6 = x5 * cos(r6) + y5 * sin(r6);
	y6 = y5 * cos(r6) - x5 * sin(r6);
	z6 = z5;
	a6 = a5 * cos(r6) + b5 * sin(r6);
	b6 = b5 * cos(r6) - a5 * sin(r6);
	c6 = c5;
	Xa6 = Xa5 * cos(r6) + Xb5 * sin(r6);
	Xb6 = Xb5 * cos(r6) - Xa5 * sin(r6);
	Xc6 = Xc5;
	Ya6 = Ya5 * cos(r6) + Yb5 * sin(r6);
	Yb6 = Yb5 * cos(r6) - Ya5 * sin(r6);
	Yc6 = Yc5;
	Za6 = Za5 * cos(r6) + Zb5 * sin(r6);
	Zb6 = Zb5 * cos(r6) - Za5 * sin(r6);
	Zc6 = Zc5;
	DBSTREAM3( cdebug << "Zr = " << Zr << std::endl; )
	DBSTREAM3( cdebug << "r6 = " << r6 << "\tx6 = " << x6 << "\ty6 = " << y6 << "\tz6 = " << z6 << std::endl; )
	DBSTREAM3( cdebug << "		a6 = " << a6 << "	b6 = " << b6 << "	c6 = " << c6 << std::endl; )
	DBSTREAM3( cdebug << "		Xa6 = " << Xa6 << "	Xb6 = " << Xb6 << "	Xc6 = " << Xc6 << std::endl; )
	DBSTREAM3( cdebug << "		Ya6 = " << Ya6 << "	Yb6 = " << Yb6 << "	Yc6 = " << Yc6 << std::endl; )
	DBSTREAM3( cdebug << "		Za6 = " << Za6 << "	Zb6 = " << Zb6 << "	Zc6 = " << Zc6 << std::endl; )

	DBSTREAM3( cdebug << "Yrotate Z -> X to get unitX into X axis" << std::endl; )
//	if ((Xc6 < 0.00001) && (Xc6 > -0.00001))
//	{
//		Yr = 0.0;
//	}
//	else
//	{
//		Yr = atan2(Xc6, Xa6);
//	}

	Yr = SafeATan2(Xc6,Xa6);

	r7 = Yr;
	x7 = x6 * cos(r7) + z6 * sin(r7);
	y7 = y6;
	z7 = z6 * cos(r7) - x6 * sin(r7);
	a7 = a6 * cos(r7) + c6 * sin(r7);
	b7 = b6;
	c7 = c6 * cos(r7) - a6 * sin(r7);
	Xa7 = Xa6 * cos(r7) + Xc6 * sin(r7);
	Xb7 = Xb6;
	Xc7 = Xc6 * cos(r7) - Xa6 * sin(r7);
	Ya7 = Ya6 * cos(r7) + Yc6 * sin(r7);
	Yb7 = Yb6;
	Yc7 = Yc6 * cos(r7) - Ya6 * sin(r7);
	Za7 = Za6 * cos(r7) + Zc6 * sin(r7);
	Zb7 = Zb6;
	Zc7 = Zc6 * cos(r7) - Za6 * sin(r7);
	DBSTREAM3( cdebug << "Yr = " << Yr << std::endl; )
	DBSTREAM3( cdebug << "r7 = " << r7 << "\tx7 = " << x7 << "\ty7 = " << y7 << "\tz7 = " << z7 << std::endl; )
	DBSTREAM3( cdebug << "		a7 = " << a7 << "	b7 = " << b7 << "	c7 = " << c7 << std::endl; )
	DBSTREAM3( cdebug << "		Xa7 = " << Xa7 << "	Xb7 = " << Xb7 << "	Xc7 = " << Xc7 << std::endl; )
	DBSTREAM3( cdebug << "		Ya7 = " << Ya7 << "	Yb7 = " << Yb7 << "	Yc7 = " << Yc7 << std::endl; )
	DBSTREAM3( cdebug << "		Za7 = " << Za7 << "	Zb7 = " << Zb7 << "	Zc7 = " << Zc7 << std::endl; )

	DBSTREAM3( cdebug << "Xrotate Z -> Y to get unitY and unitZ into axis" << std::endl; )
//	if ((Yc7 < 0.00001) && (Yc7 > -0.00001))
//	{
//		Xr = 0.0;
//	}
//	else
//	{
//		Xr = atan2(Yc7, Yb7);
//	}
	Xr = SafeATan2(Yc7,Yb7);
	r8 = Xr;
	x8 = x7;
	y8 = y7 * cos(r8) + z7 * sin(r8);
	z8 = z7 * cos(r8) - y7 * sin(r8);
	a8 = a7;
	b8 = b7 * cos(r8) + c7 * sin(r8);
	c8 = c7 * cos(r8) - b7 * sin(r8);
	Xa8 = Xa7;
	Xb8 = Xb7 * cos(r8) + Xc7 * sin(r8);
	Xc8 = Xc7 * cos(r8) - Xb7 * sin(r8);
	Ya8 = Ya7;
	Yb8 = Yb7 * cos(r8) + Yc7 * sin(r8);
	Yc8 = Yc7 * cos(r8) - Yb7 * sin(r8);
	Za8 = Za7;
	Zb8 = Zb7 * cos(r8) + Zc7 * sin(r8);
	Zc8 = Zc7 * cos(r8) - Zb7 * sin(r8);
	DBSTREAM3( cdebug << "Xr = " << Xr << std::endl; )
	DBSTREAM3( cdebug << "r8 = " << r8 << "\tx8 = " << x8 << "\ty8 = " << y8 << "\tz8 = " << z8 << std::endl; )
	DBSTREAM3( cdebug << "		a8 =  " << a8  << "	b8 =  " << b8  << "	c8 =  " << c8  << std::endl; )
	DBSTREAM3( cdebug << "		Xa8 = " << Xa8 << "	Xb8 = " << Xb8 << "	Xc8 = " << Xc8 << std::endl; )
	DBSTREAM3( cdebug << "		Ya8 = " << Ya8 << "	Yb8 = " << Yb8 << "	Yc8 = " << Yc8 << std::endl; )
	DBSTREAM3( cdebug << "		Za8 = " << Za8 << "	Zb8 = " << Zb8 << "	Zc8 = " << Zc8 << std::endl; )
	*newX = Xr;
	*newY = Yr;
	*newZ = Zr;
}

#if 0
void main(void)
{
	float	pi = acos(-1);
	float	x, y, z, r;
									// arbitrary vector and angle
	r = (4.0 * pi) / 3.0;					// -120 degrees
	x = y = z = sqrt(1.0 / 3.0);			// nake normalized 1,1,1 vector

	float newX,newY,newZ;
	Convert(x,y,z,r,&newX, &newY, &newZ);

	printf( "x = %f, y = %f, z = %f\n" , newX, newY, newZ);
}
#endif

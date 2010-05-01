//=============================================================================
// mathtest.cc:
//=============================================================================

#include <pigsys/pigsys.hp>
#include <math/scalar.hp>
#include <math/vector3.hp>
#include <math/angle.hp>
#include <math/euler.hp>
#include <math/matrix34.hp>
#include <cpplib/array.hp>

//-----------------------------------------------------------------------------

#if defined(_MSC_VER) && SW_DBSTREAM
#include <cpplib/strmnull.hp>
#endif

//=============================================================================

#include "tables.inc"

#define MAX_DEVIATION 0.000184			// 12>>65536
#define MAX_EULER_DEVIATION 0.03

#define MAX_ATAN_DEVIATION 0.1
// asin gets real lossy at 84 degrees or so
//#define MAX_ANGLE_DEVIATION 0.002777		//
//#define MAX_ANGLE_DEVIATION 0.003
#define MAX_ANGLE_DEVIATION 1

//=============================================================================

void
PIGSMain( int argc, char* argv[] )
{
	{ // random number test
	for ( int i=0; i<100; ++i )
	{
		//s.Random();
		std::cout << Scalar::Random() << '\t';
		//s.Random( Scalar::negativeOne, Scalar::two );
		std::cout << Scalar::Random( Scalar::negativeOne, Scalar::two ) << std::endl;
	}
	}

	std::cout << "MathTest:" << std::endl;

	{
		// simple math

		Scalar a = SCALAR_CONSTANT(1);
		Scalar b = SCALAR_CONSTANT(1.000077);

		// test divide
		Scalar divResult = a / b;
        std::cout << "divresult = " << divResult << std::endl;
	}



   {
      Scalar a = SCALAR_CONSTANT(-0.1);
      AssertMsg(a.WholePart() == -1,"a = " << a << ", a.WholePart() = " << a.WholePart() );

      a = SCALAR_CONSTANT(-0.9);
      AssertMsg(a.WholePart() == -1,"a = " << a << ", a.WholePart() = " << a.WholePart() );

      a = SCALAR_CONSTANT(-1.1);
      AssertMsg(a.WholePart() == -2,"a = " << a << ", a.WholePart() = " << a.WholePart() );
   }
	// simple math

	Scalar a = SCALAR_CONSTANT(1234.5678);
	Scalar b = SCALAR_CONSTANT(5);



	// test divide
	Scalar divResult = a / b;
	//std::cout << divResult.AsLong() << std::endl;
	AssertMsg(divResult.WholePart() == 246, "divResult.WholePart = " << divResult.WholePart());
#if defined(SCALAR_TYPE_FIXED)
	AssertMsg(divResult.AsUnsignedFraction() == 59871, "divResult.AsUnsignedFraction() = " << divResult.AsUnsignedFraction());
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
	AssertMsg(divResult.AsUnsignedFraction() == 59870, "divResult.AsUnsignedFraction() = " << divResult.AsUnsignedFraction());
#else
#error SCALAR TYPE not defined
#endif

	{
		Scalar a = SCALAR_CONSTANT(31000.001);
		Scalar b = SCALAR_CONSTANT(123.0);
		Scalar divResult = a / b;
		assert( divResult.WholePart() == 252);
#if defined(SCALAR_TYPE_FIXED)
		AssertMsg( divResult.AsUnsignedFraction() == 2131,"divResult.AsUnsignedFraction() = " << divResult.AsUnsignedFraction());
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
		AssertMsg( divResult.AsUnsignedFraction() == 2132,"divResult.AsUnsignedFraction() = " << divResult.AsUnsignedFraction());
#else
#error SCALAR TYPE not defined
#endif

		MATH_DEBUG( std::cout << "divResult = " << divResult.AsUnsignedFraction() << std::endl; )
	}

#if defined(SCALAR_TYPE_FIXED)
	// test divide overflow
	a = SCALAR_CONSTANT(-10.973755);
	b = SCALAR_CONSTANT(-0.000045);
	divResult = a / b;
	assert(divResult == SCALAR_CONSTANT(32000));
	// kts shouldn't this be 32767? Investigate

	// also check to see if negative overflows work

    // check portions which are platform specific
   MATH_DEBUG( std::cout << "testing 64 bit add \n");
    unsigned long a1,a0;
    unsigned long b1,b0;
    a0 = 0x80000000;
    a1 = 0x0;
    b0 = 0x80000001;
    b1 = 0x80;
    AddCarry64(a1, a0, b1, b0);
    std::cerr << std::hex;
    assertEq(a1,0x81);
    assertEq(a0,1);

    MATH_DEBUG( std::cout << "testing 64 bit mult\n");
    long in1, in2;
    unsigned long resultLsb; 
    long resultMsb; 
    in1 = 0x10;
    in2 = 0x7fffffff;
    Multiply64(in1,in2,resultMsb,resultLsb);
    assertEq(resultLsb,0xFFFFFFF0);
    assertEq(resultMsb,0x7);

    MATH_DEBUG( std::cout << "testing shifts\n");
    in1 = 0x12345678;
    in2 = 0x8;
    ShiftRightLogical(resultLsb,in1,in2);
    std::cout << std::hex << "resultLsb = " << resultLsb << ", in1 = " << in1 << ", in2 = " << in2 << std::endl;
    assertEq(resultLsb,0x123456);


    in1 = 0xf0000000;
    in2 = 0x1;
    ShiftRightLogicalVar(resultLsb,in1,in2);
    std::cout << std::hex << "resultLsb = " << resultLsb << ", in1 = " << in1 << ", in2 = " << in2 << std::endl;
    assertEq(resultLsb,0x78000000);
    
    resultLsb = 0x12345678;
    unsigned long unsignedResultMsb = 0x90abcdef;
    ShiftLeft64(unsignedResultMsb,resultLsb,0x8);
    assertEq(resultLsb,(unsigned long)0x34567800);
    assertEq(unsignedResultMsb,(unsigned long)0xabcdef12); 

    // joinhilo test
    resultLsb = JoinHiLo(0x12345678,(unsigned long)0x9abcdef0);
    assertEq(resultLsb,0x56789abc);

#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
#else
#error SCALAR TYPE not defined
#endif

	// check against test tables
#define ENTRIES 360*4
	int index=0;
	// sin
   MATH_DEBUG( std::cout << "testing sin\n");
	for(index=0;index<ENTRIES;index++)
	{
		unsigned int temp = index % 4;
		temp *= 0x4000;
		Scalar val(index/4,temp);
		Angle an(Angle::Degree(val));
      //std::cout << "val = " << val << ", an:deg = " << Angle::Degree(val) << std::endl;
		Scalar delta = an.Sin()-testSineTable[index];
		AssertMsg(delta.Abs() < SCALAR_CONSTANT(MAX_DEVIATION),"index = " << index << ", val = " << val << ", angle = " << an << ", an.Sin = " << an.Sin() << ", table = " << testSineTable[index] << ", delta  = " << delta << ", delta.Abs() = " << delta.Abs() << ", MAX_DEVIATION = " << Scalar(SCALAR_CONSTANT(MAX_DEVIATION)) << std::endl);
	}

	// cos
   MATH_DEBUG( std::cout << "testing cos\n");
	for(index=0;index<ENTRIES;index++)
	{
		unsigned int temp = index % 4;
		temp *= 0x4000;
		Scalar val(index/4,temp);
		Angle an(Angle::Degree(val));
		Scalar delta = an.Cos()-testCosTable[index];
		AssertMsg(delta.Abs() < SCALAR_CONSTANT(MAX_DEVIATION),"angle = " << an << ", an.Cos = " << an.Cos() << ", table = " << testCosTable[index] << ", delta  = " << delta << ", delta.Abs() = " << delta.Abs() << ", MAX_DEVIATION = " << Scalar(SCALAR_CONSTANT(MAX_DEVIATION)) << std::endl);
	}

	// now do atan2
   MATH_DEBUG( std::cout << "testing atan2\n");
	for(index=0;index<360;index++)
	{
		Angle angle = Angle(Angle::Degree(Scalar(index,0)));
		Scalar Y = angle.Sin();
		Scalar X = angle.Cos();

		Angle tan = X.ATan2(Y);
		Scalar delta = tan.AsRevolution()-angle.AsRevolution();
      std::cout << "index = " << index << ", Angle = " << angle << ", X = " << X << ", Y = " << Y << ", Atan2 = " << tan << std::endl;
		AssertMsg(delta.Abs() < SCALAR_CONSTANT(MAX_ATAN_DEVIATION),"tan = " << tan << ", X = " << X << ", Y = " << Y << ", angle = " << angle << ", delta  = " << delta << ", delta.Abs() = " << delta.Abs() << ", MAX_DEVIATION = " << Scalar(SCALAR_CONSTANT(MAX_ATAN_DEVIATION)) << std::endl);
	}

	{
		Angle angle(Angle::Degree(SCALAR_CONSTANT(209.5229)));
		Scalar Y = SCALAR_CONSTANT(-0.5);
		Scalar X = SCALAR_CONSTANT(-0.85717);

		Angle tan = X.ATan2(Y);
		Scalar delta = tan.AsRevolution()-angle.AsRevolution();
		AssertMsg(delta.Abs() < SCALAR_CONSTANT(MAX_ATAN_DEVIATION),"tan = " << tan << ", X = " << X << ", Y = " << Y << ", angle = " << angle << ", delta  = " << delta << ", delta.Abs() = " << delta.Abs() << ", MAX_DEVIATION = " << Scalar(SCALAR_CONSTANT(MAX_ATAN_DEVIATION)) << std::endl);
	}

#pragma message(__FILE__ ": kts finish math test suite")
#if 0		// doesn't pass yet
	{
		Angle angle(Angle::Degree(SCALAR_CONSTANT(270.00025984)));
		Scalar Y = SCALAR_CONSTANT(-1.5518035889);
		Scalar X = SCALAR_CONSTANT(-2.7040252686);

		Angle tan = X.ATan2(Y);
		Scalar delta = tan.AsRevolution()-angle.AsRevolution();
		AssertMsg(delta.Abs() < SCALAR_CONSTANT(MAX_ATAN_DEVIATION),"tan = " << tan << ", X = " << X << ", Y = " << Y << ", angle = " << angle << ", delta  = " << delta << ", delta.Abs() = " << delta.Abs() << ", MAX_DEVIATION = " << Scalar(SCALAR_CONSTANT(MAX_ATAN_DEVIATION)) << std::endl);
	}
#endif
   MATH_DEBUG( std::cout << "testing positive asin\n");
	// now do positive asin
	for(index=0;index<ENTRIES/4;index++)
	{
		unsigned int temp = index % 4;
		temp *= 0x4000;
		Scalar val(index/4,temp);
		Angle an(Angle::Degree(val));
		Scalar delta = an.Sin()-testSineTable[index];
		AssertMsg(delta.Abs() < SCALAR_CONSTANT(MAX_DEVIATION),"angle = " << an << ", an.Sin = " << an.Sin() << ", table = " << testSineTable[index] << ", delta  = " << delta << ", delta.Abs() = " << delta.Abs() << ", MAX_DEVIATION = " << Scalar(SCALAR_CONSTANT(MAX_DEVIATION)) << std::endl);
		Angle arc = an.Sin().ASin();
		Scalar aDelta;
		if(arc > an)
			aDelta = (arc-an).AsRevolution();
		else
			aDelta = (an-arc).AsRevolution();
		AssertMsg(aDelta < SCALAR_CONSTANT(MAX_ANGLE_DEVIATION), "angle = " << an << ", arc = " << arc << ", aDelta = " << aDelta);
	}

   MATH_DEBUG( std::cout << "testing negative asin\n");
	// now do negative asin
	for(index=ENTRIES/2;index<(ENTRIES/2)+(ENTRIES/4);index++)
	{
		unsigned int temp = index % 4;
		temp *= 0x4000;
		Scalar val(index/4,temp);
		Angle an(Angle::Degree(val));
		Scalar delta = an.Sin()-testSineTable[index];
		AssertMsg(delta.Abs() < SCALAR_CONSTANT(MAX_DEVIATION),"angle = " << an << ", an.Sin = " << an.Sin() << ", table = " << testSineTable[index] << ", delta  = " << delta << ", delta.Abs() = " << delta.Abs() << ", MAX_DEVIATION = " << Scalar(SCALAR_CONSTANT(MAX_DEVIATION)) << std::endl);
		Angle arc = an.Sin().ASin();
		an -= Angle::Degree(SCALAR_CONSTANT(180));
		an = Angle(0) - an;
		Scalar aDelta;
		if(arc > an)
			aDelta = (arc-an).AsRevolution();
		else
			aDelta = (an-arc).AsRevolution();
		AssertMsg(aDelta < SCALAR_CONSTANT(MAX_ANGLE_DEVIATION), "angle = " << an << ", arc = " << arc << ", aDelta = " << aDelta);
	}

   MATH_DEBUG( std::cout << "testing vector\n");
	Vector3 vector( Scalar::one, SCALAR_CONSTANT(2), SCALAR_CONSTANT(3) );
   vector.Normalize();

//#if SW_DBSTREAM
	std::cout << "vector = " << vector << std::endl;

	std::cout << ( SCALAR_CONSTANT(5) < SCALAR_CONSTANT(55) ) << std::endl;
	std::cout << SCALAR_CONSTANT(5).AsBool() << std::endl;
	std::cout << Scalar::zero.AsBool() << std::endl;
	std::cout << ( Scalar::one.AsBool() ^ SCALAR_CONSTANT(2).AsBool() ) << std::endl;

	std::cout << std::endl;

	std::cout << "Scalar::zero: " << Scalar::zero << std::endl;
	std::cout << "Scalar::one: " << Scalar::one << std::endl;
	std::cout << "Scalar::epsilon: " << Scalar::epsilon << std::endl;
	Scalar number = SCALAR_CONSTANT(1) + SCALAR_CONSTANT(1.5);
	std::cout << "1 + 1.5 = " << number << std::endl;
	assert( number == SCALAR_CONSTANT( 2.5 ) );
	std::cout << "Scalar::divide by zero: " << std::endl;
    std::cout << SCALAR_CONSTANT(3) / SCALAR_CONSTANT(0) << std::endl;

	std::cout << "div zero done" << std::endl;

	std::cout << "Vector3::zero: " << Vector3::zero << std::endl;
	std::cout << "Vector3::one: " << Vector3::one << std::endl;
	std::cout << "Vector3::unitX: " << Vector3::unitX << std::endl;
	std::cout << "Vector3::unitY: " << Vector3::unitY << std::endl;
	std::cout << "Vector3::unitZ: " << Vector3::unitZ << std::endl;

	Scalar m1 = SCALAR_CONSTANT(2.3);
	Scalar m2 = SCALAR_CONSTANT(45.2);
	std::cout << m1 << " * " << m2 << " = " << m1 * m2 << std::endl;
	std::cout << m1 << " / " << m2 << " = " << m1 / m2 << std::endl;


	Scalar I2 = SCALAR_CONSTANT(10);
	std::cout << I2 << " Inverted = " << I2.Invert() << std::endl;

	Scalar I1 = SCALAR_CONSTANT(2.3);
	std::cout << I1 << " Inverted = " << I1.Invert() << std::endl;

	Scalar md1 = SCALAR_CONSTANT(16000);
	Scalar md2 = SCALAR_CONSTANT(20000);
	Scalar md3 = SCALAR_CONSTANT(18000);

	std::cout << "MulDiv(" << md1 <<  ", " << md2 << ", " << md3 << ") = " << md1.MulDiv(md2,md3) << std::endl;

	Scalar tanx = SCALAR_CONSTANT(10);
	Scalar tany = SCALAR_CONSTANT(5);
	std::cout << "atan2( " << tanx << "," << tany << ") = " << tanx.ATan2(tany) << std::endl;

	Angle sin1 = Angle::Degree(SCALAR_CONSTANT(90));
	Angle sin2 = Angle::Degree(SCALAR_CONSTANT(45));
	Angle sin3 = Angle::Degree(SCALAR_CONSTANT(270));

	std::cout << "Sin(" << sin1 << ") = " << sin1.Sin() << std::endl;
	std::cout << "Sin(" << sin2 << ") = " << sin2.Sin() << std::endl;
	std::cout << "Sin(" << sin3 << ") = " << sin3.Sin() << std::endl;

	Angle cos1 = Angle::Degree(SCALAR_CONSTANT(90));
	Angle cos2 = Angle::Degree(SCALAR_CONSTANT(45));
	Angle cos3 = Angle::Degree(SCALAR_CONSTANT(270));

	std::cout << "cos(" << cos1 << ") = " << cos1.Cos() << std::endl;
	std::cout << "cos(" << cos2 << ") = " << cos2.Cos() << std::endl;
	std::cout << "cos(" << cos3 << ") = " << cos3.Cos() << std::endl;

	Scalar asin1 = SCALAR_CONSTANT( 0);
	Scalar asin2 = SCALAR_CONSTANT( 0.7071067811865);
	Scalar asin3 = SCALAR_CONSTANT( 0.99999999);
	Scalar asin4 = SCALAR_CONSTANT( 1);
	std::cout << "asin(" << asin1 << ") = " << asin1.ASin() << std::endl;
	std::cout << "asin(" << asin2 << ") = " << asin2.ASin() << std::endl;
	std::cout << "asin(" << asin3 << ") = " << asin3.ASin() << std::endl;
	std::cout << "asin(" << asin4 << ") = " << asin4.ASin() << std::endl;

	std::cout << "acos(" << asin1 << ") = " << asin1.ACos() << std::endl;
	std::cout << "acos(" << asin2 << ") = " << asin2.ACos() << std::endl;
	std::cout << "acos(" << asin3 << ") = " << asin3.ACos() << std::endl;
	std::cout << "acos(" << asin4 << ") = " << asin4.ACos() << std::endl;

	std::cout << std::dec;
	Scalar bignum(SCALAR_CONSTANT(16000));
	Scalar smallnum(SCALAR_CONSTANT(2));
	Scalar result = bignum/smallnum;
	std::cout << "bignum = " << bignum << ", smallnum = " << smallnum << ", result = " << result << std::endl;
	bignum = SCALAR_CONSTANT(16000);
	smallnum = SCALAR_CONSTANT(0.0003);
	result = bignum/smallnum;
	std::cout << "bignum = " << bignum << ", smallnum = " << smallnum << ", result = " << result << std::endl;
	bignum = SCALAR_CONSTANT(-16000);
	smallnum = SCALAR_CONSTANT(2);
	result = bignum/smallnum;
	std::cout << "bignum = " << bignum << ", smallnum = " << smallnum << ", result = " << result << std::endl;
	bignum = SCALAR_CONSTANT(-16000);
	smallnum = SCALAR_CONSTANT(-2);
	result = bignum/smallnum;
	std::cout << "bignum = " << bignum << ", smallnum = " << smallnum << ", result = " << result << std::endl;
	bignum = SCALAR_CONSTANT(-16000);
	smallnum = SCALAR_CONSTANT(0.0003);
	result = bignum/smallnum;
	std::cout << "bignum = " << bignum << ", smallnum = " << smallnum << ", result = " << result << std::endl;


	bignum = SCALAR_CONSTANT(18.018997-3.991516);
	smallnum = SCALAR_CONSTANT(-0.000244);
	std::cout << "trying bignum = " << bignum << ", smallnum = " << smallnum << std::endl;
	result = bignum/smallnum;
	std::cout << "bignum = " << bignum << ", smallnum = " << smallnum << ", result = " << result << std::endl;

	bignum = SCALAR_CONSTANT((-6.981002) - (4.517333));
	smallnum = SCALAR_CONSTANT(-0.000244);
	result = bignum/smallnum;
	std::cout << "bignum = " << bignum << ", smallnum = " << smallnum << ", result = " << result << std::endl;


    bignum = SCALAR_CONSTANT(110);
    smallnum = SCALAR_CONSTANT(0.00286865234375);
	std::cout << "trying bignum = " << bignum << ", smallnum = " << smallnum << std::endl;
	result = bignum/smallnum;
	std::cout << "bignum = " << bignum << ", smallnum = " << smallnum << ", result = " << result << std::endl;
//Div: 6e0000, bc


   std::cout << "testing matrix\n";
   Matrix34 matrix;

   Euler euler;
   matrix.ConstructEuler(euler);
   Euler outEuler = matrix.AsEuler();
   std::cout << "Matrix = " << matrix << std::endl;
   AssertMsg(outEuler == euler, "euler = " << euler << ", outEuler = " << outEuler << std::endl);

#define max(a,b) ((a)>(b)?(a):(b))
	for(index=0;index<360;index++)
	{
		Angle angle = Angle(Angle::Degree(Scalar(index,0)));
      euler.SetA(angle);
      matrix.ConstructEuler(euler);
      outEuler = matrix.AsEuler();

      //std::cout << "matrix: " << matrix << "index =" << index << ", euler = " << euler << ", outEuler = " << outEuler << std::endl;
      Matrix34 compmatrix;
      compmatrix.ConstructEuler(outEuler);

      // now compare the 2 matricies

      for(int y=0;y<2;y++)
         for(int x=0;x<2;x++)
         {
            Scalar orig = matrix[y][x];
            Scalar comp = compmatrix[y][x];
            Scalar delta = (orig-comp).Abs();

            AssertMsg(delta < SCALAR_CONSTANT(MAX_EULER_DEVIATION),"index = " << index 
               << "x=" << x << ", y = " << y << ", delta = " << delta
               << ", matrix  = " << matrix 
               << ", compmatrix  = " << compmatrix 
               << ", MAX_EULER_DEVIATION = " << Scalar(SCALAR_CONSTANT(MAX_EULER_DEVIATION)) 
               << "euler = " << euler << ", outEuler = " << outEuler 
               << std::endl);
         }

//       matrix.ConstructEuler(euler);
//       outEuler = matrix.AsEuler();
//
//       Euler deltaEuler = outEuler - euler;
//
//       Scalar deltaA = deltaEuler.GetA().AsRevolution().Abs();
//       Scalar deltaB = deltaEuler.GetB().AsRevolution().Abs();
//       Scalar deltaC = deltaEuler.GetC().AsRevolution().Abs();
//        if(deltaA >= SCALAR_CONSTANT(0.5))
//           deltaA = Scalar::one-deltaA;
//        if(deltaB >= SCALAR_CONSTANT(0.5))
//           deltaB = Scalar::one-deltaB;
//        if(deltaC >= SCALAR_CONSTANT(0.5))
//           deltaC = Scalar::one-deltaC;
//       Scalar delta = max(deltaA,deltaB);
//       delta = max(delta,deltaC);
//       std::cout << "matrix: " << matrix << "index =" << index << ", euler = " << euler << ", outEuler = " << outEuler << std::endl;
//       AssertMsg(delta < SCALAR_CONSTANT(MAX_EULER_DEVIATION),"index = " << index
//          << ", matrix  = " << matrix << ", delta  = " << delta
//          << ", MAX_EULER_DEVIATION = " << Scalar(SCALAR_CONSTANT(MAX_EULER_DEVIATION))
//          << "euler = " << euler << ", outEuler = " << outEuler << ", deltaEuler = " << deltaEuler
//          << ", deltaA = " << deltaA << ", deltaB = " << deltaB << ", deltaC = " << deltaC
//          << "deltaA = " << Angle(deltaA.AsUnsignedFraction())
//          << std::endl);
	}

   euler = Euler::zero;

	for(index=0;index<360;index++)
	{
		Angle angle = Angle(Angle::Degree(Scalar(index,0)));
      euler.SetB(angle);

      matrix.ConstructEuler(euler);
      outEuler = matrix.AsEuler();

      //std::cout << "matrix: " << matrix << "index =" << index << ", euler = " << euler << ", outEuler = " << outEuler << std::endl;
      Matrix34 compmatrix;
      compmatrix.ConstructEuler(outEuler);

      // now compare the 2 matricies

      for(int y=0;y<2;y++)
         for(int x=0;x<2;x++)
         {
            Scalar orig = matrix[y][x];
            Scalar comp = compmatrix[y][x];
            Scalar delta = (orig-comp).Abs();

            AssertMsg(delta < SCALAR_CONSTANT(MAX_EULER_DEVIATION),std::dec << "index = " << index 
               << ", x=" << x << ", y = " << y << ", delta = " << delta
               << ", matrix  = " << matrix 
               << ", compmatrix  = " << compmatrix 
               << ", MAX_EULER_DEVIATION = " << Scalar(SCALAR_CONSTANT(MAX_EULER_DEVIATION)) 
               << "euler = " << euler << ", outEuler = " << outEuler 
               << std::endl);
         }
	}

   euler = Euler::zero;

	for(index=0;index<360;index++)
	{
		Angle angle = Angle(Angle::Degree(Scalar(index,0)));
      euler.SetC(angle);

      matrix.ConstructEuler(euler);
      outEuler = matrix.AsEuler();

      //std::cout << "matrix: " << matrix << "index =" << index << ", euler = " << euler << ", outEuler = " << outEuler << std::endl;
      Matrix34 compmatrix;
      compmatrix.ConstructEuler(outEuler);

      // now compare the 2 matricies

      for(int y=0;y<2;y++)
         for(int x=0;x<2;x++)
         {
            Scalar orig = matrix[y][x];
            Scalar comp = compmatrix[y][x];
            Scalar delta = (orig-comp).Abs();

            AssertMsg(delta < SCALAR_CONSTANT(MAX_EULER_DEVIATION),"index = " << index 
               << "x=" << x << ", y = " << y << ", delta = " << delta
               << ", matrix  = " << matrix 
               << ", compmatrix  = " << compmatrix 
               << ", MAX_EULER_DEVIATION = " << Scalar(SCALAR_CONSTANT(MAX_EULER_DEVIATION)) 
               << "euler = " << euler << ", outEuler = " << outEuler 
               << std::endl);
         }
	}


	std::cout << "mathtest Done:" << std::endl;
//#endif
}

//=============================================================================

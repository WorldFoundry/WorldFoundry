//=============================================================================
// cpplib/linux/scalar.cc: linux specific scalar code
//=============================================================================


//=============================================================================
// result(a1:a0) = a2:a1/a0

void 
UnSignedDivide64(long __a2,long& __a1, long& __a0)
{
    unsigned long __total_hi;
	unsigned long __total_lo;
	unsigned long __counter_hi;
	unsigned long __counter_lo;
	unsigned long __t5;
	unsigned long __t4;
	unsigned long __t3;
	unsigned long __t2;

   MATH_DEBUG( cout << "UnsignedDivide64 called with " << __a2 << "," << __a1 << ", " << __a0 << endl; )

	__total_hi = __total_lo = 0;

	if(__a0 != 0)
	{
		__counter_hi = 0;
		__counter_lo = 1;

		__t3 = __a2;
		__t2 = __a1;	// t3:t2 = a1:a0

		__t4 = __a0;
		__t5 = 0;	// t5:t4 = 0:a0

//		DivideLoop(__t3,__t2,__t5,__t4,__counter_hi,__counter_lo,__total_hi,__total_lo);
    	long __v1;
		long __v0;

		MATH_DEBUG( cout << "UnSignedSubDivide: called with " << __a2 << __a1 << __a0 << endl; )
		MATH_DEBUG( cout << "__t2 = " << __t2 << ", __t3 = " << __t3 << "__t4 = " << __t4 << endl; )
		for(;;)
		{
			__v1 = __t3;
			__v0 = __t2;
			MATH_DEBUG( cout << "before SubCarry64: " << __v1 << ", " << __v0 << ", " << __t5 << ", " << __t4 << endl; )
			SubCarry64(__v1,__v0,__t5,__t4);
			MATH_DEBUG( cout << "after: " << __v1 << ", " << __v0 << ", " << __t5 << ", " << __t4 << endl; )

			if(__v1 & 0x80000000)
				break;

         MATH_DEBUG( cout << "before shiftleft:" << __t5 << ", " << __t4 << endl; )
			ShiftLeft64(__t5,__t4,1);
         MATH_DEBUG( cout << "before shiftleft counter:" << __counter_hi << ", " << __counter_lo << endl; )
			ShiftLeft64(__counter_hi,__counter_lo,1);
		}

      MATH_DEBUG( cout << "before inbetween shiftright:" << __t5 << ", " << __t4 << endl; )
		ShiftRight64(__t5,__t4,1);
      MATH_DEBUG( cout << "after inbetween shiftright:" << __t5 << ", " << __t4 << endl; )
      MATH_DEBUG( cout << "before shiftright counter:" << __counter_hi << ", " << __counter_lo << endl; )
		ShiftRight64(__counter_hi,__counter_lo,1);
      MATH_DEBUG( cout << "after shiftright counter:" << __counter_hi << ", " << __counter_lo << endl; )

		for(;;)
		{
	    	if(__counter_hi+__counter_lo == 0)
				break;

			__v1 = __t3;
			__v0 = __t2;
			MATH_DEBUG( cout << "before SubCarry64: " << __v1 << ", " << __v0 << ", " << __t5 << ", " << __t4 << endl; )
			SubCarry64(__v1,__v0,__t5,__t4);
			MATH_DEBUG( cout << "after: " << __v1 << ", " << __v0 << ", " << __t5 << ", " << __t4 << endl; )

			if(!(__v1 & 0x80000000))
			{
		    	__t3 = __v1;
				__t2 = __v0;

            MATH_DEBUG( cout << "before AddCarry64: " << __total_hi << ", " << __total_lo << ", " << __counter_hi << ", " << __counter_lo << endl; )
				AddCarry64(__total_hi,__total_lo,__counter_hi,__counter_lo);
            MATH_DEBUG( cout << "after AddCarry64: " << __total_hi << ", " << __total_lo << endl; )
			}
         MATH_DEBUG( cout << "before shiftright:" << __t5 << ", " << __t4 << endl; )
			ShiftRight64(__t5,__t4,1);
         MATH_DEBUG( cout << "before shiftright counter:" << __counter_hi << ", " << __counter_lo << endl; )
			ShiftRight64(__counter_hi,__counter_lo,1);
		}
	}
	__a1 = __total_hi;
	__a0 = __total_lo;

   MATH_DEBUG( cout << "UnsignedDIvide64 returning __a1 = " << __a1 << ", __a0 =" << __a0 << endl; )
}

//-----------------------------------------------------------------------------
// result(a1:a0) = a2:a1/a0

void 
SignedDivide64(long __a2,long& __a1,long& __a0)
{
   MATH_DEBUG( cout << "SignedDivide64 called with " << __a2 << ", " << __a1 << ", " << __a0 << endl; )

    int __sign_1,__sign_0;
	long __v1;
	long __v0;

	if(!(__a2 & 0x80000000))
		__sign_1 = 0;
	else
	{
	    __sign_1 = 1;
		__v1 = __v0 = 0;
      MATH_DEBUG( cout << "SignedDivide64 before SubCarry64: __v1 = " << __v1 << ", __v0 = " << __v0 << ", __a2 = " << __a2 << ", __a1 = " << __a1 << endl; )
		SubCarry64(__v1,__v0,__a2,__a1);
      MATH_DEBUG( cout << "after SubCarry64: __v1 = " << __v1 << ", __v0 = " << __v0 << endl; )

		__a2 = __v1;
		__a1 = __v0;
	}

	if(__a0 >= 0)
		__sign_0 = 0;
	else
	{
	    __sign_0 = 1;
		__a0 = (signed long)(- __a0);
	}

	__sign_1 ^= __sign_0;	// sign of result

	UnSignedDivide64(__a2,__a1,__a0);

	if(__sign_1)
	{

	    __v1 = __v0 = 0;
       MATH_DEBUG( cout << "SignedDivide64 before SubCarry64: __v1 = " << __v1 << ", __v0 = " << __v0 << ", __a2 = " << __a2 << ", __a1 = " << __a1 << endl; )
		SubCarry64(__v1,__v0,__a1,__a0);
      MATH_DEBUG( cout << "after SubCarry64: __v1 = " << __v1 << ", __v0 = " << __v0 << endl; )
		__a1 = __v1;
		__a0 = __v0;
	}
}



// result in1Hi:in1Lo = in1Hi:in2Lo - in2Hi:in2Lo
//=============================================================================
#if 0
Scalar&
Scalar::operator /= ( const Scalar& b )
{
   	long a2;
	long a1;
	long a0 = b._value;

	// result = a/b

	ShiftRightArithmetic(a2,_value,16);
	a1 = _value << 16;

	SignedDivide64(a2,a1,a0);

//	printf("a0 = %lx, a1 = %lx\n",a0,a1);
	if(
		((a1 == 0 && ((a0 & 0x80000000) == 0 )) ||
		(a1 == 0xffffffff && ((a0 & 0x80000000)==0x80000000)))
		&& b._value != 0
	 )
		_value = a0;
	else
		_value = 32000<<16;
	return *this;
}

//Scalar&
//Scalar::operator /= ( const Scalar& b )
//{
//	_value = BR_DIV( _value, b._value );
//	return *this;
//}

//=============================================================================

Scalar
Scalar::Invert( void ) const
{
	long a2;
	long a1;
	long a0 = _value;

	// result = 1.0/a
	a2 = 1;
	a1 = 0;

	SignedDivide64(a2,a1,a0);
	return Scalar(a0);
}

// ------------------------------------------------------------------------

Scalar
Scalar::MulDiv(const Scalar b, const Scalar c) const
{
   	long out2;
	long out1;
	long out0;

	// result = a*b/c
	MultiplyAndNop64(_value,b._value,out2,out1);

	out0 = c._value;
	SignedDivide64(out2,out1,out0);
	return Scalar(out0);
}
#endif

//=============================================================================


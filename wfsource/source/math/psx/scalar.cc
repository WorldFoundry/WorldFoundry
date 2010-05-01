//=============================================================================
// cpplib/psx/scalar.cc: psx specific scalar code
//=============================================================================

#include <hal/hal.h>
// result in1Hi:in1Lo = in1Hi:in2Lo - in2Hi:in2Lo

#define SubCarry(in1Hi, in1Lo, in2Hi, in2Lo)				\
	({								\
		register long carry;					\
		register long resultHi;					\
		register long resultLo;					\
	    								\
		asm (							\
			"sltu	%2,%4,%6\n"				\
			"subu	%0,%3,%5\n"				\
			"subu	%0,%2\n"				\
			"subu	%1,%4,%6\n"				\
									\
		: "=&r" (resultHi), "=r" (resultLo), "=&r" (carry) 				\
		: "r" (in1Hi), "r" (in1Lo), "r" (in2Hi), "r" (in2Lo)\
		);							\
									\
		in1Hi = resultHi;						\
		in1Lo = resultLo;						\
	})

//=============================================================================

INLINE
void UnSignedDivide64(unsigned long __a2,unsigned long& __a1, unsigned long& __a0)
{
    unsigned long __total_hi;
	unsigned long __total_lo;
	unsigned long __counter_hi;
	unsigned long __counter_lo;
	unsigned long __t5;
	unsigned long __t4;
	unsigned long __t3;
	unsigned long __t2;

	// result = a2:a1/a0

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
    	register long __v1;
		register long __v0;

//		cout << "UnSignedSubDivide: called with " << __a2 << __a1 << __a0 << endl;
//		cout << "__t2 = " << __t2 << ", __t3 = " << __t3 << "__t4 = " << __t4 << endl;
		for(;;)
		{
			__v1 = __t3;
			__v0 = __t2;
//			cout << "before: " << __v1 << ", " << __v0 << ", " << __t5 << ", " << __t4 << endl;
			SubCarry(__v1,__v0,__t5,__t4);
//			cout << "after: " << __v1 << ", " << __v0 << ", " << __t5 << ", " << __t4 << endl;

			if(__v1 & 0x80000000)
				break;

			ShiftLeft64(__t5,__t4,1);
			ShiftLeft64(__counter_hi,__counter_lo,1);
		}

		ShiftRight64(__t5,__t4,1);
		ShiftRight64(__counter_hi,__counter_lo,1);

		for(;;)
		{
	    	if(__counter_hi+__counter_lo == 0)
				break;

			__v1 = __t3;
			__v0 = __t2;
			SubCarry(__v1,__v0,__t5,__t4);

			if(!(__v1 & 0x80000000))
			{
		    	__t3 = __v1;
				__t2 = __v0;

				AddCarry64(__total_hi,__total_lo,__counter_hi,__counter_lo);
			}
			ShiftRight64(__t5,__t4,1);
			ShiftRight64(__counter_hi,__counter_lo,1);
		}
	}
	__a1 = __total_hi;
	__a0 = __total_lo;
}

//-----------------------------------------------------------------------------

INLINE
void SignedDivide64(long __a2,long& __a1,long& __a0)
{
    int __sign_1,__sign_0;
	register long __v1;
	register long __v0;

	if(!(__a2 & 0x80000000))
		__sign_1 = 0;
	else
	{
	    __sign_1 = 1;
		__v1 = __v0 = 0;
		SubCarry(__v1,__v0,__a2,__a1);
		__a2 = __v1;
		__a1 = __v0;
	}

	if(__a0 >= 0) __sign_0 = 0;
	else
	{
	    __sign_0 = 1;
		__a0 = (signed long)(- __a0);
	}

	__sign_1 ^= __sign_0;	// sign of result

	unsigned long in1 = __a2;
	unsigned long in2 = __a1;
	unsigned long in3 = __a0;
	UnSignedDivide64(in1,in2,in3);
	__a2 = in1;
	__a1 = in2;
	__a0 = in3;

	if(__sign_1)
	{
	    __v1 = __v0 = 0;
		SubCarry(__v1,__v0,__a1,__a0);
		__a1 = __v1;
		__a0 = __v0;
	}
}

//=============================================================================
#if 0
static inline int
ScalarLeadingZeroCount( Scalar a )
{
	int lzc;

	gte_Lzc( (long)a, &lzc );

	return lzc;
}

//-----------------------------------------------------------------------------

static inline Scalar
ScalarDiv( Scalar a, Scalar b )
{
	int bNegative, lzcN, dLeft;
	uint32 nn, dd, qr;
	Scalar n, d;

	ASSERT( b != 0 );

	bNegative = ( a & 0x80000000 ) ^ ( b & 0x80000000 );
	n = ScalarAbs( a );
	d = ScalarAbs( b );

	lzcN = ScalarLeadingZeroCount( n );

	// make n as large as possible
	dLeft = lzcN - 12;
	if( dLeft < 0 )
	{	// large n, need to shift d right to multiply n by 1.0
		int lzcD, pastPoint, dPre;

		nn = n << lzcN;

		lzcD = ScalarLeadingZeroCount( d );
		pastPoint = lzcD - lzcN;
		if( pastPoint >= 0 )
		{
			dPre = 12 - lzcD;
			if( dPre >= 0 )
				dd = d >> dPre;
			else
				dd = d << -dPre;

			ASSERT( dd != 0 );
			qr = nn / dd;

			qr = qr << pastPoint;
		}
		else
		{
			dd = d >> -dLeft;

			ASSERT( dd != 0 );
			qr = nn / dd;
		}
	}
	else
	{	// for small n we can safely multiply by 1.0 by a left shift of n by 12

		nn = n << 12;
		qr = nn / d;
	}

	if( bNegative )
		return -qr;
	else
		return qr;
}
#endif

//=============================================================================

#if 0
#if DO_ASSERTIONS
SlowScalarDivideEquals(Scalar& a, const Scalar& B)
{
//#if 0
//#else
   	long a2;
	long a1;
	long a0 = b._value;

	// result = a/b
	ShiftRightArithmetic(a2,_value,16);
	a1 = _value << 16;

	// a1:a0 = a2::a1 / a0
	SignedDivide64(a2,a1,a0);

//	printf("a0 = %lx, a1 = %lx\n",a0,a1);
	if(
		(
			(a1 == 0 && ((a0 & 0x80000000) == 0 )) ||
			(a1 == 0xffffffff && ((a0 & 0x80000000)==0x80000000))
		)
		&& b._value != 0
	 )
		_value = a0;
	else
	{
		if(a1 < 0)
			_value = -32000<<16;
		else
			_value = 32000<<16;
	}
//#endif
	long delta = abs(_value-foo);

	AssertMsg(delta <= 1, "orig = " << orig << ",b._value = " << b._value << ",foo = " << foo << ", _value = " << _value);
	if(delta > 1)
	{
		char buffer[200];

		sprintf(buffer, "Scalar Divide: orig = %ld, b._value = %ld,foo = %ld, _value = %ld" , orig, b._value, foo , _value);
		FatalError(buffer);
	}
}
#endif
#endif
//=============================================================================
// kts improved 7/13/98 9:36PM
// ab / cd

#define longabs(a) (a>=0?a:-a)

Scalar&
Scalar::operator /= ( const Scalar& b )
{
	long orig = _value;
	bool bNegative = ( _value & 0x80000000 ) ^ ( b._value & 0x80000000 );
	register unsigned long dividend = longabs( _value );
	register unsigned long divisor = longabs( b._value );

	register long temp;
	register long temp2;
	register long temp3;
	register long temp4;
	asm (
		"div	%5,%6\n"                // ab / cd
		// kts code to determine if we are dividing by zero, if so, return 32000<<16
		// since this occurs while waiting for the divide, it is free
		"lui	%0,0x7d00\n"
		"beq	$0,%6,divbyzero\n"
		"addu	%2,%6,$0\n"             // copy %6 into %2

		"mfhi	%1\n"					// get remainder into temp
		"mtc2	%1, $30\n"				// write to gte lzc register
		"mflo	%3\n"			        // get result of divide
		"srl	%4,%3,15\n"				// test if any upper bits are set, if so, overflow
		"bne	%4,$0,overflow\n"		// do overflow, %0 already contains 0x7d00
		"nop\n"
		"sll	%0,%3,16\n"				// shift up into upper 16 bits
		"mfc2	%3,$31\n"				// get result of lzc
		"subu	%4,%3,16\n"				// check if larger than 16
		"bltz	%4,shiftnottoobig\n"
		"nop\n"
		"add	%3,$0,16\n"				// clip to 16 bits
"shiftnottoobig:"
		"sllv	%1,%1,%3\n"
		"add	%4,$0,16\n"
		"subu	%3,%4,%3\n"				// invert: 16 - shift value
		"srlv	%2,%2,%3\n"
		"divu	%1,%2\n"
		"mflo	%1\n"					// get fraction
		"add	%0,%1,%0\n"				// accumulate final result
"overflow:\n"							// maximum value
"divbyzero:\n"
	: "=r" (_value), "=r" (temp), "=r" (temp2), "r=" (temp3), "r=" (temp4)
	: "r" (dividend), "r" (divisor)
	);

	if( bNegative )
		_value =  -_value;

	return *this;
}

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

//=============================================================================

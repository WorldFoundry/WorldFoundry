//=============================================================================
// cpplib/win/scalar.cc: intel specifc scalar code
//=============================================================================

long __upperResult;

long WFShiftRightArithmatic(long input,char amount);
#pragma aux WFShiftRightArithmatic = 	\
	"sar	eax,cl"							\
	parm nomemory [eax] [cl] 				\
	value [eax] 							\
	modify nomemory [edx] ;

#define ShiftRightArithmetic(__sra1,__sra0,__sran) (__sra1 = WFShiftRightArithmatic(__sra0,__sran))


long WFSubCarry(long aa1,long aa0, long bb1, long bb0);
#pragma aux WFSubCarry = 	\
	"sub	eax,ecx"							\
	"sbb	ebx,edx"							\
	"mov	__upperResult,ebx"				\
	parm [ebx] [eax] [edx] [ecx] 				\
	modify [ecx edx]        			\
	value [eax];


// aa1:aa0 = aa1:aa0 + bb1:bb0
#define SubCarry(__aa1, __aa0, __bb1, __bb0)		\
		__aa0 = WFSubCarry(__aa1,__aa0,__bb1,__bb0);	\
		__aa1 = __upperResult

//=============================================================================

INLINE
void UnSignedDivide64(long __a2,long& __a1, long& __a0)
{
    unsigned long __total_hi;
	unsigned long __total_lo;
	register unsigned long __counter_hi;
	register unsigned long __counter_lo;
	register unsigned long __t5;
	register unsigned long __t4;
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

		MATH_DEBUG( cout << "UnSignedSubDivide: called with " << __a2 << __a1 << __a0 << endl; )
		MATH_DEBUG( cout << "__t2 = " << __t2 << ", __t3 = " << __t3 << "__t4 = " << __t4 << endl; )
		for(;;)
		{
			__v1 = __t3;
			__v0 = __t2;
			MATH_DEBUG( cout << "before: " << __v1 << ", " << __v0 << ", " << __t5 << ", " << __t4 << endl; )
			SubCarry(__v1,__v0,__t5,__t4);
			MATH_DEBUG( cout << "after: " << __v1 << ", " << __v0 << ", " << __t5 << ", " << __t4 << endl; )

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
		SubCarry(__v1,__v0,__a1,__a0);
		__a1 = __v1;
		__a0 = __v0;
	}
}

//=============================================================================

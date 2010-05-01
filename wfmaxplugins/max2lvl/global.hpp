//============================================================================
// global.hp: for levelcon, all levelcon files include this
//============================================================================

// Copyright(c) 1995 Cave Logic Studios / PF.Magic
// By Kevin T. Seghetti
//============================================================================

// use only once insurance
#ifndef _GLOBAL_HPP
#define _GLOBAL_HPP

#if 1
typedef struct
{
	float a;
	float b;
	float c;
} Euler;
#include <../lib/global.hp>
#include <stdstrm.hp>
// This is a replacement (in progress) for br_euler.  The ordering is always XYZ_S
#else

//#ifdef _DEBUG
#define DEBUG 1
//#endif

#if DEBUG > 0
#define SW_DBSTREAM 3
#else
#define SW_DBSTREAM 0
#endif


#define OLD_IOSTREAMS 1


#pragma warning( disable : 4786 )
const char eos = '\0';


#define NOMINMAX		// Turn off windows.h definition of min() and max(),
						// use the one from STL instead

typedef unsigned long ulong;

#include <stl/algobase.h>
#include <stl/bstring.h>
#include <max.h>
#include <istdplug.h>
#include "stdstrm.hpp"
#include "dbstrm.hpp"
#include "pigtool.h"

// This is a replacement (in progress) for br_euler.  The ordering is always XYZ_S
typedef struct
{
	float a;
	float b;
	float c;
} Euler;

#include <iostream.h>
#include <strstrea.h>
#include <assert.h>

//============================================================================
// kts this macro is used to convert floats into velocity compatible scalars

#define WF_ONE_LS 	(1<<16)
#define WF_FLOAT_TO_SCALAR(f) ((fixed32)((f)*(float)WF_ONE_LS))
#define WF_SCALAR_TO_FLOAT(s)	((s)/(float)WF_ONE_LS)
#define WF_INT_TO_SCALAR(i)	((fixed32)((i)*(int)WF_ONE_LS))

//============================================================================
// Exception handling class
class LVLExporterException
{
public:
	long	errorCode;
};

//============================================================================
// copy of parts of the microsoft assert macro

extern void LevelconAssert(void *, void *, unsigned);

#include <assert.h>
#undef	assert
#ifdef NDEBUG
#define assert(exp)	((void)0)
#else
//#define assert(exp) (void)( (exp) || (_assert(#exp, __FILE__, __LINE__), 0) )
#define assert(exp) (void)( (exp) || (LevelconAssert(#exp, __FILE__, __LINE__), 0) )
#endif	/* NDEBUG */

//============================================================================

#define AssertMessageBox(__cond__, __msg__)\
	do{\
		if (!(__cond__))\
		{\
			char messageBuffer[1024];\
			strstream outputStream(messageBuffer,1024,ios::out|ios::binary);\
			outputStream << __msg__ << eos; \
			cfatal << "Assertion failed: " << __msg__ << eos;\
			MessageBox(gMaxInterface->GetMAXHWnd(), outputStream.str(), "Level Exporter Error", MB_OK);\
			throw LVLExporterException();\
		}\
	} while ( 0 )


#define AssertMsg AssertMessageBox

//============================================================================

// MAX's Point3 class doesn't come with ostream operators...
ostream& operator<<(ostream& s, const Point3 &o);
ostream& operator<<(ostream& s, const Matrix3 &o);


#endif
//============================================================================
#endif

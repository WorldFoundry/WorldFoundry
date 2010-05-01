//=============================================================================
// ktstoken.cpp: extensions to the rouge-wave RWCToken class
// By Kevin T. Seghetti
// Copyright (c) 1995 Cave Logic Studios, Ltd
// based on code
// (c) Copyright 1989, 1990, 1991, 1992, 1993, 1994 Rogue Wave Software, Inc.
// ALL RIGHTS RESERVED
//=============================================================================

#include "global.hp"
#include <string>
#include <cpplib/stdstrm.hp>
#include "ktstoken.hp"
#if !defined(USE_OLD_IOSTREAMS)
using namespace std;
#endif

//=============================================================================
// return sub-string which is rest of string

string
ktsRWCTokenizer::restOfString()
{
	DBSTREAM4( cdebug << "ktsRWCTokenizer::restOfString" << endl; )
	size_t start = offset;
	if(theString.length() == 0)
		return(string(""));
	string retVal = theString.substr(start, theString.length()-start);
	DBSTREAM4( cdebug << "ktsRWCTokenizer::restOfString done" << endl; )
	return retVal;
}

//=============================================================================

ktsRWCTokenizer::ktsRWCTokenizer(const string& s) : theString(s), offset(0)
{
	DBSTREAM2( cdebug << "ktsRWCTokenizer::constructor" << endl; )
}

//=============================================================================

ktsRWCTokenizer&
ktsRWCTokenizer::operator+=(int i)
{
	offset += i;
	if(offset >= theString.length())
		offset = theString.length();	// cannot go off end of string
	return *this;
}

//=============================================================================

size_t
ktsstrcspn(const char* s1, const char* s2)
{
	size_t count = 0;
	const char* inter;

	while(*s1)
	 {
		inter = s2;

		while(*inter)
		 {
			if(*inter == *s1)
				return(count);
			inter++;
		 }
		count++;
		s1++;
	 }
	return(count);
}

//=============================================================================

size_t
strcspnparen(const char* s1, const char* s2)
{
	size_t count = 0;
	long parenCount = 0;
	const char* inter;

//	DBSTREAM2( cdebug << "strcspnparen called with <" << s1 << ">" << endl; )
	while(*s1)
	 {
//		DBSTREAM2( cdebug << "ParenCount = " << parenCount << endl; )
		if(*s1 == '(')
			parenCount++;

		if(parenCount == 0)
		 {
			inter = s2;
			while(*inter)
			 {
				if(*inter == *s1)
					return(count);
				inter++;
			 }
		 }
		else if(*s1 == ')')
			parenCount--;

		assert(parenCount >= 0);

		count++;
		s1++;
	 }
	return(count);
}

//=============================================================================

string
ktsRWCTokenizer::operator()(const string& delimiters,const string& whiteSpace)
{
	size_t extent = 0;
	if(offset >= theString.length())
		return(string(""));
	offset += strspn(theString.c_str()+offset, whiteSpace.c_str());					// eat leading white space
	extent = strcspn(theString.c_str()+offset, delimiters.c_str());					// scan to next whitespace
	if (!extent)
	{
		++offset; // skip null
		if (offset >= theString.length())
			return(string(""));
		offset += strspn(theString.c_str()+offset, whiteSpace.c_str());						// eat leading white space
		extent = strcspn(theString.c_str()+offset, delimiters.c_str());					// scan to next whitespace
	}

	size_t start = offset;
	offset += extent;		// Advance the placeholder

	string retVal(theString.substr(start, extent));
	return(retVal);
}

//=============================================================================

string
ktsRWCTokenizer::NextTrackParens(const string& ws)
{
	assert( ws[0]!=0 );

	size_t extent = 0;
	while (1)
	 {
		if (offset >= theString.length())
			return(string(""));
		offset += strspn(theString.c_str()+offset, ws.c_str());						// eat leading white space
		extent = strcspnparen(theString.c_str()+offset, ws.c_str());					// scan to next whitespace
		if (extent)
			break;
		++offset; // skip null
	 }
	size_t start = offset;
	offset += extent;		// Advance the placeholder

	string retVal(theString.substr(start, extent));
	return retVal;
}

//=============================================================================

char
ktsRWCTokenizer::ReadChar(int additionalOffset)
{
	if ((offset+additionalOffset) >= theString.length())
		return(0);
	return(theString[offset+additionalOffset]);
}

//=============================================================================

char
ktsRWCTokenizer::GetNextChar()
{
	if (offset >= theString.length())
		return(0);
	return(theString[offset++]);
}

//=============================================================================
// returns true if next character matches list

int
ktsRWCTokenizer::MatchNextChar(const string& ws)
{
	if (offset >= theString.length())
		return false;
	if(strspn(theString.c_str()+offset,ws.c_str()) == 0)
		return(false);
	return(true);
}

//=============================================================================
// should only be called after a matching token is found

string
ktsRWCTokenizer::GetWhiteSpace(const string& ws)
{
	size_t extent = 0;

	if (offset >= theString.length())
		return string("");
	offset += strcspn(theString.c_str()+offset,ws.c_str());			// skip any non-white space characters
	extent = strspn(theString.c_str()+offset,ws.c_str());

	size_t start = offset;
	offset += extent;		// Advance the placeholder
	string retVal(theString.substr(start, extent) );
	return(retVal);
}

//=============================================================================

string
ktsRWCTokenizer::operator()()
{
	return operator()(" \t\n"," \t\n");
}

//=============================================================================

//============================================================================
// qstring.hp:
// Copyright(c) 1996 Cave Logic Studios / PF.Magic
//============================================================================
/* Documentation:

	Abstract:
			A subset of the draft standard String class with better performance
			for use within Levelcon.  (Note:  'QString' stands for QuickString;
			it does NOT indicate that this is a new 'qwass' definition!)
	History:
			Created	14:02 11-18-96 Phil Torre

	Class Hierarchy:
			none

*/
//============================================================================

// use only once insurance
#ifndef _qSTRING_HP
#define _qSTRING_HP

#include "global.hpp"
#include <malloc.h>

#define NOCOPY false	// for passing to const char* ctor (optional)

//============================================================================

#define NPOS NULL	// this class doesn't know about iterators

class QString
{
public:
	QString::QString();
	QString::QString(const QString& other);
	QString::QString(const char* theString, bool copy=true);
	QString::~QString();

	QString::operator=(const QString& other);
	QString::operator=(const char* other);
	friend ostream& operator<<(ostream& s, const QString& theQString) { s << theQString.buffer; return (s); }
	int operator==(const QString& other) const;
	int operator!=(const QString& other) const;
	const QString& QString::operator+=(const QString& other);
	const QString& QString::operator+=(const char* other);
	int QString::length() const { return strlen(buffer); }
	const char* QString::c_str() const { return buffer; }
	const char* QString::find(const char* searchString, int pos = 0) const;
	const char* QString::find(const char& searchString, int pos = 0) const;
	QString QString::substr(int first, int last) const;
	QString QString::tolower() const;



private:
	char*	buffer;
	bool	ownBuffer;
};

//============================================================================


#endif	/* _qSTRING_HP */

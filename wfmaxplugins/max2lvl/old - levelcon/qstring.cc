//============================================================================
// qstring.cc:
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

#include "global.hp"
#include "qstring.hp"
#include <string.h>
#include <ctype.h>

//============================================================================

QString::QString()
{
	DBSTREAM4( cdebug << "*** null ctor on object at " << hex << (long)this << dec << endl; )
	buffer = NULL;
	DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")   Setting buffer to " << hex << (long)buffer << dec << endl; )
	ownBuffer = false;
}

QString::QString(const QString& other)
{
	buffer = NULL;
	ownBuffer = false;
	*this = other;
}

QString::QString(const char* theString, bool copy)
{
	DBSTREAM4( cdebug << "*** (const char*) ctor on object at " << hex << (long)this << dec << endl; )
	if (copy)
	{
		buffer = (char*)malloc(strlen(theString)+1);
		DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")          Allocating " << hex << (long)buffer << dec << endl; )
		strcpy(buffer, theString);
		ownBuffer = true;
	}
	else
	{
		buffer = (char*) theString;
		DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")   Setting buffer to " << hex << (long)buffer << dec << " in const char* ctor" << endl; )
	}
}

QString::~QString()
{
	if (ownBuffer)
	{
		DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")             Freeing " << hex << (long)buffer << dec << " in dtor" << endl; )
		free(buffer);
	}
}


QString::operator=(const QString& other)
{
	if (ownBuffer)
	{
		DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")             Freeing " << hex << (long)buffer << dec << " in =(const QString&)" << endl; )
		free((void*)buffer);
	}

	if (other.ownBuffer)
	{
		buffer = (char*)malloc(strlen(other.buffer)+1);
		DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")          Allocating " << hex << (long)buffer << dec << " in =(const QString&)" << endl; )
		strcpy(buffer, other.buffer);
		ownBuffer = true;
	}
	else
	{
		buffer = other.buffer;
		DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")   Setting buffer to " << hex << (long)buffer << dec << " in =(const QString&)" << endl; )
		ownBuffer = false;
	}
}

QString::operator=(const char* other)
{
	if (ownBuffer)
	{
		DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")             Freeing " << hex << (long)buffer << dec << " in =(const char*)" << endl; )
		free((void*)buffer);
	}

	buffer = (char*) malloc(strlen(other)+1);
	DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")          Allocating " << hex << (long)buffer << dec << " in =(const char*)" << endl; )
	strcpy(buffer, other);
	ownBuffer = true;
}

int
QString::operator==(const QString& other) const
{
	return (!strcmp(buffer, other.buffer));
}

int
QString::operator!=(const QString& other) const
{
	return (strcmp(buffer, other.buffer));
}

const QString&
QString::operator+=(const QString& other)
{
	char* newBuffer = (char*)malloc(strlen(buffer) + strlen(other.buffer) + 1);
	DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")          Allocating " << hex << (long)newBuffer << dec << " in +=(const QString&)" << endl; )
	strcpy(newBuffer, buffer);
	strcat(newBuffer, other.buffer);
	DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")             Freeing " << hex << (long)buffer << dec << " in +=(const QString&)" << endl; )
	free(buffer);
	buffer = newBuffer;
	DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")   Setting buffer to " << hex << (long)buffer << dec << " in +=(const QString&)" << endl; )
	ownBuffer = true;
	return *this;
}

const QString&
QString::operator+=(const char* other)
{
	char* newBuffer = (char*)malloc(strlen(buffer) + strlen(other) + 1);
	DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")          Allocating " << hex << (long)newBuffer << dec << " in +=(const char*)" << endl; )
	strcpy(newBuffer, buffer);
	strcat(newBuffer, other);
	DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")             Freeing " << hex << (long)buffer << dec << " in +=(const char*)" << endl; )
	free(buffer);
	buffer = newBuffer;
	DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")   Setting buffer to " << hex << (long)buffer << dec << " in +=(const char*)" << endl; )
	ownBuffer = true;
	return *this;
}

const char*
QString::find(const char* searchString, int pos) const
{
	return (strstr(buffer + pos, searchString));
}

const char*
QString::find(const char& searchString, int pos) const
{
	return (strstr(buffer + pos, &searchString));
}

QString
QString::substr(int first, int last) const
{
	if ( (first > strlen(buffer)-1) || (last > strlen(buffer)-1) )
		return NULL;

	char* temp = (char*)malloc((last - first) + 2);
	DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")          Allocating " << hex << (long)temp << dec << " in substr()" << endl; )
	strncpy(temp, buffer+first, (last-first)+1);
	return QString(temp);
}

QString
QString::tolower() const
{
	int size = strlen(buffer);
	char* temp = (char*)malloc(size+1);
	DBSTREAM4( cdebug << "QString (" << hex << (long)this << dec << ")          Allocating " << hex << (long)temp << dec << " in tolower()" << endl; )
	strcpy(temp, buffer);
	strlwr(temp);
	return QString(temp);
}




//============================================================================
//=============================================================================
// istream.h
// Copyright ( c ) 1998,99 World Foundry Group  
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org

//=============================================================================
// Description:
//		Here is my first stab at a simple ostream (kts)
//
// Original Author: Brad McKee
// Since then: Kevin T. Seghetti
// ------------------------------------------------------------------------

#ifndef _ISTREAM_H
#define _ISTREAM_H

//=============================================================================

class StreamInput
{
public:
	inline StreamInput() { useCount = 0; }
	virtual ~StreamInput();
	int UseCount() { return(useCount); }
	void IncUseCount() { useCount++; }
	void DecUseCount() { useCount--; }

	virtual void InputChars(char* string,int len) = 0;
	void Validate() const;
private:
	int useCount;
};

// ------------------------------------------------------------------------
// Warning: This class never closes the filehandles it points to; it is
// one big file resource leak.

class istream : public ios
{
  public:
	istream( StreamInput& input  );
	istream( istream& other  );
	virtual ~istream( void );

//	istream& operator >> ( char val );
//	istream& operator >> ( short val );
	istream& operator >> ( long val );
//	istream& operator >> ( int val );
//	istream& operator >> ( unsigned char val );
//	istream& operator >> ( unsigned short val );
//	istream& operator >> ( unsigned long val );
//	istream& operator >> ( unsigned int val );
//	istream& operator >> ( char * val );
//	istream& operator >> ( void * val);

	istream& getline(         char *, int maxLength, char delimiter='\n');
//    inline istream& getline(unsigned char *, int maxLength, char delimiter='\n');
//    inline istream& getline(  signed char *, int maxLength, char delimiter='\n');

	istream& operator = ( const istream& other );

	inline iostate rdstate( void ) { return ios::goodbit; }
	bool good( void ) { return rdstate() == ios::goodbit; }
	void Validate() const;

  protected:
	StreamInput* _input;

  private:
	int get(char* buffer, int size, char delim);
	istream();			 // must construct with a StreamOutput
};

//=============================================================================
#include "istream.hpi"
//=============================================================================
#endif
//=============================================================================


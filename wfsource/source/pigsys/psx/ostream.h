//=============================================================================
// ostream.h
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

#ifndef _OSTREAM_H
#define _OSTREAM_H

// ------------------------------------------------------------------------
// Class Declarations
// ------------------------------------------------------------------------

// throw away class
class ostreamModifier
{
#ifdef __PSX__
	int	fake;				// gnu sucks
#endif
};


// throw away class, used to detect end of stream
class stream_endl
{
#ifdef __PSX__
	int	fake;				// gnu sucks
#endif
};

extern stream_endl endl;

// ------------------------------------------------------------------------

class StreamOutput
{
public:
	StreamOutput() { useCount = 0; }
	virtual ~StreamOutput();
	int UseCount() { return(useCount); }
	void IncUseCount() { useCount++; }
	void DecUseCount() { useCount--; }

	virtual void OutputChars(const char* string,int len) = 0;
	virtual void Flush() {}
private:
	int useCount;
};

// ------------------------------------------------------------------------
// Warning: This class never closes the filehandles it points to; it is
// one big file resource leak.

class ostream : public ios
{
  public:
	enum
	{
		longest_printf = 4096
	};

	ostream( StreamOutput& output  );
	ostream( ostream& other  );
	virtual ~ostream( void );

	ostream& operator << ( const char val );
	ostream& operator << ( const short val );
	ostream& operator << ( const long val );
	ostream& operator << ( const int val );
	ostream& operator << ( const unsigned char val );
	ostream& operator << ( const unsigned short val );
	ostream& operator << ( const unsigned long val );
	ostream& operator << ( const unsigned int val );
	ostream& operator << ( const char * val );
	ostream& operator << ( const void * val);

	ostream& operator << ( const stream_endl&);

	ostream& operator << ( const ostreamModifier val );

	ostream& operator << ( ostream& (*f)( ostream& ) );

	ostream& operator = ( const ostream& other );

	inline ostream& dec();
	inline ostream& hex();

	iostate rdstate( void ) { return ios::goodbit; }
	bool good( void ) { return rdstate() == ios::goodbit; }

	void Validate() const;

  protected:
	StreamOutput* _output;

  private:
	ostream();		  	// must construct with a StreamInput
	char* _PrintNumBase(char* buffer, int bufferLen, unsigned int val, int base);
};


// ------------------------------------------------------------------------
// Globals
// ------------------------------------------------------------------------

extern ostream cnull;
extern ostream cout;
extern ostream cerr;
extern ostream cscreen;
extern ostream cscreenflush;

//=============================================================================
#endif
//=============================================================================

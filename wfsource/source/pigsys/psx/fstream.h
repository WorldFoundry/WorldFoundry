//=============================================================================
// fstream.h:
// Copyright ( c ) 1994,95,96,97,99 World Foundry Group  
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
//		A dummy output file stream class to hide the absence of such
//		a class on the PSX.  It should be replaced by a true
//		implementation.
// Original Author: Brad McKee
// ------------------------------------------------------------------------

#ifndef _FSTREAM_H
#define _FSTREAM_H

//=============================================================================

//#include "anmswtch.hp"

// all related classes have been moved to iostream.*
#include "iostream.h"
#include "_iostr.hp"

// ------------------------------------------------------------------------
// Warning: This class never closes the filehandles it points to; it is
// one big file resource leak.

class ifstream : public istream
{
  public:
	ifstream( const char* filename );
	inline iostate rdstate( void ) { return ios::goodbit; }
	bool good( void ) { return rdstate() == ios::goodbit; }
	void Validate() const;
protected:
private:
	ifstream();				// must construct with a filename
};

// ------------------------------------------------------------------------
// Warning: This class never closes the filehandles it points to; it is
// one big file resource leak.

class ofstream : public ostream
{
  public:
	ofstream( const char* filename );
	void Validate() const;
private:
	ofstream();				// must construct with a filename
};

//=============================================================================
#endif // _FSTREAM_H
//=============================================================================

//=============================================================================
// ios.h
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
//=============================================================================

class ios
{
  public:
	ios::ios();
    enum io_state
	{                     				// Error state
        goodbit = 0x00,                 // - no errors
        badbit  = 0x01,                 // - operation failed, may not proceed
        failbit = 0x02,                 // - operation failed, may proceed
        eofbit  = 0x04                  // - end of file
    };
    typedef int iostate;

    enum
	{
//		skipws     = 0x0001,
//	    left       = 0x0002,
//	    right      = 0x0004,
//	    internal   = 0x0008,
	    dec        = 0x0010,
	    oct        = 0x0020,
	    hex        = 0x0040,
//	    showbase   = 0x0080,
//	    showpoint  = 0x0100,
//	    uppercase  = 0x0200,
//	    showpos    = 0x0400,
//	    scientific = 0x0800,
//	    fixed      = 0x1000,
//	    unitbuf    = 0x2000,
//	    stdio      = 0x4000
		basefield = dec | oct | hex
	 };

	inline long flags() const;
	inline long setf(long bits_to_set);
	inline long setf(long bits_to_set, long mask);
	inline long unsetf(long bits_to_clear);

    inline int width() const;
    inline int width(int _i);

    inline char fill() const;
    inline char fill(char _c);

	virtual ~ios( void ) { };
	void Validate() const;

protected:
	long	x_flags;
    char    x_fill;
    int     x_width;

	int GetBase();
};


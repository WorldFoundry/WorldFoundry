//=============================================================================
// math.cc:
// Copyright ( c ) 1997,1998,1999,2000 World Foundry Group  
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

// ===========================================================================
// Description: The Display class encapsulates data and behavior for a single
//	 hardware screen
// Original Author: Kevin T. Seghetti
//============================================================================
//=============================================================================

#include <gfx/math.hp>

//=============================================================================

#if DO_IOSTREAMS
#if defined(RENDERER_PIPELINE_PSX)

#if 0
void
PrintFixed412(ostream& out, int16 value)
{
	int16 temp;
	if (value < 0)
	{
		temp = ((~value)+1);
		out << "-";
	}
	else
		temp = value;

	// dump out the top 4 bits (whole part)
	out << ( temp >> 12);

	// get the bottom 12 bits (fractional part)
	uint16 frac = uint16(temp) & 0xfff;
	if( frac != 0 )
	{
		out << '.';

		assert((frac & 0xf000) == 0);

		int digits = 0;
		const int max_scalar_fractional_digits = 6;
		while( frac && digits < max_scalar_fractional_digits )
		{
			frac *= 10;
			int digit = frac >> 12;
			out << digit;

			frac = frac & 0xfff;
			++digits;
		}
	}
//	out << "(" << value << ")";
}
#endif

//=============================================================================

#if 0
ostream& operator<< (ostream& out, const Fixed412 num)
{
	PrintFixed412(out,num._val);
	return out;
}
#endif

//=============================================================================

#if 0
ostream& Vector3_PS::PrintAs412(ostream& out) const
{
	Fixed412 x(X());
	Fixed412 y(Y());
	Fixed412 z(Z());
	out << x << ", " << y << ", " << z;
	return out;
}
#endif
#endif      // RENDERER_PIPELINE_PSX
#endif		// DO_IOSTREAMS

//=============================================================================

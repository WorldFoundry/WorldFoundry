//=============================================================================
// math/psx/ScalarPS.cc: Playstation/gte specific Scalars
// Copyright (c) 1997,98,99 World Foundry Group  
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

// Orignal Author: Kevin T. Seghetti
//=============================================================================
#if DO_IOSTREAMS
//=============================================================================

void
PrintFixed16(ostream& out, int16 value)
{
	int32 temp;
	if (value < 0)
	{
		temp = ((~(int32)value)+1);
		out << "-";
	}
	else
		temp = value;

	// dump out the top 16 bits (whole part)
	out << ( temp >> 8);

	// get the bottom 8 bits (fractional part)
	uint16 frac = uint16(temp) & 0xff;
	if( frac != 0 )
	{
		out << '.';

		assert((frac & 0xff00) == 0);

		int digits = 0;
		const int max_scalar_fractional_digits = 6;
		while( frac && digits < max_scalar_fractional_digits )
		{
			frac *= 10;
			int digit = frac >> 8;
			out << digit;

			frac = frac & 0xff;
			++digits;
		}
	}
//	out << "(" << value << ")";
}

//=============================================================================

ostream& operator<< (ostream& out, const Fixed16 num)
{
	PrintFixed16(out,num._val);
	return out;
}

//=============================================================================
#endif
//=============================================================================

//==============================================================================
// scalar.cc:
// Copyright (c) 1996,1997,1998,1999,2000,2001,2002 World Foundry Group   
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

//==============================================================================

//#include "anmswtch.hp"
#define _SCALAR_CC
#include <math/scalar.hp>
//#include <visual/brstrm.hp>
#include <math/angle.hp>

// ------------------------------------------------------------------------

#if defined(SCALAR_TYPE_FIXED)
#if defined ( __PSX__ )
#include <math/psx/scalar.cc>
#elif defined ( __WIN__ )
#include <math/win/scalar.cc>
#elif defined ( __LINUX__ )
#include <math/linux/scalar.cc>
#else
#error no scalar asm code!
#endif
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
#else
#error SCALAR TYPE not defined
#endif

// ------------------------------------------------------------------------
// static data members
// ------------------------------------------------------------------------

const Scalar Scalar::zero = SCALAR_CONSTANT( 0.0 );
const Scalar Scalar::one = SCALAR_CONSTANT( 1.0 );
const Scalar Scalar::negativeOne = SCALAR_CONSTANT( -1.0 );
const Scalar Scalar::two = SCALAR_CONSTANT( 2.0 );
const Scalar Scalar::half = SCALAR_CONSTANT( 0.5 );
const Scalar Scalar::epsilon(0,1);


#pragma message( __FILE__ ": better Scalar min and max definitions" )
const Scalar Scalar::min( -32767, 0 );
const Scalar Scalar::max( 32767, 0 );


//==============================================================================

void
Scalar::AsText(char* buffer, int length)
{
   Validate();

   assert(ValidPtr(buffer));
   assert(length > 0);

#if defined(SCALAR_TYPE_FIXED)

	Scalar temp;
	if (*this < Scalar::zero)
	{
		temp._value = ((~_value)+1);
      if(length > 0)
      {
         *buffer++ = '-';
         length--;
      }
	}
	else
		temp = *this;

	// dump out the top 16 bits (whole part)
   if(length > 0)
   {
      int offset = snprintf(buffer,length,"%d",temp.WholePart());
      buffer += offset;
      length -= offset;
   }

	// get the bottom 16 bits (fractional part)
	Scalar frac = Scalar((long)temp.AsUnsignedFraction());					// mask off whole part
	if( frac != Scalar::zero )
	{
      if(length > 0)
      {
         *buffer++ = '.';
         length--;
      }

		assert(frac.WholePart() == 0);

		int digits = 0;
		const int max_scalar_fractional_digits = 6;
		while( frac._value && digits < max_scalar_fractional_digits )
		{
			frac *= SCALAR_CONSTANT(10);
			int digit = frac.WholePart();
         if(length > 0)
         {
            *buffer++ = '0'+digit;
            length--;
         }

			frac = Scalar((long)frac.AsUnsignedFraction());
			assert(frac.WholePart() == 0);

			++digits;
		}
	}
   *buffer = 0;
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
   snprintf(buffer,length,"%f",_value);

   int pos = strlen(buffer)-1;
   // delete trailing 0's and decimal if no fractional part
   while(pos > 0 && (buffer[pos] == '0'))
   {
      buffer[pos] = 0;        
      pos--;
   }
   if(buffer[pos] == '.')
      buffer[pos] = 0;        
#else
#error SCALAR TYPE not defined
#endif
}

// ------------------------------------------------------------------------
// IOStream Support
// ------------------------------------------------------------------------

#if DO_IOSTREAMS
std::ostream&
operator << ( std::ostream& s, const Scalar& q )
{
   q.Validate();
#if defined(SCALAR_TYPE_FIXED)

	Scalar temp;
	if (q < Scalar::zero)
	{
		temp._value = ((~q._value)+1);
		s << "-";
	}
	else
		temp = q;

	// dump out the top 16 bits (whole part)
	s << temp.WholePart();

	// get the bottom 16 bits (fractional part)
	Scalar frac = Scalar((long)temp.AsUnsignedFraction());					// mask off whole part
	if( frac != Scalar::zero )
	{
		s << '.';

		assert(frac.WholePart() == 0);

		int digits = 0;
		const int max_scalar_fractional_digits = 6;
		while( frac._value && digits < max_scalar_fractional_digits )
		{
			frac *= SCALAR_CONSTANT(10);
			int digit = frac.WholePart();
			s << digit;

			frac = Scalar((long)frac.AsUnsignedFraction());
			assert(frac.WholePart() == 0);

			++digits;
		}
	}
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
   s << std::setprecision(10) << q._value;
#else
#error SCALAR TYPE not defined
#endif
	return s;
}

#endif // DO_IOSTREAMS

// ------------------------------------------------------------------------
// Binary I/O Stream Support
// ------------------------------------------------------------------------

binistream& operator >> ( binistream& binis, Scalar& scalar )
{

#if defined(SCALAR_TYPE_FIXED)
	if( binis.scalartype() == binios::fixed16_16 )
		return binis >> scalar._value;
	else // need to convert float32 -> fixed16_16
	{
		assert(0);
//		uint32 float_scalar;
//		binis >> float_scalar;
//		scalar._value = XBrFloatToScalar( float_scalar );
		return binis;
	}
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
	if( binis.scalartype() == binios::float32 )
		return binis >> (uint32&)scalar._value;
	else // need to convert fixed16_16 -> float32
	{
      assert(0);
//       uint32 fixed_scalar;
//       binis >> fixed_scalar;
//       scalar._value = XBrFixedToScalar( fixed_scalar );
		return binis;
	}
#else
#error SCALAR TYPE not defined
#endif
}

// ------------------------------------------------------------------------

#if defined( WRITER )

binostream& operator << ( binostream& binos, const Scalar scalar )
{
  #if BASED_FLOAT
	if( binos.scalartype() == binios::float32 )
		return binos << (uint32&)scalar._value;
	else // need to convert float32 -> fixed16_16
	{
		uint32 fixed_scalar = XBrScalarToFixed( scalar._value );
		return binos << fixed_scalar;
	}
  #else // BASED_FIXED
	if( binos.scalartype() == binios::fixed16_16 )
		return binos << (uint32&)scalar._value;
	else
	{
		assert(0);
//		uint32 float_scalar = XBrScalarToFloat( scalar._value );
//		return binos << float_scalar;
		return binos;
	}
  #endif
}

#endif

// ------------------------------------------------------------------------

#if defined(SCALAR_TYPE_FIXED)

#pragma pack (1)

// Full wave+1/4 table
static uint16 sin_table[]=
{
	0x0000,0x0324,0x0647,0x096A,0x0C8B,0x0FAB,0x12C7,0x15E1,
	0x18F8,0x1C0B,0x1F19,0x2223,0x2527,0x2826,0x2B1E,0x2E10,
	0x30FB,0x33DE,0x36B9,0x398C,0x3C56,0x3F16,0x41CD,0x447A,
	0x471C,0x49B3,0x4C3F,0x4EBF,0x5133,0x539A,0x55F4,0x5842,
	0x5A81,0x5CB3,0x5ED6,0x60EB,0x62F1,0x64E7,0x66CE,0x68A5,
	0x6A6C,0x6C23,0x6DC9,0x6F5E,0x70E1,0x7254,0x73B5,0x7503,
	0x7640,0x776B,0x7883,0x7989,0x7A7C,0x7B5C,0x7C29,0x7CE2,
	0x7D89,0x7E1C,0x7E9C,0x7F08,0x7F61,0x7FA6,0x7FD7,0x7FF5,
	0x7FFF,0x7FF5,0x7FD7,0x7FA6,0x7F61,0x7F08,0x7E9C,0x7E1C,
	0x7D89,0x7CE2,0x7C29,0x7B5C,0x7A7C,0x7989,0x7883,0x776B,
	0x7640,0x7503,0x73B5,0x7254,0x70E1,0x6F5E,0x6DC9,0x6C23,
	0x6A6C,0x68A5,0x66CE,0x64E7,0x62F1,0x60EB,0x5ED6,0x5CB3,
	0x5A81,0x5842,0x55F4,0x539A,0x5133,0x4EBF,0x4C3F,0x49B3,
	0x471C,0x447A,0x41CD,0x3F16,0x3C56,0x398C,0x36B9,0x33DE,
	0x30FB,0x2E10,0x2B1E,0x2826,0x2527,0x2223,0x1F19,0x1C0B,
	0x18F8,0x15E1,0x12C7,0x0FAB,0x0C8B,0x096A,0x0647,0x0324,
	0x0000,0xFCDC,0xF9B9,0xF696,0xF375,0xF055,0xED39,0xEA1F,
	0xE708,0xE3F5,0xE0E7,0xDDDD,0xDAD9,0xD7DA,0xD4E2,0xD1F0,
	0xCF05,0xCC22,0xC947,0xC674,0xC3AA,0xC0EA,0xBE33,0xBB86,
	0xB8E4,0xB64D,0xB3C1,0xB141,0xAECD,0xAC66,0xAA0C,0xA7BE,
	0xA57F,0xA34D,0xA12A,0x9F15,0x9D0F,0x9B19,0x9932,0x975B,
	0x9594,0x93DD,0x9237,0x90A2,0x8F1F,0x8DAC,0x8C4B,0x8AFD,
	0x89C0,0x8895,0x877D,0x8677,0x8584,0x84A4,0x83D7,0x831E,
	0x8277,0x81E4,0x8164,0x80F8,0x809F,0x805A,0x8029,0x800B,
	0x8001,0x800B,0x8029,0x805A,0x809F,0x80F8,0x8164,0x81E4,
	0x8277,0x831E,0x83D7,0x84A4,0x8584,0x8677,0x877D,0x8895,
	0x89C0,0x8AFD,0x8C4B,0x8DAC,0x8F1F,0x90A2,0x9237,0x93DD,
	0x9594,0x975B,0x9932,0x9B19,0x9D0F,0x9F15,0xA12A,0xA34D,
	0xA57F,0xA7BE,0xAA0C,0xAC66,0xAECD,0xB141,0xB3C1,0xB64D,
	0xB8E4,0xBB86,0xBE33,0xC0EA,0xC3AA,0xC674,0xC947,0xCC22,
	0xCF05,0xD1F0,0xD4E2,0xD7DA,0xDAD9,0xDDDD,0xE0E7,0xE3F5,
	0xE708,0xEA1F,0xED39,0xF055,0xF375,0xF696,0xF9B9,0xFCDC,
	0x0000,0x0324,0x0647,0x096A,0x0C8B,0x0FAB,0x12C7,0x15E1,
	0x18F8,0x1C0B,0x1F19,0x2223,0x2527,0x2826,0x2B1E,0x2E10,
	0x30FB,0x33DE,0x36B9,0x398C,0x3C56,0x3F16,0x41CD,0x447A,
	0x471C,0x49B3,0x4C3F,0x4EBF,0x5133,0x539A,0x55F4,0x5842,
	0x5A81,0x5CB3,0x5ED6,0x60EB,0x62F1,0x64E7,0x66CE,0x68A5,
	0x6A6C,0x6C23,0x6DC9,0x6F5E,0x70E1,0x7254,0x73B5,0x7503,
	0x7640,0x776B,0x7883,0x7989,0x7A7C,0x7B5C,0x7C29,0x7CE2,
	0x7D89,0x7E1C,0x7E9C,0x7F08,0x7F61,0x7FA6,0x7FD7,0x7FF5,
	0x7FFF,0x7FFF,
};

static uint16 arcsin_table[]=
{
	0xC001,0xC519,0xC737,0xC8D7,0xCA37,0xCB6D,0xCC87,0xCD8A,
	0xCE7C,0xCF5F,0xD037,0xD104,0xD1C9,0xD286,0xD33C,0xD3ED,
	0xD498,0xD53E,0xD5DF,0xD67C,0xD716,0xD7AC,0xD83F,0xD8CF,
	0xD95C,0xD9E7,0xDA6F,0xDAF4,0xDB78,0xDBF9,0xDC79,0xDCF7,
	0xDD73,0xDDED,0xDE66,0xDEDD,0xDF53,0xDFC8,0xE03B,0xE0AD,
	0xE11E,0xE18D,0xE1FC,0xE26A,0xE2D6,0xE342,0xE3AC,0xE416,
	0xE47F,0xE4E7,0xE54E,0xE5B4,0xE61A,0xE67F,0xE6E3,0xE746,
	0xE7A9,0xE80C,0xE86D,0xE8CE,0xE92F,0xE98F,0xE9EE,0xEA4D,
	0xEAAB,0xEB09,0xEB66,0xEBC3,0xEC20,0xEC7C,0xECD7,0xED33,
	0xED8D,0xEDE8,0xEE42,0xEE9C,0xEEF5,0xEF4E,0xEFA7,0xEFFF,
	0xF057,0xF0AF,0xF106,0xF15D,0xF1B4,0xF20B,0xF261,0xF2B8,
	0xF30D,0xF363,0xF3B9,0xF40E,0xF463,0xF4B8,0xF50C,0xF561,
	0xF5B5,0xF609,0xF65D,0xF6B1,0xF704,0xF758,0xF7AB,0xF7FE,
	0xF851,0xF8A4,0xF8F7,0xF949,0xF99C,0xF9EE,0xFA41,0xFA93,
	0xFAE5,0xFB37,0xFB89,0xFBDB,0xFC2D,0xFC7F,0xFCD1,0xFD23,
	0xFD74,0xFDC6,0xFE17,0xFE69,0xFEBA,0xFF0C,0xFF5E,0xFFAF,
	0x0000,0x0051,0x00A2,0x00F4,0x0146,0x0197,0x01E9,0x023A,
	0x028C,0x02DD,0x032F,0x0381,0x03D3,0x0425,0x0477,0x04C9,
	0x051B,0x056D,0x05BF,0x0612,0x0664,0x06B7,0x0709,0x075C,
	0x07AF,0x0802,0x0855,0x08A8,0x08FC,0x094F,0x09A3,0x09F7,
	0x0A4B,0x0A9F,0x0AF4,0x0B48,0x0B9D,0x0BF2,0x0C47,0x0C9D,
	0x0CF3,0x0D48,0x0D9F,0x0DF5,0x0E4C,0x0EA3,0x0EFA,0x0F51,
	0x0FA9,0x1001,0x1059,0x10B2,0x110B,0x1164,0x11BE,0x1218,
	0x1273,0x12CD,0x1329,0x1384,0x13E0,0x143D,0x149A,0x14F7,
	0x1555,0x15B3,0x1612,0x1671,0x16D1,0x1732,0x1793,0x17F4,
	0x1857,0x18BA,0x191D,0x1981,0x19E6,0x1A4C,0x1AB2,0x1B19,
	0x1B81,0x1BEA,0x1C54,0x1CBE,0x1D2A,0x1D96,0x1E04,0x1E73,
	0x1EE2,0x1F53,0x1FC5,0x2038,0x20AD,0x2123,0x219A,0x2213,
	0x228D,0x2309,0x2387,0x2407,0x2488,0x250C,0x2591,0x2619,
	0x26A4,0x2731,0x27C1,0x2854,0x28EA,0x2984,0x2A21,0x2AC2,
	0x2B68,0x2C13,0x2CC4,0x2D7A,0x2E37,0x2EFC,0x2FC9,0x30A1,
	0x3184,0x3276,0x3379,0x3493,0x35C9,0x3729,0x38C9,0x3AE7,
	0x4000,
};

static uint16 arccos_table[]=
{
	0x7FFF,0x7AE7,0x78C9,0x7729,0x75C9,0x7493,0x7379,0x7276,
	0x7184,0x70A1,0x6FC9,0x6EFC,0x6E37,0x6D7A,0x6CC4,0x6C13,
	0x6B68,0x6AC2,0x6A21,0x6984,0x68EA,0x6854,0x67C1,0x6731,
	0x66A4,0x6619,0x6591,0x650C,0x6488,0x6407,0x6387,0x6309,
	0x628D,0x6213,0x619A,0x6123,0x60AD,0x6038,0x5FC5,0x5F53,
	0x5EE2,0x5E73,0x5E04,0x5D96,0x5D2A,0x5CBE,0x5C54,0x5BEA,
	0x5B81,0x5B19,0x5AB2,0x5A4C,0x59E6,0x5981,0x591D,0x58BA,
	0x5857,0x57F4,0x5793,0x5732,0x56D1,0x5671,0x5612,0x55B3,
	0x5555,0x54F7,0x549A,0x543D,0x53E0,0x5384,0x5329,0x52CD,
	0x5273,0x5218,0x51BE,0x5164,0x510B,0x50B2,0x5059,0x5001,
	0x4FA9,0x4F51,0x4EFA,0x4EA3,0x4E4C,0x4DF5,0x4D9F,0x4D48,
	0x4CF3,0x4C9D,0x4C47,0x4BF2,0x4B9D,0x4B48,0x4AF4,0x4A9F,
	0x4A4B,0x49F7,0x49A3,0x494F,0x48FC,0x48A8,0x4855,0x4802,
	0x47AF,0x475C,0x4709,0x46B7,0x4664,0x4612,0x45BF,0x456D,
	0x451B,0x44C9,0x4477,0x4425,0x43D3,0x4381,0x432F,0x42DD,
	0x428C,0x423A,0x41E9,0x4197,0x4146,0x40F4,0x40A2,0x4051,
	0x3FFF,0x3FAE,0x3F5D,0x3F0B,0x3EB9,0x3E68,0x3E16,0x3DC5,
	0x3D73,0x3D22,0x3CD0,0x3C7E,0x3C2C,0x3BDA,0x3B88,0x3B36,
	0x3AE4,0x3A92,0x3A40,0x39ED,0x399B,0x3948,0x38F6,0x38A3,
	0x3850,0x37FD,0x37AA,0x3757,0x3703,0x36B0,0x365C,0x3608,
	0x35B4,0x3560,0x350B,0x34B7,0x3462,0x340D,0x33B8,0x3362,
	0x330C,0x32B7,0x3260,0x320A,0x31B3,0x315C,0x3105,0x30AE,
	0x3056,0x2FFE,0x2FA6,0x2F4D,0x2EF4,0x2E9B,0x2E41,0x2DE7,
	0x2D8C,0x2D32,0x2CD6,0x2C7B,0x2C1F,0x2BC2,0x2B65,0x2B08,
	0x2AAA,0x2A4C,0x29ED,0x298E,0x292E,0x28CD,0x286C,0x280B,
	0x27A8,0x2745,0x26E2,0x267E,0x2619,0x25B3,0x254D,0x24E6,
	0x247E,0x2415,0x23AB,0x2341,0x22D5,0x2269,0x21FB,0x218C,
	0x211D,0x20AC,0x203A,0x1FC7,0x1F52,0x1EDC,0x1E65,0x1DEC,
	0x1D72,0x1CF6,0x1C78,0x1BF8,0x1B77,0x1AF3,0x1A6E,0x19E6,
	0x195B,0x18CE,0x183E,0x17AB,0x1715,0x167B,0x15DE,0x153D,
	0x1497,0x13EC,0x133B,0x1285,0x11C8,0x1103,0x1036,0x0F5E,
	0x0E7B,0x0D89,0x0C86,0x0B6C,0x0A36,0x08D6,0x0736,0x0518,
	0x0000,
};

static uint16 arctan_table[]=
{
	0x0000,0x0028,0x0051,0x007A,0x00A2,0x00CB,0x00F4,0x011D,
	0x0145,0x016E,0x0197,0x01BF,0x01E8,0x0211,0x0239,0x0262,
	0x028B,0x02B3,0x02DC,0x0304,0x032D,0x0355,0x037E,0x03A6,
	0x03CE,0x03F7,0x041F,0x0448,0x0470,0x0498,0x04C0,0x04E8,
	0x0511,0x0539,0x0561,0x0589,0x05B1,0x05D9,0x0601,0x0628,
	0x0650,0x0678,0x06A0,0x06C7,0x06EF,0x0716,0x073E,0x0765,
	0x078D,0x07B4,0x07DB,0x0803,0x082A,0x0851,0x0878,0x089F,
	0x08C6,0x08ED,0x0913,0x093A,0x0961,0x0987,0x09AE,0x09D4,
	0x09FB,0x0A21,0x0A47,0x0A6D,0x0A94,0x0ABA,0x0AE0,0x0B05,
	0x0B2B,0x0B51,0x0B77,0x0B9C,0x0BC2,0x0BE7,0x0C0C,0x0C32,
	0x0C57,0x0C7C,0x0CA1,0x0CC6,0x0CEB,0x0D0F,0x0D34,0x0D58,
	0x0D7D,0x0DA1,0x0DC6,0x0DEA,0x0E0E,0x0E32,0x0E56,0x0E7A,
	0x0E9E,0x0EC1,0x0EE5,0x0F08,0x0F2C,0x0F4F,0x0F72,0x0F95,
	0x0FB8,0x0FDB,0x0FFE,0x1021,0x1044,0x1066,0x1089,0x10AB,
	0x10CD,0x10EF,0x1111,0x1133,0x1155,0x1177,0x1199,0x11BA,
	0x11DC,0x11FD,0x121E,0x123F,0x1260,0x1281,0x12A2,0x12C3,
	0x12E4,0x1304,0x1325,0x1345,0x1365,0x1385,0x13A5,0x13C5,
	0x13E5,0x1405,0x1424,0x1444,0x1463,0x1483,0x14A2,0x14C1,
	0x14E0,0x14FF,0x151E,0x153C,0x155B,0x1579,0x1598,0x15B6,
	0x15D4,0x15F2,0x1610,0x162E,0x164C,0x166A,0x1687,0x16A5,
	0x16C2,0x16DF,0x16FC,0x1719,0x1736,0x1753,0x1770,0x178C,
	0x17A9,0x17C5,0x17E2,0x17FE,0x181A,0x1836,0x1852,0x186E,
	0x188A,0x18A5,0x18C1,0x18DC,0x18F7,0x1913,0x192E,0x1949,
	0x1964,0x197F,0x1999,0x19B4,0x19CE,0x19E9,0x1A03,0x1A1D,
	0x1A37,0x1A51,0x1A6B,0x1A85,0x1A9F,0x1AB9,0x1AD2,0x1AEC,
	0x1B05,0x1B1E,0x1B37,0x1B50,0x1B69,0x1B82,0x1B9B,0x1BB4,
	0x1BCC,0x1BE5,0x1BFD,0x1C16,0x1C2E,0x1C46,0x1C5E,0x1C76,
	0x1C8E,0x1CA5,0x1CBD,0x1CD5,0x1CEC,0x1D04,0x1D1B,0x1D32,
	0x1D49,0x1D60,0x1D77,0x1D8E,0x1DA5,0x1DBB,0x1DD2,0x1DE9,
	0x1DFF,0x1E15,0x1E2C,0x1E42,0x1E58,0x1E6E,0x1E84,0x1E99,
	0x1EAF,0x1EC5,0x1EDA,0x1EF0,0x1F05,0x1F1B,0x1F30,0x1F45,
	0x1F5A,0x1F6F,0x1F84,0x1F99,0x1FAD,0x1FC2,0x1FD7,0x1FEB,
	0x2000,
};

#pragma pack ()

//=============================================================================

INLINE long MoveSignExtend(long __a0)
{
	    register long __ret;
		// sign extend low 16 bits to 32 bits
		ShiftRightArithmetic(__ret,__a0 << 16,16);
		return(__ret);
}

//-----------------------------------------------------------------------------

inline
short SignedTableInterpolate(unsigned short in, short* table)
{
//	 given a 16 bit input, generate the output value
//	 of a function using a 256-word lookup table with
//	 interpolation between entries
// table must actually be at least 257 entries long

	int index = in >> 8;
	assert(index >= 0);
	assert(index < 256);

	MATH_DEBUG( std::cout << "\nSTI:: in = " << std::hex << in << ", table[index] = " << table[index] << ", table[index+1] = " << table[index+1] << std::endl; )

	Scalar v1(table[index],0);
	Scalar v2(table[index+1],0);
	MATH_DEBUG( std::cout << "v1 = " << v1 << ", v2 = " << v2 << std::endl; )
	Scalar percentage(0,(in & 0xff) << 8);
//	std::cout << "percentage = " << percentage << std::endl;
//	std::cout << "v2-v2 = " << (v2-v1) << std::endl;
//	std::cout << "v2-v2*% = " << ((v2-v1)*percentage) << std::endl;
//	Scalar ret = v1 + ((v2-v1)*percentage);
//	std::cout << "ret = " << ret << std::dec << std::endl;
//	return ret.WholePart();
	return (v1 + ((v2-v1)*percentage)).WholePart();
}

//-----------------------------------------------------------------------------

INLINE
unsigned short INTERP_FUNCTION(long __a0,unsigned short* __table)
//	 given a 16 bit input, generate the output value
//	 of a function using a 256-word lookup table with
//	 interpolation between entries
{
	register long __t3;
	register long __t2;
	register long __t0;
	register long __t1;
	register long __out1;
	register unsigned long __out0;

	// t1:t0 = split input into high and low bytes

	ShiftRightLogical(__t1,__a0,8);

	__t1 &= 0xff;
	__t0 =__a0 & 0xff;

	MATH_DEBUG( std::cout << "\nIF: " << std::hex << __a0; )

	__t2 = *(((uint16 *)__table) + __t1); // table[t1*2]
	__t3 = *(((uint16 *)__table) + __t1 + 1);	// table[ +2[t1*2]

	MATH_DEBUG( std::cout << ";" << __t2 << "," << __t3; )
	assert(__t2 < __t3);				// cannot handle falling values

	// t3 = delta to next entry in table

	__t3 -= __t2;
	MATH_DEBUG( std::cout << ", new t3 = " << __t3 << ", t0 = " << __t0; )
	// scale delta by low byte of input

	Multiply64(__t0,__t3,__out1,__out0);
	MATH_DEBUG( std::cout << ":m64: " << __out1 << ":" << __out0 << std::dec << std::endl; )

	ShiftRightLogical(__out1,__out1,16);

	ShiftRightLogical(__out0,__out0,8);
	__out1 <<= 8;

	// Add base entry to interpolated delta

//	assert(__out1 < QS_ONE_LS);
//	assert(__out0 < QS_ONE_LS);
//	assert(__t2 < QS_ONE_LS);
//	assert(((__out1 | __out0) + __t2) < QS_ONE_LS);
   	return uint16((__out1 | __out0) + __t2);
}

#endif         // defined(SCALAR_TYPE_FIXED)

// ------------------------------------------------------------------------
// Trigonometric Methods
// ------------------------------------------------------------------------

Scalar
Scalar::Sin( void ) const
{
   Validate();
#if defined(SCALAR_TYPE_FIXED)
   register long out0;
	uint16 a = (uint16( _value));
	out0 = SignedTableInterpolate((a & 0xffff),(short*)sin_table);
	return Scalar(MoveSignExtend(out0) << 1);
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
   return Scalar(sin(_value*(2.0*PI)));
#else
#error SCALAR TYPE not defined
#endif
}

// ------------------------------------------------------------------------

Scalar
Scalar::Cos( void ) const
{
   Validate();

#if defined(SCALAR_TYPE_FIXED)
   register long out0;
	uint16 a = (uint16(_value));

	out0 = SignedTableInterpolate((a & 0xffff),((short*)sin_table + 64));
 	return Scalar(MoveSignExtend(out0) << 1);
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
   return Scalar(cos(_value*(2.0*PI)));
#else
#error SCALAR TYPE not defined
#endif
}

//-----------------------------------------------------------------------------

Angle
Scalar::ACos( void ) const
{
   Validate();
#if defined(SCALAR_TYPE_FIXED)
	// Handle special cases where the bottom word is zero
	if (_value == 65536)
		return Angle::zero;

	if (_value == -65536)
		return Angle::half;

   	register long out0;
	uint16 c = uint16(_value);

	ShiftRightArithmetic(out0,c + 0x10000,1);

	return Angle(SignedTableInterpolate(out0,(short*)arccos_table));
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
   return Angle::Radian(Scalar(acos(_value)));
#else
#error SCALAR TYPE not defined
#endif
}

// ------------------------------------------------------------------------

Angle
Scalar::ASin( void ) const
{
   Validate();
#if defined(SCALAR_TYPE_FIXED)

	// Handle special cases where the bottom word is zero
	if (_value == 65536)
		return Angle::onequarter;

	if (_value == -65536)
		return Angle::threequarters;

   	register long out0;
	uint16 s = uint16(_value);
   	ShiftRightArithmetic(out0,s + 0x10000,1);

	return Angle(SignedTableInterpolate(out0,(short*)arcsin_table));
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
   double result = asin(_value);
   AssertMsg(!isnan(result),"_value = " << _value << ", result = " << result);    
   return Angle::Radian(Scalar(result));
#else
#error SCALAR TYPE not defined
#endif
}

// ------------------------------------------------------------------------

Angle
Scalar::ATan2( const Scalar& in ) const
{
   Validate();
   //std::cout << "ATan2: _value = " << _value << ", in._value = " << in._value << std::endl;

#if defined(SCALAR_TYPE_FIXED)
	int32 x = _value;
	int32 y = in._value;

//	cscreen << "AT2: " << x << "," << y << std::endl;
//	cscreen << "Hex: " << hex << x << "," << y << dec << std::endl;

	long out2;
	long out1=0;
	long out0;

//	 * Break in 8 octants, based on sign of X, sign of Y and relative magnitude
//	 * of X to Y
//	 *
// 	 *
//	 *	0	x>0  y>0  |x| > |y| 	=       +atan(|y|/|x|)
//	 *	1	x>0  y>0  |x| < |y| 	= PI/2  -atan(|x|/|y|)
//	 *	2	x<0  y>0  |x| < |y| 	= PI/2  +atan(|x|/|y|)
//	 *	3	x<0  y>0  |x| > |y| 	= PI    -atan(|y|/|x|)
//	 *	4	x<0  y<0  |x| > |y| 	= PI    +atan(|y|/|x|)
//	 *	5	x<0  y<0  |x| < |y| 	= PI*3/2-atan(|x|/|y|)
//	 *	6	x>0  y<0  |x| < |y| 	= PI*3/2+atan(|x|/|y|)
//	 *	7	x>0  y<0  |x| > |y| 	= PI*2  -atan(|y|/|x|)

	if(y < 0)			// octant 4567
	{
		y = -y;
		if(x > 0)		// octant 67
		{
			if(x > y)	// octant 7
			{

				out2 = y;
				out0 = x;
// result(a1:a0) = a2:a1/a0

            MATH_DEBUG( std::cout << "upper = " << out2 << ", lower = " << out1 << ", dividend = " << out0 << std::endl; )
				SignedDivide64(out2,out1,out0);
            MATH_DEBUG( std::cout << ", upper result = " << out1 << ", lower result = " << out0 << std::endl; )
				ShiftRightLogical(out1,out0,16);
				return Angle(-INTERP_FUNCTION(out1,arctan_table));
			}
			else
			{
				if(x == y)
					return Angle(0xe000); // boundary 67
				else
				{
					// octant 6
					out2 = x;
					out0 = y;
               MATH_DEBUG( std::cout << "upper = " << out2 << ", lower = " << out1 << ", dividend = " << out0 << std::endl; )
					SignedDivide64(out2,out1,out0);
               MATH_DEBUG( std::cout << ", upper result = " << out1 << ", lower result = " << out0 << std::endl; )
					ShiftRightLogical(out1,out0,16);
					return Angle(INTERP_FUNCTION(out1,arctan_table) + 0xc000);
				}
			}
		}
		else
		{
			x = -x;
			if(x < y)	// octant 5
			{
				out2 = x;
				out0 = y;
            MATH_DEBUG( std::cout << "upper = " << out2 << ", lower = " << out1 << ", dividend = " << out0 << std::endl; )
				SignedDivide64(out2,out1,out0);
            MATH_DEBUG( std::cout << ", upper result = " << out1 << ", lower result = " << out0 << std::endl; )
				ShiftRightLogical(out1,out0,16);
				return Angle((-INTERP_FUNCTION(out1,arctan_table)) + 0xc000);
			}
			else
			{
				if(x == y)
					return Angle(0xa000); // boundary 45
				else
				{
					// octant 4
					out2 = y;
					out0 = x;
               MATH_DEBUG( std::cout << "upper = " << out2 << ", lower = " << out1 << ", dividend = " << out0 << std::endl; )
					SignedDivide64(out2,out1,out0);
               MATH_DEBUG( std::cout << ", upper result = " << out1 << ", lower result = " << out0 << std::endl; )
					ShiftRightLogical(out1,out0,16);
					return Angle(INTERP_FUNCTION(out1,arctan_table) + 0x8000);
				}
			}
		}

	}
	else if(x == 0)
	{
		if(y < 0)
			return Angle(0);
		else
			return Angle(0x4000);
	}
	else    // octant 0123
	{
		if(x < 0)	// octant 23
		{
			x = -x;
			if(x > y)   // octant 3
			{
				out2 = y;
				out0 = x;
            MATH_DEBUG( std::cout << "upper = " << out2 << ", lower = " << out1 << ", dividend = " << out0 << std::endl; )
				SignedDivide64(out2,out1,out0);
            MATH_DEBUG( std::cout << ", upper result = " << out1 << ", lower result = " << out0 << std::endl; )
				ShiftRightLogical(out1,out0,16);
				return Angle((-INTERP_FUNCTION(out1,arctan_table)) + 0x8000);
			}
			else
			{
				if(x == y)
					return Angle(0x6000); // boundary 23
				else    // octant 2
				{
					out2 = x;
					out0 = y;
               MATH_DEBUG( std::cout << "upper = " << out2 << ", lower = " << out1 << ", dividend = " << out0 << std::endl; )
					SignedDivide64(out2,out1,out0);
               MATH_DEBUG( std::cout << ", upper result = " << out1 << ", lower result = " << out0 << std::endl; )
					ShiftRightLogical(out1,out0,16);
					return Angle(INTERP_FUNCTION(out1,arctan_table) + 0x4000);
				}
			}
		}
		else
		{
			if(x < y)	// octant 1
			{
				out2 = x;
				out0 = y;
				SignedDivide64(out2,out1,out0);
            MATH_DEBUG( std::cout << "upper = " << out2 << ", lower = " << out1 << ", dividend = " << out0 << std::endl; )
				ShiftRightLogical(out1,out0,16);
            MATH_DEBUG( std::cout << "upper result = " << out1 << ", lower result = " << out0 << std::endl; )
				return Angle( (-INTERP_FUNCTION(out1,arctan_table)) + 0x4000);
			}
			else
			{
				if(x == y)
					return Angle(0x2000); // boundary 01
				else
				{
				    // octant 0
					out2 = y;
					out0 = x;
               MATH_DEBUG( std::cout << "upper = " << out2 << ", lower = " << out1 << ", dividend = " << out0 << std::endl; )
					SignedDivide64(out2,out1,out0);
               MATH_DEBUG( std::cout << ", upper result = " << out1 << ", lower result = " << out0 << std::endl; )
					ShiftRightLogical(out1,out0,16);
					return Angle(INTERP_FUNCTION(out1,arctan_table));
				}
			}
		}
	}
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
   FLOAT_TYPE result = atan2(in._value, _value);
   //std::cout << "in._value = " << in._value << ", _value = " << _value << ", result = " << result << std::endl;

   AssertMsg(!isnan(result),"in._value = " << in._value << ", _value = " << _value << ", result = " << result);    
   return Angle::Radian(Scalar(result));
#else
#error SCALAR TYPE not defined
#endif
}

// ------------------------------------------------------------------------

Angle
Scalar::ATan2Quick( const Scalar& in ) const
{
   Validate();
#if defined(SCALAR_TYPE_FIXED)

	int32 x = _value;
	int32 y = in._value;

    long out2;
	long out1 = 0;
	long out0;

//	 *
//	 * Less accurate atan2() - approximate atan(a) to (PI/4 * a) for 0-PI/4
//	 *
//	 * Break in 8 octants, based on sign of X, sign of Y and relative magnitude
//	 * of X to Y
//	 *
//	 *	0	x>0  y>0  |x| > |y| 	=        PI/4*(|y|/|x|)
//	 *	1	x>0  y>0  |x| < |y| 	= PI/2  -PI/4*(|x|/|y|)
//	 *	2	x<0  y>0  |x| < |y| 	= PI/2  +PI/4*(|x|/|y|)
//	 *	3	x<0  y>0  |x| > |y| 	= PI    -PI/4*(|y|/|x|)
//	 *	4	x<0  y<0  |x| > |y| 	= PI    +PI/4*(|y|/|x|)
//	 *	5	x<0  y<0  |x| < |y| 	= PI*3/2-PI/4*(|x|/|y|)
//	 *	6	x>0  y<0  |x| < |y| 	= PI*3/2+PI/4*(|x|/|y|)
//	 *	7	x>0  y<0  |x| > |y| 	= PI*2  -PI/4*(|y|/|x|)

	if(x < 0)
	{
	    	// octant 4567
		x = -x;

		if(y > 0)
		{
		    	// octant 67

			if(x > y)
			{
			    	// octant 7

				out2 = x;
				out0 = y;

				SignedDivide64(out2,out1,out0);

				ShiftRightLogical(out1,out0,19);

				return Angle(-out1);
			}
			else
				if (x == y)
					return Angle(0xe000); // boundary 67
				else
				{
				    	// octant 6

					out2 = y;
					out0 = x;

					SignedDivide64(out2,out1,out0);

					ShiftRightLogical(out1,out0,19);

					return Angle(out1 + 0xc000);
				}
		}
		else
			if(y == 0)
				return Angle(0xa000); // boundary 45
			else
			{
				// octant 45

				y = -y;
				if(x < y)
				{
				    	// octant 5
					out2 = y;
					out0 = x;

					SignedDivide64(out2,out1,out0);

					ShiftRightLogical(out1,out0,19);

					return Angle((-out1) + 0xc000);
				}
				else
				{
				    	// octant 4

					out2 = x;
					out0 = y;

					SignedDivide64(out2,out1,out0);

					ShiftRightLogical(out1,out0,19);

					return Angle(out1 + 0x8000);
				}
			}
	}
	else if(x == 0)
	{
	    	// y zero

		if(y < 0)
			return Angle(0x8000);
		else
			return Angle(0);
	}
		else
		{
			if(y < 0)
			{
		    		// octant 23
				y = -y;
				if(x > y)
				{
				    	// octant 3

					out2 = x;
					out0 = y;

					SignedDivide64(out2,out1,out0);

					ShiftRightLogical(out1,out0,19);

					return Angle((-out1) + 0x8000);
				}
				else
					if(x == y)
						return Angle(0x6000); // boundary 23
					else
					{
					    	// octant 2

						out2 = y;
						out0 = x;

						SignedDivide64(out2,out1,out0);

						ShiftRightLogical(out1,out0,19);

						return Angle(out1 + 0x4000);
					}
			}
			else if(x < y)
			{
			    	// octant 1

				out2 = y;
				out0 = x;

				SignedDivide64(out2,out1,out0);

				ShiftRightLogical(out1,out0,19);

				return Angle((-out1) + 0x4000);
			}
			else
			if(x == y)
				return Angle(0x2000); // boundary 01
		}
	assert(0);
	return Angle(0);
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
   assert(0);
   // kts write a fast version of this 
#else
#error SCALAR TYPE not defined
#endif
}

//=============================================================================
// 64 bit operations
//=============================================================================

#if defined(SCALAR_TYPE_FIXED)

unsigned long
Sqrt64(long __a1,long __a0)
{
	unsigned long __t1;
	unsigned long __t2;
	register unsigned long __Estimate;
	register unsigned long __EstimateX2;
	unsigned long __Error;
	int __i;

	//  result = sqrt(a1:a0)

	__Estimate = __EstimateX2 = __Error = 0;
	__t1 = __a0;
	__t2 = __a1;

 	for(__i=0; __i<16; __i++) 			// 16 bit pairs from high dword
	{
		ShiftLeft64(__Error,__t2,2); // 2 bits into error

		__Estimate <<= 1;		// estimate *= 2
		__EstimateX2 <<= 1;

		if (__EstimateX2 < __Error) 		// if estimate*2 < error
		{

			__Error -= __EstimateX2 + 1;	// update error
			++__Estimate;					// set low bit of estimate
			__EstimateX2 += 2;				// set bit 1 of estimate*2
		}
	}

	for(__i=0; __i<16; __i++) 				// 16 bit pairs from low dword
	{

		ShiftLeft64(__Error,__t1,2); 		// 2 bits into error

		__Estimate <<= 1;					// estimate *=2
		__EstimateX2 <<= 1;

		if(__EstimateX2 < __Error) 			// if estimate*2 < error
		{
			__Error -= __EstimateX2 + 1;	// update error
			++__Estimate;					// set low bit of estimate
			__EstimateX2 += 2;				// set low bit of estimate*2
		}
	}
	return(__Estimate);
}

//=============================================================================

#pragma pack (1)
// 0x10000/sqrt(x) from 64 to 255
static uint16 frsqrt_table[] =
{
	0xFFFF,0xFE05,0xFC17,0xFA33,0xF85B,0xF68C,0xF4C8,0xF30D,
	0xF15B,0xEFB3,0xEE13,0xEC7B,0xEAEB,0xE964,0xE7E3,0xE66B,
	0xE4F9,0xE38E,0xE229,0xE0CC,0xDF74,0xDE23,0xDCD7,0xDB91,
	0xDA51,0xD916,0xD7E0,0xD6B0,0xD584,0xD45E,0xD33C,0xD21E,
	0xD105,0xCFF1,0xCEE1,0xCDD4,0xCCCC,0xCBC8,0xCAC8,0xC9CB,
	0xC8D2,0xC7DD,0xC6EB,0xC5FC,0xC511,0xC429,0xC344,0xC263,
	0xC184,0xC0A8,0xBFD0,0xBEFA,0xBE26,0xBD56,0xBC88,0xBBBD,
	0xBAF4,0xBA2E,0xB96A,0xB8A9,0xB7EA,0xB72D,0xB673,0xB5BB,
	0xB504,0xB450,0xB39F,0xB2EF,0xB241,0xB195,0xB0EB,0xB043,
	0xAF9D,0xAEF8,0xAE56,0xADB5,0xAD16,0xAC79,0xABDD,0xAB43,
	0xAAAA,0xAA13,0xA97E,0xA8EA,0xA858,0xA7C7,0xA737,0xA6A9,
	0xA61D,0xA592,0xA508,0xA47F,0xA3F8,0xA372,0xA2EE,0xA26A,
	0xA1E8,0xA167,0xA0E7,0xA069,0x9FEC,0x9F6F,0x9EF4,0x9E7A,
	0x9E01,0x9D89,0x9D13,0x9C9D,0x9C28,0x9BB4,0x9B42,0x9AD0,
	0x9A5F,0x99EF,0x9981,0x9913,0x98A6,0x983A,0x97CE,0x9764,
	0x96FB,0x9692,0x962A,0x95C3,0x955D,0x94F8,0x9493,0x9430,
	0x93CD,0x936B,0x9309,0x92A9,0x9249,0x91E9,0x918B,0x912D,
	0x90D0,0x9074,0x9018,0x8FBD,0x8F63,0x8F09,0x8EB0,0x8E58,
	0x8E00,0x8DA9,0x8D53,0x8CFD,0x8CA8,0x8C53,0x8BFF,0x8BAC,
	0x8B59,0x8B06,0x8AB5,0x8A64,0x8A13,0x89C3,0x8973,0x8924,
	0x88D6,0x8888,0x883B,0x87EE,0x87A1,0x8755,0x870A,0x86BF,
	0x8675,0x862B,0x85E1,0x8598,0x8550,0x8508,0x84C0,0x8479,
	0x8432,0x83EC,0x83A6,0x8361,0x831C,0x82D7,0x8293,0x824F,
	0x820C,0x81C9,0x8186,0x8144,0x8103,0x80C1,0x8080,0x8040,
};

#pragma pack ()

//=============================================================================

#if USE_HARDWARE_GTE
#define BitScanReversed(__a0)	(Lzc(__a0))
#else
inline long
BitScanReversed(long __a0)
{
	register long __ret;

	// trashed a0
	    if(__a0)	// test high bit next
			for( __ret = 32; __a0 > 0; __ret --, __a0 <<= 1)
				;
		else
			__ret = 0;
	return(__ret);
}
#endif

//=============================================================================

unsigned long
FastRSqrt64(unsigned long __a1,unsigned long __a0)
 {
	unsigned long __tmp0;
	unsigned long __tmp1;
	unsigned long __tmp2;
	register long __offset = 64;

	// result = 64 bit low precision integer reciprocal square root
	// trashes __a1,__a0

	if(__a1) 				// has high word
	{
		__a0 = __a1;
		__offset = -__offset;
	}

	// find even exponent and shift to bits 0-7
	if(__a0)
	{
		__tmp2 = BitScanReversed(__a0) | 1;
		__tmp1 = __a0;
		__tmp0 = 0;

		ShiftRight64(__tmp1,__tmp0,__tmp2);
		__tmp1 |= __tmp0;

		__tmp0 = 0;

		ShiftRight64(__tmp1,__tmp0,1);

		ShiftRightLogical(__tmp1,__tmp1 | __tmp0, 24);

		__tmp0 = *(((uint16*)frsqrt_table) + __tmp1 + __offset);

		ShiftRightLogical(__tmp2,__tmp2,1);

		ShiftRightLogicalVar(__a0,__tmp0,__tmp2);
	}
	return __a0;
}

//=============================================================================

//Scalar
//Mac2(Scalar a, Scalar b, Scalar c, Scalar d)
//{
//	register long out0;
//	register long out1;
//	register long out2;
//	register long out3;

//	// result = a*b + c*d
//	MultiplyAndNop64(a.AsLong(),b.AsLong(),out1,out0);

//	Multiply64(c.AsLong(),d.AsLong(),out3,out2);

//	AddCarry64(out1,out0,out3,out2);

//	return JoinHiLo(out1,out0);
//}

//=============================================================================

//inline Scalar
//Mac3(Scalar a1, Scalar b1, Scalar a2, Scalar b2, Scalar a3, Scalar b3)
//{
//    register long out0;
//	register long out1;
//	register long out2;
//	register long out3;

//	// result = a1*b1 + a2*b2 + a3*b3
//	MultiplyAndNop64(a1.AsLong(),b1.AsLong(),out1,out0);

//	Multiply64(a2.AsLong(),b2.AsLong(),out3,out2);

//	AddCarry64(out1,out0,out3,out2);
            
//	Multiply64(a3.AsLong(),b3.AsLong(),out3,out2);

//	AddCarry64(out1,out0,out3,out2);

//	return JoinHiLo(out1,out0);
//}
#endif         // defined(SCALAR_TYPE_FIXED)

//=============================================================================

Scalar
Scalar::Random()
{
   long temp;
#if defined(__LINUX__)
#if !(RAND_MAX == INT_MAX)
#error update rand code
#endif
	temp = rand() >> 16;
#elif defined(__WIN__)
	temp = rand() << 1;
#else 
#error rand not implemented for this platform
#endif

	Scalar random_value;


#if defined(SCALAR_TYPE_FIXED)
   random_value._value = temp;
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
   random_value._value = temp;
   random_value._value /=  65536;
#else
#error SCALAR TYPE not defined
#endif

	RangeCheck( Scalar::zero, random_value, Scalar::one );
	return random_value;
}

//=============================================================================

Scalar
Scalar::Random( Scalar lower, Scalar upper )
{
	AssertMsg(lower < upper,"lower = " << lower << ", upper = " << upper);
	Scalar random_value = Scalar::Random();
	random_value *= ( upper - lower );
	random_value += lower;
	RangeCheck( lower, random_value, upper );

	return random_value;
}

//=============================================================================


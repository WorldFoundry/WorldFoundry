// fixed.cc

#include <iffwrite/iffwrite.hp>

size_specifier fp_1_15_16 = { 1, 15, 16 };


int
HighestBitSet(int input)
{
	int bit=-1;
	if( input < 0)
		input = (-input)-1;
	while(input)
	{
		input>>=1;
		bit++;
	}
	return bit;
}

Fixed::Fixed( size_specifier ss, double val )
{
	AssertMsg( HighestBitSet(val) < ss.whole, "value " << val << "(" << HighestBitSet(val) << ") too large to fit into size " << ss.whole); 
	_ss = ss;
	_val = val;

}


std::ostream&
operator << ( std::ostream& s, const Fixed& f )
{
	s << f._val << " => " <<
		(unsigned long)( f._val * ( 1 << f._ss.fraction ) ) / double( 1 << f._ss.fraction ) <<
		" [" << f._ss.sign << '.' << f._ss.whole << '.' << f._ss.fraction << ']';

	return s;
}


void
IffWriterText::out_fixed( const Fixed& f )
{
	*_out << PrintF( "%#.16g(%d.%d.%d) ", f._val, f._ss.sign, f._ss.whole, f._ss.fraction )();
}


// PrintF
// Fixed::print( ) const
// {
//     return PrintF( "%#.16g(%d.%d.%d) ", _val, _ss.sign, _ss.whole, _ss.fraction );
// }


void
IffWriterBinary::out_fixed( const Fixed& f )
{
	int nBits = f._ss.sign + f._ss.whole + f._ss.fraction;

	if ( nBits > 16 )
		*this << (unsigned long)( f._val * ( 1 << f._ss.fraction ) );
	else if ( nBits > 8 )
		*this << (unsigned short)( f._val * ( 1 << f._ss.fraction ) );
	else
		*this << (unsigned char)( f._val * ( 1 << f._ss.fraction ) );
}

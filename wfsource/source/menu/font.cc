// font.cc

#include <menu/font.hp>

#include "cg.c"

int tblCenturyGothicWidth[] =
{
	0,		// 0
	17,		// 1
	32,		// 2
	48,		// 3
	62,		// 4
	79,		// 5
	95,		// 6
	111,	// 7
	125, 	// 8
	143, 	// 9
	157,  	// A
	180,  	// B
	195,  	// C
	218,	// D
	237,	// E
	252,  	// F
	264,  	// G
	289,  	// H
	308,  	// I
	314,  	// J
	329,  	// K
	346,  	// L
	357,  	// M
	384,  	// N
	404,  	// O
	428,  	// P
	443,  	// Q
	467,  	// R
	482,  	// S
	496,  	// T
	510,  	// U
	527,  	// V
	546,  	// W
	571,  	// X
	590,  	// Y
	607,  	// Z
	620		// [end]
};

Font::Font()
{
}


Font::~Font()
{
}


void
Font::Render( PixelMap& pm, const char* str )
{
	assert( str );



}

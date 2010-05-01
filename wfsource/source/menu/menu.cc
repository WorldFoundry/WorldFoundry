//==============================================================================
// menu.cc
// Copyright 1998,1999,2000,2003 World Foundry Group.  
// by William B. Norris IV
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

//#include <hal/hal.h>
//#include <cpplib/stdstrm.hp>
#include <pigsys/pigsys.hp>
#include <menu/menu.hp>
#include <cpplib/range.hp>
#include <hal/sjoystic.h>
#include <gfx/rendobj2.hp>
#include <gfx/material.hp>
#include <gfx/pixelmap.hp>

//#include <gl/glaux.h>
#define FntPrint	printf

////////////////////////////////////////////////////////////////////////////////

#if 0
static Texture dummyTexture = {"",0,320,0,64,64};
const Texture&
LookupTexture(const char* name, int32 userData)
{
	return dummyTexture;
}


static Vertex2D vertexList[4] =
{
	Vertex2D( SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),Color(), Vector2( SCALAR_CONSTANT(0),SCALAR_CONSTANT( 9 ) ) ),
	Vertex2D( SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),Color(), Vector2( SCALAR_CONSTANT(319),SCALAR_CONSTANT( 9 ) ) ),
	Vertex2D( SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),Color(), Vector2( SCALAR_CONSTANT(319),SCALAR_CONSTANT( -1 ) ) ),
	Vertex2D( SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),Color(), Vector2( SCALAR_CONSTANT(0),SCALAR_CONSTANT( -1 ) ) )
};

static Material materialList[2] =
{
	Material(Color(168,20,20),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),
	Material(Color(168,20,20),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),
};

static TriFace faceList[2] =
{
	TriFace(0,1,2,0),
	TriFace(0,2,3,1)
};
#endif


static const char*
Center( const char* szMessage )
{
	const int MAX_CHARS = 40;
	static char szBuffer[ MAX_CHARS+1 ];

	assert( strlen( szMessage ) <= MAX_CHARS );

	int nSpaces = ( MAX_CHARS - strlen( szMessage ) ) / 2;
//	printf( "nSpaces = %d\n", nSpaces );
	memset( szBuffer, ' ', nSpaces );
	strcpy( szBuffer + nSpaces, szMessage );

	return szBuffer;
}


int
SimpleMenu( const char* szHeader, const char* szFooter,
	char* descStrings[], char* helpLines[],
	Range& idxSelected, int idxSelectedDefault )
{
#if 1
        idxSelected = idxSelectedDefault;
#else
	LMalloc memMenu( 200 );

	RenderObject2D menuBackground( memMenu, 4, vertexList, 2, faceList, materialList );
	Vector2 position( SCALAR_CONSTANT(0), SCALAR_CONSTANT(0) );

	Display display( 2, 320, 240 );
	PixelMap vram( PixelMap::MEMORY_VIDEO, Display::VRAMWidth, Display::VRAMHeight );

	display.SetBackgroundColor( Color::darkgrey );
	ViewPort vp( display, 4000, SCALAR_CONSTANT( 320 ), SCALAR_CONSTANT( 240 ) );

#if defined( __PSX__ )
	FntFlush( -1 );
	SetDispMask( 1 );
#endif
#if defined( __WIN__ )
//	auxCreateFont();
#endif

	IJoystick			joy1;
	joystickButtonsF	b1 = 0;

	joy1 = JoystickNew( EJW_JOYSTICK1 );
	assert( joy1 );

	idxSelected = idxSelectedDefault;

	int deltaRed = 5;

	do
	{
		{ // animate the colour bar
		Color colMenuBackground = materialList[0].GetColor();
		colMenuBackground.SetRed( colMenuBackground.Red() + deltaRed );
		materialList[ 0 ].SetColor( colMenuBackground );
		materialList[ 1 ].SetColor( colMenuBackground );
		menuBackground.ApplyMaterial( materialList );
		if ( ( colMenuBackground.Red() > 240 ) || ( colMenuBackground.Red() <= 80 ) )
			deltaRed = -deltaRed;
		}

		position.SetY( SCALAR_CONSTANT( 8*5 ) + SCALAR_CONSTANT( 16 ) * idxSelected() );

		vp.Clear();

		FntPrint( "\n" );
		FntPrint( Center( szHeader ) );
		FntPrint( "\n\n\n" );

#pragma message( "WBN: range assignment constructors" )
//		Range loop( range );
		for ( int index=0; index<=idxSelected.max(); ++index )
		{
			if ( index == idxSelected() )
			{
				menuBackground.Render( vp, position, 1 );
				FntPrint( ">" );	// temp
			}

			FntPrint( Center( descStrings[ index ] ) );
			FntPrint( "\n\n" );
		}

		FntPrint( "\n" );
		FntPrint( Center( szFooter ) );
		FntPrint( "\n" );
		FntPrint( Center( helpLines[ idxSelected() ] ) );

		b1 = JoystickGetButtonsF( joy1 );
		b1 |= ( rand() % 2 ) ? EJ_BUTTONF_DOWN : EJ_BUTTONF_UP;

		if ( b1 & EJ_BUTTONF_UP )
			--idxSelected;

		if ( b1 & EJ_BUTTONF_DOWN )
			++idxSelected;

		vp.Render();
		display.PageFlip();
	}
#pragma message( "WBN: Make EJ_BUTTONF_START and EJ_BUTTONF_STOP" )
	until( b1 & EJ_BUTTONF_I );

	JoystickDelete( joy1 );
#endif

	return idxSelected();
}

//==============================================================================


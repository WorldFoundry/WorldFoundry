// colorpck.c

#include <cstdio>
#include <pigsys/assert.hp>
#include <cstring>

#include "../util.h"
#include "source/oas/oad.h"

#include "../oaddlg.h"
#include "colorpck.h"

ColorPicker::ColorPicker( typeDescriptor* td ) : uiDialog( td )
{
	_bSave = true;
}


ColorPicker::~ColorPicker()
{
	assert( _color );
	ReleaseIColorSwatch( _color );

	assert( hwndColor );
	DestroyWindow( hwndColor );
}


#define BR_COLOUR_RGB(r,g,b) \
		((((unsigned int)(r))<<16) |\
		(((unsigned int)(g))<<8) |\
		((unsigned int)(b)))


int
ColorPicker::storedDataSize() const
{
	assert( _color );
	COLORREF colorRef = _color->GetColor();
	int r = GetRValue( colorRef );
	int g = GetGValue( colorRef );
	int b = GetBValue( colorRef );
	int32 i = BR_COLOUR_RGB( r, g, b );

	char szBuffer[ 128 ];
	sprintf( szBuffer, "%d", i );

	return uiDialog::storedDataSize() + strlen( szBuffer ) + 1;
}


dataValidation
ColorPicker::copy_to_xdata( byte* & saveData )
{
	uiDialog::copy_to_xdata( saveData );

	assert( _td );
	assert( _color );
	COLORREF colorRef = _color->GetColor();
	int r = GetRValue( colorRef );
	int g = GetGValue( colorRef );
	int b = GetBValue( colorRef );
	int32 i = BR_COLOUR_RGB( r, g, b );

	char szBuffer[ 128 ];
	sprintf( szBuffer, "%d", i );
	strcpy( (char*)saveData, szBuffer );
	saveData += strlen( szBuffer ) + 1;

	return DATA_OK;
}


void
ColorPicker::reset()
{
	assert( _td );
	reset( _td->def );
}


#define BR_RED(c) ((c >> 16) & 0xFF)
#define BR_GRN(c) ((c >> 8) & 0xFF)
#define BR_BLU(c) ((c) & 0xFF)

void
ColorPicker::reset( int32 data )
{
	assert( _td );
	assert( _color );

	int r = BR_RED( data );
	int g = BR_GRN( data );
	int b = BR_BLU( data );
	_colorRef = RGB( r, g, b );

	_color->SetColor( _colorRef, FALSE );
}


void
ColorPicker::reset( byte* & saveData )
{
	_bUserOverrideEnable = true;

	char* pSaveData = (char*)saveData;
	unsigned long lColour = atol( pSaveData );
	reset( lColour );
	saveData += strlen( pSaveData ) + 1;
}


double
ColorPicker::eval() const
{
	assert( _color );
	COLORREF colorRef = _color->GetColor();
	int r = GetRValue( colorRef );
	int g = GetGValue( colorRef );
	int b = GetBValue( colorRef );
	int32 i = BR_COLOUR_RGB( r, g, b );
	return float( i );
}


bool
ColorPicker::enable( bool bEnabled )
{
	if ( uiDialog::enable( bEnabled ) )
	{
		assert( _color );
		uiDialog::enable() ? _color->Enable() : _color->Disable();
		return true;
	}
	else
		return false;
}


int
ColorPicker::make_dialog_gadgets( HWND hPanel )
{
	assert( _td );

	assert( hPanel );
	assert( hInstance );
	assert( _td->name );

	int wLabel = uiDialog::make_dialog_gadgets( hPanel );

	int x = 5 + wLabel;
	int width = LAST_BUTTON_WIDTH( x );
	hwndColor = theAttributes.CreateWindowEx(
		0,
		"ColorSwatch",
		_td->name,
		0 | WS_TABSTOP,
		x,
		theAttributes.y,
		width, 16,
		hPanel,
		NULL,
		hInstance,
		NULL );
	assert( hwndColor );
	_colorRef = RGB(255,255,255);

	_color = GetIColorSwatch( hwndColor, _colorRef, _td->xdata.displayName );
	assert( _color );
	reset();

	return 5 + wLabel + 40;
}

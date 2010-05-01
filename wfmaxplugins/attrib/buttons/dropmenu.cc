// dropmenu.c

#include <global.hp>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "../util.h"
#include "oas/oad.h"

#include "../oaddlg.h"
#include "dropmenu.h"


DropMenu::DropMenu( typeDescriptor* td ) : uiDialog( td )
{
	_bSave = true;
}


DropMenu::~DropMenu()
{
	assert( hwndMenu );
	DestroyWindow( hwndMenu );

	assert( _nItems > 0 );
	for ( int i=0; i<_nItems; ++i )
	{
		assert( _menuItems[ i ] );
		free( _menuItems[ i ] );
	}
}


int
DropMenu::storedDataSize() const
{
	assert( hwndMenu );
	int idx = ComboBox_GetCurSel( hwndMenu );
	assert( idx != CB_ERR );

	return uiDialog::storedDataSize() + ComboBox_GetTextLength( hwndMenu ) + 1;
}


dataValidation
DropMenu::copy_to_xdata( byte* & saveData )
{
	uiDialog::copy_to_xdata( saveData );

	assert( hwndMenu );
	int idx = ComboBox_GetCurSel( hwndMenu );
	assert( idx != CB_ERR );

	char szText[ 128 ];
	ComboBox_GetText( hwndMenu, szText, sizeof( szText ) );
	strcpy( (char*)saveData, szText );
	saveData += strlen( szText ) + 1;

	return DATA_OK;
}


void
DropMenu::reset()
{
	assert( _td );
	reset( _td->def );
}


void
DropMenu::reset( int i )
{
	assert( 0 <= i && i < _nItems );

	assert( hwndMenu );
	ComboBox_SetCurSel( hwndMenu, i );
}


void
DropMenu::reset( byte* & saveData )
{
	_bUserOverrideEnable = true;

	char* pSaveData = (char*)saveData;
	// Find pSaveData in list of things
	for ( int i=0; i<_nItems; ++i )
	{
		if ( strcmp( _menuItems[ i ], pSaveData ) == 0 )
			break;
	}
	if ( i == _nItems )
		i = atol( pSaveData );
	reset( i );
	saveData += strlen( pSaveData ) + 1;
}


double
DropMenu::eval() const
{
	assert( hwndMenu );
	int idx = ComboBox_GetCurSel( hwndMenu );
	assert( idx != CB_ERR );
	return double( idx );
}


bool
DropMenu::enable( bool bEnabled )
{
	if ( uiDialog::enable( bEnabled ) )
	{
		assert( hwndMenu );
		ComboBox_Enable( hwndMenu, uiDialog::enable() );
		return true;
	}
	else
		return false;
}


int
DropMenu::make_dialog_gadgets( HWND hPanel )
{
	assert( _td );

	assert( hPanel );
	assert( hInstance );
	assert( _td->name );

	int wLabel = uiDialog::make_dialog_gadgets( hPanel );

	_nItems = 0;

	char* strBegin = _td->string;
	char* strSeperator;

	do
		{
		int len;
		strSeperator = strchr( strBegin, SEPARATOR );

		if ( strSeperator )
			len = strSeperator - strBegin;
		else
		{	// Take to end of string
			len = strlen( strBegin );
		}

		_menuItems[ _nItems ] = (char*)malloc( len + 1 );
		assert( _menuItems[ _nItems ] );
		strncpy( _menuItems[ _nItems ], strBegin, len );
		*( _menuItems[ _nItems ] + len ) = '\0';

		++_nItems;

		strBegin = strSeperator + 1;
		}
	while ( strSeperator );

	assert( _nItems > 0 );
	assert( _td->min == 0 );
	assert( _td->max+1 == _nItems );

	long maxWidth = 0;

	{ // Find width of largest
	SIZE size;
	// get the DC for the label
	HDC hdc = GetDC( hwndLabel );
	HFONT hOldFont = (HFONT)SelectObject( hdc, theAttributes._font );
	for ( int i=0; i<_nItems; ++i )
		{
		GetTextExtentPoint32( hdc, _menuItems[ i ], strlen( _menuItems[ i ] ), &size );
		maxWidth = max( maxWidth, size.cx );
		}
	SelectObject( hdc, hOldFont );
	ReleaseDC( hwndLabel, hdc );

	maxWidth += 22;			// size of down button
	}

	hwndMenu = theAttributes.CreateWindowEx(
		0,
		"COMBOBOX",
		_td->name,
		0 | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
		5 + wLabel,
		theAttributes.y,
		maxWidth,				// width
		(_nItems+1)*18,			// height
		hPanel,
		NULL,
		hInstance,
		NULL );
	assert( hwndMenu );
	SetWindowFont( hwndMenu, theAttributes._font, TRUE );

	for ( int i=0; i<_nItems; ++i )
	{
		int err = ComboBox_AddString( hwndMenu, _menuItems[ i ] );
		assert( err != CB_ERR );
	}

	//MoveWindow( hwndMenu, 10+wLabel, y, 100, _nItems*18, FALSE );
	SetWindowLong( hwndMenu, GWL_USERDATA, (LONG)this );

	reset();

	return 5 + wLabel + 100;
}

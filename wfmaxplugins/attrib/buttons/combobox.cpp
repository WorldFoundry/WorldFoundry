// combobox.c

#include <global.hp>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "../util.h"
#include "source/oas/oad.h"

#include "../oaddlg.h"
#include "combobox.h"


ComboBox::ComboBox( typeDescriptor* td ) : uiDialog( td )
{
	_bSave = true;
}


ComboBox::~ComboBox()
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
ComboBox::storedDataSize() const
{
	char buffer[ _MAX_PATH ];
	assert( hwndMenu );
	ComboBox_GetText( hwndMenu, buffer, sizeof( buffer ) );
	return uiDialog::storedDataSize() + strlen( buffer ) + 1;
}



dataValidation
ComboBox::copy_to_xdata( byte* & saveData )
{
	uiDialog::copy_to_xdata( saveData );

	char buffer[ 256 ];
	assert( hwndMenu );
	ComboBox_GetText( hwndMenu, buffer, sizeof( buffer ) );

	strncpy( (char*)saveData, buffer, strlen( buffer ) + 1 );	// include NUL
	saveData += strlen( buffer ) + 1;

	return DATA_OK;
}


void
ComboBox::reset()
{
	assert( _td );
	reset( _td->string );
}


void
ComboBox::reset( char* str )
{
//	assert( 0 <= i && i < _nItems );
	assert( hwndMenu );
	ComboBox_SetText( hwndMenu, str );
}


void
ComboBox::reset( byte* & saveData )
{
	_bUserOverrideEnable = true;

	reset( (char*)saveData );
	saveData += strlen( (char*)saveData ) + 1;
}


void
skip_ws( char* & str )
{
	while ( isspace( *str ) )
		++str;
}


double
ComboBox::eval() const
{
	return 0.0;
}


bool
ComboBox::enable( bool bEnable )
{
	return false;
}


int
ComboBox::make_dialog_gadgets( HWND hPanel )
{
	assert( 0 );

	assert( _td );

	assert( hPanel );
	assert( hInstance );
	assert( _td->name );

	int wLabel = uiDialog::make_dialog_gadgets( hPanel );

	_nItems = 0;

	char szLine[ 256 ];
	char szDefFilename[ _MAX_PATH ];
	sprintf( szDefFilename, "%s/../%s", theAttributes.szOadDir, _td->string );

	FILE* fp = fopen( szDefFilename, "rt" );
	assert( fp );

	while ( !feof( fp ) )
	{
		assert( _nItems < MAX_MENUITEMS );
		char szVariable[ 256 ];
		fgets( szLine, sizeof( szLine ), fp );
		//sscanf( szLine, "( define %s %d )", szVariable, szValue );
		char* pszLine = szLine;
		skip_ws( pszLine );
		if ( *pszLine == '(' )
		{
			++pszLine;		// move past '('
			skip_ws( pszLine );
			if ( _strnicmp( pszLine, "define", strlen( "define" ) ) == 0 )
			{
				pszLine += strlen( "define" );		// move past "define"
				skip_ws( pszLine );
				sscanf( pszLine, "%s", szVariable );

				_menuItems[ _nItems ] = _strdup( szVariable );
				assert( _menuItems[ _nItems ] );
				++_nItems;
			}
		}
	}

	assert( _nItems > 0 );
	assert( _td->min == 0 );
//	assert( _td->max+1 == _nItems );

	int maxWidth = 0;

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
		0 | WS_TABSTOP | CBS_HASSTRINGS | CBS_AUTOHSCROLL | WS_VSCROLL | CBS_DROPDOWN,
		5 + wLabel,
		theAttributes.y,
		100,	//maxWidth,				// width
//		(_nItems+1) * 18,			// height
		min( 10, _nItems+1 ) * 18,			// height
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

	reset();

	return 5 + wLabel + 100;
}

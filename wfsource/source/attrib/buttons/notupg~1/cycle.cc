// cycle.cpp

#include <cstdio>
#include <pigsys/assert.hp>
#include <cstring>
#include <cstdlib>

#include "../util.h"
#include "source/oas/oad.h"

#include "../oaddlg.h"
#include "buttons/cycle.h"


CycleButton::CycleButton( typeDescriptor* td ) : uiDialog( td )
{
	_bSave = true;
	_selected = 0;		//?
}


CycleButton::~CycleButton()
{
	assert( hwndMenu );
	DestroyWindow( hwndMenu );
}


void
CycleButton::activate( HWND hwndButton )
{
	uiDialog::activate( hwndButton );
	if ( hwndButton == hwndMenu )
	{
		if ( GetKeyState( VK_SHIFT ) )
		{
			--_selected;
			if ( _selected < _td->min )
				_selected = _td->max;
		}
		else
		{
			++_selected;
			if ( _selected > _td->max )
				_selected = _td->min;
		}

		reset( _selected );
	}
}


int
CycleButton::storedDataSize() const
{
	assert( hwndMenu );
	char szText[ 128 ];
	Button_GetText( hwndMenu, szText, sizeof( szText ) );

	return uiDialog::storedDataSize() + strlen( szText ) + 1;
}


dataValidation
CycleButton::copy_to_xdata( byte* & saveData )
{
	uiDialog::copy_to_xdata( saveData );

	assert( hwndMenu );
	char szText[ 128 ];
	Button_GetText( hwndMenu, szText, sizeof( szText ) );
	strcpy( (char*)saveData, szText );
	saveData += strlen( szText ) + 1;

	return DATA_OK;
}


void
CycleButton::reset()
{
	assert( _td );
	reset( _td->def );
}


void
CycleButton::reset( int i )
{
	assert( 0 <= i && i < _nItems );
	_selected = i;
	assert( hwndMenu );
	Button_SetText( hwndMenu, _menuItems[ _selected ] );
}


void
CycleButton::reset( byte* & saveData )
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
CycleButton::eval() const
{
	return float( _selected );
}


bool
CycleButton::enable( bool bEnabled )
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
CycleButton::make_dialog_gadgets( HWND hPanel )
{
	assert( _td );

	assert( hPanel );
	assert( hInstance );
	assert( _td->name );

	int wLabel = uiDialog::make_dialog_gadgets( hPanel );

	hwndMenu = theAttributes.CreateWindowEx(
		0,
		"BUTTON",
		_td->name,
		0 | WS_TABSTOP | BS_PUSHBUTTON,
		5 + wLabel,
		theAttributes.y,
		70,		// width
		18,			// height
		hPanel,
		NULL,
		hInstance,
		NULL );
	assert( hwndMenu );
	SetWindowFont( hwndMenu, (HFONT)theAttributes.ip->GetIObjParam()->GetAppHFont(), TRUE );
	SetWindowLong( hwndMenu, GWL_USERDATA, (LONG)this );

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
			{ // Take to end of string
			len = strlen( strBegin );
			}

		char menuItem[ 100 ];
		strncpy( menuItem, strBegin, len );
		*( menuItem + len ) = '\0';

		//int err = SendMessage( hwndMenu, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)menuItem );
		//assert( err != CB_ERR );
		_menuItems[ _nItems ] = _strdup( menuItem );
		assert( _menuItems[ _nItems ] );

		++_nItems;

		strBegin = strSeperator + 1;
	}
	while ( strSeperator );

	assert( _nItems > 0 );
	assert( _td->min == 0 );
	assert( _td->max+1 == _nItems );

	reset();

	return 5 + wLabel + 70;
}

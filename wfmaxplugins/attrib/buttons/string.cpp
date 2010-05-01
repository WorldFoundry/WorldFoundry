// string.cpp

#include <global.hp>
#include "buttons/button.h"
#include "buttons/string.h"

String::String( typeDescriptor* td ) : uiDialog( td )
{
	_bSave = true;
	hwndEdit = NULL;
	_edit = NULL;
}


String::~String()
{
	assert( _edit );
	ReleaseICustEdit( _edit );

	assert( hwndEdit );
	DestroyWindow( hwndEdit );
}


int
String::storedDataSize() const
{
	assert( _edit );
	char buffer[ _MAX_PATH ];
	_edit->GetText( buffer, sizeof( buffer ) );
	return uiDialog::storedDataSize() + strlen( buffer ) + 1;
}


dataValidation
String::copy_to_xdata( byte* & saveData )
{
	uiDialog::copy_to_xdata( saveData );

	assert( _td );
	assert( _edit );

	char buffer[ _MAX_PATH ];
	_edit->GetText( buffer, sizeof( buffer ) );
	strncpy( (char*)saveData, buffer, strlen( buffer ) + 1 );	// include NUL
	saveData += strlen( buffer ) + 1;

	return DATA_OK;
}


void
String::reset()
{
	assert( _td );
	reset( _td->string );
}


void
String::reset( char* str )
{
	assert( str );
	assert( _edit );
	_edit->SetText( str );
}


void
String::reset( byte* & saveData )
{
	_bUserOverrideEnable = true;
	reset( (char*)saveData );
	saveData += strlen( (char*)saveData ) + 1;
}


double
String::eval() const
{
	assert( _edit );
	char buffer[ _MAX_PATH ];
	_edit->GetText( buffer, sizeof( buffer ) );
	return *buffer ? 1.0 : 0.0;
}


bool
String::enable( bool bEnabled )
{
	if ( uiDialog::enable( bEnabled ) )
	{
		assert( hwndEdit );
		Edit_Enable( hwndEdit, uiDialog::enable() );
		return true;
	}
	else
		return false;
}


int
String::make_dialog_gadgets( HWND hPanel )
{
	int x;
	int width;

	assert( _td );

	assert( hPanel );
	assert( hInstance );
	assert( _td->name );

	int wLabel = uiDialog::make_dialog_gadgets( hPanel );

	x = 5 + wLabel;
	width = LAST_BUTTON_WIDTH( x );

	hwndEdit = theAttributes.CreateWindowEx(
		0,
		"CustEdit",
		_td->name,
		0 | WS_TABSTOP,
		x,
		theAttributes.y,
		width,
		16,			// height
		hPanel,
		NULL,
		hInstance,
		NULL );
	assert( hwndEdit );
	_edit = GetICustEdit( hwndEdit );
	assert( _edit );

	reset();

	return x + width;
}

// fixed.cc

#include <cstdio>
#include <pigsys/assert.hp>
#include <cstring>

#include <attrib/util.hp>
#include <oas/oad.h>

#include <attrib/oaddlg.hp>
#include <attrib/buttons/fixed.hp>

///////////////////////////////////////////////////////////////////////////////

Fixed32::Fixed32( typeDescriptor* td ) : uiDialog( td )
{
	_bSave = true;
}


Fixed32::~Fixed32()
{
//	assert( _edit );
//	ReleaseICustEdit( _edit );

//	assert( _spinner );
//	ReleaseISpinner( _spinner );

	assert( hwndEdit );
	DestroyWindow( hwndEdit );

//	assert( hwndSpinner );
//	DestroyWindow( hwndSpinner );
}


void
Fixed32::show_hide( int msgCode )
{
	uiDialog::show_hide( msgCode );
	assert( hwndEdit );
	ShowWindow( hwndEdit, msgCode );
}


int
Fixed32::storedDataSize() const
{
	char szBuffer[ 128 ];
//	_edit->GetText( szBuffer, sizeof( szBuffer ) );
	assert( hwndEdit );
	Edit_GetText( hwndEdit, szBuffer, sizeof( szBuffer ) );

	return uiDialog::storedDataSize() + strlen( szBuffer ) + 1;
}


dataValidation
Fixed32::copy_to_xdata( byte* & saveData )
{
	uiDialog::copy_to_xdata( saveData );

	char szBuffer[ 128 ];
//	assert( _edit );
//	_edit->GetText( szBuffer, sizeof( szBuffer ) );
	assert( hwndEdit );
	Edit_GetText( hwndEdit, szBuffer, sizeof( szBuffer ) );

	strcpy( (char*)saveData, szBuffer );
	saveData += strlen( (char*)saveData ) + 1;

	return DATA_OK;
}


void
Fixed32::reset()
{
	assert( _td );
	reset( (fixed32)_td->def );
}


void
Fixed32::reset( fixed32 data )
{
	char szBuffer[ 128 ];
//	assert( _edit );
//	_edit->SetText( float( data / 65536.0 ) );
	sprintf( szBuffer, "%g", data / 65536.0 );
	Edit_SetText( hwndEdit, szBuffer );

//	assert( _spinner );
//	_spinner->SetLimits( float( _td->min / 65536.0 ), float( _td->max / 65536.0 ) );
//	_spinner->SetResetValue( float( _td->def / 65536.0 ) );
//	_spinner->SetValue( float( data / 65536.0 ), FALSE );
}


void
Fixed32::reset( char* data )
{
//	assert( _edit );
//	_edit->SetText( data );
	assert( hwndEdit );
	Edit_SetText( hwndEdit, data );
}


void
Fixed32::reset( byte* & saveData )
{
	_bUserOverrideEnable = true;
	byte* pSaveData = saveData;
	reset( (char*)pSaveData );
	saveData += strlen( (char*)pSaveData ) + 1;
}


double
Fixed32::eval() const
{
//	assert( _edit );
//	return _edit->GetFloat();

	assert( hwndEdit );
	char szBuffer[ 128 ];
	Edit_GetText( hwndEdit, szBuffer, sizeof( szBuffer ) );

	return atof( szBuffer );
}


bool
Fixed32::enable( bool bEnabled )
{
	if ( uiDialog::enable( bEnabled ) )
	{
//		assert( _edit );
//		uiDialog::enable() ? _edit->Enable() : _edit->Disable();
		assert( hwndEdit );
		Button_Enable( hwndEdit, uiDialog::enable() );
//		assert( _spinner );
//		uiDialog::enable() ? _spinner->Enable() : _spinner->Disable();
		return true;
	}
	else
		return false;
}


int
Fixed32::make_dialog_gadgets( HWND hPanel )
{
	int x;
	int width;

	assert( _td );

	assert( hPanel );
	assert( hInstance );
	assert( _td->name );

	int wLabel = uiDialog::make_dialog_gadgets( hPanel );

	x = 5 + wLabel;
	width = min( LAST_BUTTON_WIDTH( x + 12 + 1 ), 55 );
	hwndEdit = theAttributes->CreateWindowEx(
		0,
//		"CustEdit",
		"EDIT",
		_td->name,
		0 | WS_TABSTOP,
		x,
		theAttributes->y,
		width, 16,
		hPanel );
	assert( hwndEdit );
	Button_SetFont( hwndEdit, theAttributes->_font );
#if 0
	_edit = GetICustEdit( hwndEdit );
	assert( _edit );

	hwndSpinner = theAttributes.CreateWindowEx(
		0,
		"SpinnerControl",
		"",
		0x50000000,
		x + width + 1,
		theAttributes.y,
		7,
		10,
		hPanel,
		NULL,
		hInstance,
		NULL );
	assert( hwndSpinner );
	_spinner = GetISpinner( hwndSpinner );
	assert( _spinner );
#endif

	reset();

#if 0
	_spinner->LinkToEdit( hwndEdit, EDITTYPE_FLOAT );
	_spinner->SetAutoScale();
#endif

	return x + width + 1 + 7;
}

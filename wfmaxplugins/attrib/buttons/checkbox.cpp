// checkbox.cpp

#include <global.hp>
#include "button.h"
#include "checkbox.h"


CheckBox::CheckBox( typeDescriptor* td ) : uiDialog( td )
{
	_bSave = true;
}


CheckBox::~CheckBox()
{
	assert( hwndCheckbox );
	DestroyWindow( hwndCheckbox );
}


int
CheckBox::storedDataSize() const
{
	return uiDialog::storedDataSize() + strlen( "0" ) + 1;
}


dataValidation
CheckBox::copy_to_xdata( byte* & saveData )
{
	uiDialog::copy_to_xdata( saveData );

	assert( hwndCheckbox );
	int32 i = Button_GetCheck( hwndCheckbox );
	strcpy( (char*)saveData, i ? "1" : "0" );
	saveData += strlen( (char*)saveData ) + 1;

	return DATA_OK;
}


void
CheckBox::reset()
{
	assert( _td );
	reset( _td->def );
}


void
CheckBox::reset( int i )
{
	assert( hwndCheckbox );
	Button_SetCheck( hwndCheckbox, i );
}


void
CheckBox::reset( byte* & saveData )
{
	_bUserOverrideEnable = true;

	char* pSaveData = (char*)saveData;
	int bChecked = atoi( pSaveData );
	reset( bChecked );
	saveData += strlen( pSaveData ) + 1;
}


double
CheckBox::eval() const
{
	assert( hwndCheckbox );
	int32 i = Button_GetCheck( hwndCheckbox );
	return double( i ? 1.0 : 0.0 );
}


bool
CheckBox::enable( bool bEnabled )
{
	if ( uiDialog::enable( bEnabled ) )
	{
		assert( hwndCheckbox );
		Button_Enable( hwndCheckbox, uiDialog::enable() );
		return true;
	}
	else
		return false;
}


int
CheckBox::make_dialog_gadgets( HWND hPanel )
{
	assert( _td );

	assert( hPanel );
	assert( hInstance );
	assert( _td->name );

	int wLabel = uiDialog::make_dialog_gadgets( hPanel );

	hwndCheckbox = theAttributes.CreateWindowEx(
		0,
		"BUTTON",
		_td->xdata.displayName,
		0 | WS_TABSTOP | BS_AUTOCHECKBOX,
		5 + wLabel,
		theAttributes.y,
		16,		// width
		16,			// height
		hPanel );
	assert( hwndCheckbox );
	SetWindowFont( hwndCheckbox, (HFONT)theAttributes.ip->GetIObjParam()->GetAppHFont(), TRUE );

	reset();

	return 5 + wLabel + 16;
}

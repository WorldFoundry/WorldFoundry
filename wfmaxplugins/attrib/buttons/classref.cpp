// classref.cpp

#include <global.hp>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "../util.h"
#include "source/oas/oad.h"

#include "../oaddlg.h"
#include "classref.h"


ClassReference::ClassReference( typeDescriptor* td ) : uiDialog( td )
{
	_bSave = true;
}


ClassReference::~ClassReference()
{
	assert( hwndMenu );
	DestroyWindow( hwndMenu );
}


int
ClassReference::storedDataSize() const
{
	char szClassName[ _MAX_PATH ] = { 0 };

	assert( hwndMenu );
	int idx = ComboBox_GetCurSel( hwndMenu );
	if ( idx != CB_ERR )
	{
		int ret = ComboBox_GetLBText( hwndMenu, idx, szClassName );
		assert( ret != CB_ERR );
	}

	return uiDialog::storedDataSize() + strlen( szClassName ) + 1;
}


dataValidation
ClassReference::copy_to_xdata( byte* & saveData )
{
	char szClassName[ _MAX_PATH ] = { 0 };

	uiDialog::copy_to_xdata( saveData );

	assert( hwndMenu );
	int idx = ComboBox_GetCurSel( hwndMenu );
	if ( idx != CB_ERR )
	{
		int ret = ComboBox_GetLBText( hwndMenu, idx, szClassName );
		assert( ret != CB_ERR );
	}

	strncpy( (char*)saveData, szClassName, strlen( szClassName ) + 1 );
	saveData += strlen( szClassName ) + 1;

	return DATA_OK;
}


void
ClassReference::reset( char* szSelectedClass )
{
	if ( *szSelectedClass )
	{
		int index = ComboBox_FindStringExact( hwndMenu, -1, szSelectedClass );
		assert( index != CB_ERR );
		ComboBox_SetCurSel( hwndMenu, index );
	}
}


void
ClassReference::reset( byte* & saveData )
{
	_bUserOverrideEnable = true;

	byte* pSaveData = saveData;
	reset( (char*)pSaveData );
	saveData += strlen( (char*)saveData ) + 1;
}


bool
ClassReference::enable( bool bEnabled )
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


double
ClassReference::eval() const
{
	char szClassName[ _MAX_PATH ];

	assert( hwndMenu );
	int idx = ComboBox_GetCurSel( hwndMenu );
	if ( idx != CB_ERR )
	{
		int ret = ComboBox_GetLBText( hwndMenu, idx, szClassName );
		assert( ret != CB_ERR );
	}

	return *szClassName ? 1.0 : 0.0;
}


int
ClassReference::make_dialog_gadgets( HWND hPanel )
{
	assert( _td );

	assert( hPanel );
	assert( hInstance );
	assert( _td->name );

	int wLabel = uiDialog::make_dialog_gadgets( hPanel );

	hwndMenu = theAttributes.CreateWindowEx(
		0,
		"COMBOBOX",
		_td->name,
		0 | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_SORT,
		5 + wLabel,
		theAttributes.y,
		100,				// width
		200,			// height
		hPanel,
		NULL,
		hInstance,
		NULL );
	assert( hwndMenu );
	theAttributes.FillInClassReference( hwndMenu );

	// No default from _typeDescriptor

	return 5 + wLabel + 100;
}

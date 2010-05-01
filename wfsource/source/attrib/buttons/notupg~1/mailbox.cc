// mailbox.cpp

#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <pigsys/assert.hp>
#include <cstring>

#include "../util.h"
#include "source/oas/oad.h"

#include "../oaddlg.h"
#include "mailbox.h"

Mailbox::Mailbox( typeDescriptor* td ) : uiDialog( td )
{
	_bSave = true;
}


Mailbox::~Mailbox()
{
	assert( _edit );
	ReleaseICustEdit( _edit );

	assert( _spinner );
	ReleaseISpinner( _spinner );

	assert( hwndEdit );
	DestroyWindow( hwndEdit );

	assert( hwndSpinner );
	DestroyWindow( hwndSpinner );
}


int
Mailbox::storedDataSize() const
{
	return uiDialog::storedDataSize() + sizeof( long );
}


double
Mailbox::eval() const
{
	return 0.0;
}


bool
Mailbox::enable( bool bEnable )
{
	return false;
}


int
Mailbox::make_dialog_gadgets( HWND hPanel )
{
	int x;
	int width;

	assert( _td );

	assert( hPanel );
	assert( hInstance );
	assert( _td->name );

	int wLabel = uiDialog::make_dialog_gadgets( hPanel );

	hwndEdit = theAttributes.CreateWindowEx(
		0,
		"CustEdit",
		_td->name,
		0 | WS_TABSTOP,
		x = 5 + wLabel,
		theAttributes.y,
		width = 40,
		16,			// height
		hPanel,
		NULL,
		hInstance,
		NULL );
	assert( hwndEdit );
	_edit = GetICustEdit( hwndEdit );
	assert( _edit );

	hwndSpinner = theAttributes.CreateWindowEx(
		0,
		"SpinnerControl",
		"",
		0,
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

	reset();

	_spinner->LinkToEdit( hwndEdit, EDITTYPE_INT );
	_spinner->SetAutoScale();

	return x + width + 1 + 7;
}

///////////////////////////////////////////////////////////////////////////////

dataValidation
Mailbox::copy_to_xdata( byte* & saveData )
{
	int i;

	assert( _td );

	assert( _edit );
	i = _edit->GetInt();

	uiDialog::copy_to_xdata( saveData );
	(*(long*)saveData) = i;		saveData += 4;

	return DATA_OK;
}


void
Mailbox::reset()
{
	assert( _td );
	reset( _td->def );
}


void
Mailbox::reset( long data )
{
	assert( _td );

	assert( _edit );
	_edit->SetText( data );

	assert( _spinner );
	_spinner->SetLimits( _td->min, _td->max );
	_spinner->SetResetValue( _td->def );
	_spinner->SetValue( data, FALSE );
}


void
Mailbox::reset( byte* & saveData )
{
	_bUserOverrideEnable = true;
	byte* pSaveData = saveData;
	reset( *((long*)pSaveData) );
	saveData += sizeof( int );
}

///////////////////////////////////////////////////////////////////////////////

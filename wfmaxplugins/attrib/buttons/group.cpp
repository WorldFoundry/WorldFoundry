// group.cpp

#include <global.hp>
#include "buttons/button.h"
#include "buttons/group.h"

///////////////////////////////////////////////////////////////////////////////

Group::Group( typeDescriptor* td ) : uiDialog( td )
{
}


Group::~Group()
{
	assert( hwndGroup );
	DestroyWindow( hwndGroup );
}


int
Group::storedDataSize() const
{
	assert( 0 );
	return 0;
}


dataValidation
Group::copy_to_xdata( byte* & saveData )
{
	assert( 0 );
	return DATA_OK;
}


void
Group::reset()
{
}


void
Group::reset( byte* & saveData )
{
}


void
Group::endGroup()
{
	assert( hwndGroup );
	assert( theAttributes.szGroupName );
	SetWindowPos( hwndGroup, HWND_TOP, 0, 0, 157, theAttributes.y-_yStart-2, SWP_NOMOVE | SWP_DRAWFRAME );
	theAttributes.szGroupName = NULL;

	theAttributes.x -= _GROUP_X_INC;
}


double
Group::eval() const
{
	return 1.0;
}


bool
Group::enable( bool bEnabled )
{
	return true;

	if ( uiDialog::enable( bEnabled ) )
	{
		return true;
	}
	else
		return false;

#if 0
	if ( bEnabled != _bEnabled )
	{
		assert( hwndGroup );
		Static_Enable( hwndGroup, bEnabled );
		uiDialog::enable( bEnabled );
		return true;
	}
	else
		return false;
#endif
}


int
Group::make_dialog_gadgets( HWND hPanel )
{
	assert( _td );
	assert( hPanel );
	assert( hInstance );
	assert( _td->name );

	strcpy( _td->xdata.displayName, _td->name );

	assert( !theAttributes.szGroupName );
	theAttributes.szGroupName = _td->name;

	_yStart = theAttributes.y;

	hwndGroup = theAttributes.CreateWindowEx(
		0,
		"STATIC",
		_td->name,
		0 | SS_ETCHEDFRAME,
		3,
		theAttributes.y+6,
		157,			// width
		100,			// height
		hPanel );
	assert( hwndGroup );
	SetWindowFont( hwndGroup, (HFONT)theAttributes.ip->GetIObjParam()->GetAppHFont(), FALSE );

	uiDialog::make_dialog_gadgets( hPanel );

	reset();

	theAttributes.x += _GROUP_X_INC;

	return 0;
}

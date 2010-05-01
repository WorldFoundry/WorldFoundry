// group.cc

#include <attrib/buttons/group.hp>
extern char* szGroupName;

///////////////////////////////////////////////////////////////////////////////

#pragma message( "TODO: Groups need to track what's inside of them for:" )
#pragma message( "1] re-size width" )
#pragma message( "2] enable/disable children when group enabled/disabled" )
#pragma message( "3] I forgot" )

Group::Group( typeDescriptor* td ) : uiDialog( td )
{
}


Group::~Group()
{
	assert( hwndGroup );
	DestroyWindow( hwndGroup );
}


void
Group::show_hide( int msgCode )
{
	uiDialog::show_hide( msgCode );
	assert( hwndGroup );
	ShowWindow( hwndGroup, msgCode );
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
	assert( szGroupName );
	SetWindowPos( hwndGroup, HWND_TOP, 0, 0, 160, theAttributes->y-_yStart-2, SWP_NOMOVE | SWP_DRAWFRAME );
	szGroupName = NULL;

	theAttributes->x -= _GROUP_X_INC;
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

	assert( !szGroupName );
	szGroupName = _td->name;

	_yStart = theAttributes->y;

	hwndGroup = theAttributes->CreateWindowEx(
		0,
		"STATIC",
		_td->name,
		0 | SS_ETCHEDFRAME,
		3,
		theAttributes->y+6,
		157,			// width
		100,			// height
		hPanel );
	assert( hwndGroup );
//	SetWindowFont( hwndGroup, (HFONT)theAttributes.ip->GetIObjParam()->GetAppHFont(), FALSE );

	uiDialog::make_dialog_gadgets( hPanel );

	reset();

	theAttributes->x += _GROUP_X_INC;

	return 0;
}

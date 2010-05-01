// objref.cpp

#include "button.h"
#include "objref.h"

ObjectReference::ObjectReference( typeDescriptor* td ) : uiDialog( td )
{
	_bSave = true;
	hwndEdit = NULL;
	_edit = NULL;
	hwndBrowseButton = NULL;
	hwndGoToObjectButton = NULL;
}


ObjectReference::~ObjectReference()
{
	assert( _edit );
	ReleaseICustEdit( _edit );

	assert( hwndEdit );
	DestroyWindow( hwndEdit );

	assert( hwndBrowseButton );
	DestroyWindow( hwndBrowseButton );

	assert( hwndGoToObjectButton );
	DestroyWindow( hwndGoToObjectButton );
}


int
ObjectReference::storedDataSize() const
{
	assert( _edit );
	char buffer[ _MAX_PATH ];
	_edit->GetText( buffer, sizeof( buffer ) );
	return uiDialog::storedDataSize() + strlen( buffer ) + 1;
}


bool
ObjectReference::validReference( const char* szObjectReference ) const
{
	assert( 0 );
	return bool(
		(*szObjectReference == '\0')
		|| theAttributes.ip->GetINodeByName( szObjectReference )
		);
}


dataValidation
ObjectReference::copy_to_xdata( byte* & saveData )
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
ObjectReference::reset()
{
	assert( _td );
	reset( _td->string );
}


void
ObjectReference::reset( char* str )
{
	assert( str );
	assert( _edit );
	_edit->SetText( str );
}


void
ObjectReference::reset( byte* & saveData )
{
	_bUserOverrideEnable = true;
	enable( uiDialog::enable() );

	reset( (char*)saveData );
	saveData += strlen( (char*)saveData ) + 1;
}


bool
ObjectReference::updateGoToObjectButton() const
{
	char szObjectName[ 256 ];
	assert( _edit );
	_edit->GetText( szObjectName, sizeof( szObjectName ) );
	INode* pNode = theAttributes.ip->GetINodeByName( szObjectName );

	assert( hwndGoToObjectButton );
	Button_Enable( hwndGoToObjectButton, bool( pNode ) );

	return bool( pNode );
}


void
ObjectReference::activate( HWND hwndButton )
{
	uiDialog::activate( hwndButton );

#if 0
	char szObjectName[ 256 ];
	assert( _edit );
	_edit->GetText( szObjectName, sizeof( szObjectName ) );
	INode* pNode = theAttributes.ip->GetINodeByName( szObjectName );

	assert( hwndGoToObjectButton );
	Button_Enable( hwndGoToObjectButton, bool( pNode ) );
#endif

	if ( hwndButton == hwndBrowseButton )
	{
		TrackViewPick pick;
		if ( theAttributes.ip->TrackViewPickDlg( _hPanel, &pick ) )
			reset( pick.anim->NodeName() );
	}
	else if ( hwndButton == hwndGoToObjectButton )
	{
		char szObjectName[ 256 ];
		assert( _edit );
		_edit->GetText( szObjectName, sizeof( szObjectName ) );
		INode* pNode = theAttributes.ip->GetINodeByName( szObjectName );
		assert( pNode );

		theHold.Begin();
		theAttributes.ip->SelectNode( pNode );
		static char szUndoMessage[ 512 ];
		sprintf( szUndoMessage, "Attributes - go to %s", szObjectName );
		theHold.Accept( szUndoMessage );
	}
}


double
ObjectReference::eval() const
{
#if 1

	char szObjectName[ 256 ];
	assert( _edit );
	_edit->GetText( szObjectName, sizeof( szObjectName ) );
	INode* pNode = theAttributes.ip->GetINodeByName( szObjectName );
	updateGoToObjectButton();

	return *szObjectName ? ( pNode ? 2.0 : 0.0 ) : 0.0;

#else

	assert( _edit );
	char buffer[ _MAX_PATH ];
	_edit->GetText( buffer, sizeof( buffer ) );
	return *buffer ? 1.0 : 0.0;

#endif
}


bool
ObjectReference::enable( bool bEnabled )
{
	bool ret = false;
	if ( uiDialog::enable( bEnabled ) )
	{
		assert( hwndEdit );
		Edit_Enable( hwndEdit, uiDialog::enable() );
		assert( hwndBrowseButton );
		Button_Enable( hwndBrowseButton, uiDialog::enable() );
		updateGoToObjectButton();
		return true;
	}

	eval();
	return ret;
}


int
ObjectReference::make_dialog_gadgets( HWND hPanel )
{
	int x;
	int width;

	assert( _td );

	assert( hPanel );
	assert( hInstance );
	assert( _td->name );

	int wLabel = uiDialog::make_dialog_gadgets( hPanel );

	x = 5 + wLabel;
	width = LAST_BUTTON_WIDTH( x + 16+1 + 16+1 );

	//theAttributes.y += 18;

	hwndEdit = theAttributes.CreateWindowEx(
		0,
		"CustEdit",
		_td->name,
		0 | WS_TABSTOP,
		x,
		theAttributes.y,
		width,
		16,
		hPanel );
	assert( hwndEdit );
	_edit = GetICustEdit( hwndEdit );
	assert( _edit );

	hwndBrowseButton = theAttributes.CreateWindowEx(
		0,
		"BUTTON",
		"8",
		0,
		x + width + 1,
		theAttributes.y,
		16,
		16,
		hPanel );
	assert( hwndBrowseButton );
	SetWindowLong( hwndBrowseButton, GWL_USERDATA, (LONG)this );
	SetWindowFont( hwndBrowseButton, theAttributes._fontWingdings, true );

	char szButton[ 2 ];
	szButton[ 0 ] = 240;
	szButton[ 1 ] = '\0';

	hwndGoToObjectButton = theAttributes.CreateWindowEx(
		0,
		"BUTTON",
		szButton,
		0,
		x + width + 16+1 + 1,
		theAttributes.y,
		16,
		16,
		hPanel );
	assert( hwndGoToObjectButton );
	SetWindowLong( hwndGoToObjectButton, GWL_USERDATA, (LONG)this );
	SetWindowFont( hwndGoToObjectButton, theAttributes._fontWingdings, true );

	reset();

	return x + width + 1 + 16 + 16;
}



#if 0
void
ObjectReference::activate( HWND hwndButton )
{
	uiDialog::activate( hwndButton );

	char szObjectName[ 256 ];
	assert( _edit );
	_edit->GetText( szObjectName, sizeof( szObjectName ) );
	INode* pNode = theAttributes.ip->GetINodeByName( szObjectName );

	assert( hwndGoToObjectButton );
	Button_Enable( hwndGoToObjectButton, bool( pNode ) );

	if ( hwndButton == hwndBrowseButton )
	{
		TrackViewPick pick;
		if ( theAttributes.ip->TrackViewPickDlg( _hPanel, &pick ) )
			reset( pick.anim->NodeName() );
	}
	else if ( hwndButton == hwndGoToObjectButton )
	{
		assert( pNode );
		theHold.Begin();
		theAttributes.ip->SelectNode( pNode );
		char szUndoMessage[ 512 ];
		sprintf( szUndoMessage, "Attributes - go to %s", szObjectName );
		theHold.Accept( szUndoMessage );
	}
}

bool
ObjectReference::enable( bool bEnabled )
{
	bool ret = false;

	if ( uiDialog::enable( bEnabled ) )
	{
		assert( hwndEdit );
		Edit_Enable( hwndEdit, uiDialog::enable() );
		assert( hwndBrowseButton );
		Button_Enable( hwndBrowseButton, uiDialog::enable() );
		Button_Enable( hwndGoToObjectButton, uiDialog::enable() );
		ret = true;
	}

	eval();
	return ret;
}
#endif

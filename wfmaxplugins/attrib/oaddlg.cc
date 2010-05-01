// oaddlg.cpp

#include <global.hp>
#include "oaddlg.h"
#include <eval/eval.h>

_IffWriter&
uiDialog::_print( _IffWriter& iff ) const
{
	iff.enterChunk( ID( "STRU" ) );

		iff.enterChunk( ID( "NAME" ) );
			iff << fieldName();
		iff.exitChunk();

		iff.enterChunk( ID(  ) );
			//iff <<
		iff.exitChunk();

	iff.exitChunk();

	return iff;
}


uiDialog::uiDialog( typeDescriptor* td )
{
	_td = td;
	_bSave = false;
	hwndLabel = NULL;
	_hwndMenuButton = NULL;
	_hPanel = NULL;
#if defined( TOOLTIPS )
	_label = NULL;
#endif
	_bEnabled = true;
	_bLastEnable = true;
	_bUserOverrideEnable = false;
	// construct _szVariableName by removing all spaces
	const char* szFieldName = fieldName();
	char* pszVariableName = _szVariableName;
	for ( ; *szFieldName; ++szFieldName )
	{
		if ( *szFieldName != ' ' )
			*pszVariableName++ = *szFieldName;
	}
	*pszVariableName = '\0';
}


uiDialog::~uiDialog()
{
#if defined( TOOLTIPS )
	assert( _label );
	ReleaseICustButton( _label );
#endif
	assert( _td );
	if ( _td->type != BUTTON_PROPERTY_SHEET )
	{
		if ( _hwndMenuButton )
			DestroyWindow( _hwndMenuButton );

		assert( hwndLabel );
		DestroyWindow( hwndLabel );
	}
}


bool
uiDialog::enable() const
{
	return _bUserOverrideEnable && _bEnabled;
}


bool
uiDialog::enable( bool bEnabled )
{
	if ( _hwndMenuButton )
	{
		Button_Enable( _hwndMenuButton, bEnabled );
		Button_SetCheck( _hwndMenuButton, _bUserOverrideEnable );
	}

	if ( (_bUserOverrideEnable && bEnabled) != _bLastEnable )
	{
		_bEnabled = bEnabled;
		_bLastEnable = enable();
		if ( hwndLabel )
			Static_Enable( hwndLabel, _bLastEnable );
		return true;
	}
	else
		return false;
}


double
fnSymbolLookup( const char* yytext )
{
	uiDialog* field = theAttributes.findGadget( yytext );
	return field ? field->eval() : 0.0;
}


bool
uiDialog::checkEnable()
{
	//bool b = bool( ::eval( _td->xdata.szEnableExpression, fnSymbolLookup ) );
	bool b = true;
	enable( b );
	return b;
}


bool
uiDialog::canSave() const
{
	return _bSave && _bUserOverrideEnable;
}


void
uiDialog::activate( HWND hwnd )
{
	if ( _hwndMenuButton )
		_bUserOverrideEnable = Button_GetCheck( _hwndMenuButton );

#if 0
	if ( hwnd == _hwndMenuButton )
	{
//		UINT id = _td->type == BUTTON_INT32 ? IDM_RMENU_INT : IDM_RMENU;
		UINT id = IDM_RMENU;

		assert( hInstance );
		HMENU hmenu = LoadMenu( hInstance, MAKEINTRESOURCE( id ) );
		assert( hmenu );
		HMENU hmenuPopup = GetSubMenu( hmenu, 0 );
		assert( hmenuPopup );

		POINT point;
		point.x = 0;
		point.y = 0;
		ClientToScreen( hwnd, &point );

		CheckMenuItem( hmenuPopup, IDM_ENABLED, _bUserOverrideEnable );
		CheckMenuItem( hmenuPopup, IDM_ENABLED, FALSE );
//		EnableMenuItem( hmenuPopup, 0, FALSE );
		EnableMenuItem( hmenuPopup, 2, _bUserOverrideEnable );

		HWND hDlg = GetParent( hwnd );
		assert( hDlg );

//		TrackPopupMenu( hmenuPopup, 0, point.x, point.y, 0, hDlg, NULL );
		TrackPopupMenu( hmenuPopup, 0, point.x, point.y, 0, theAttributes._hPanelClass, NULL );

		MENUITEMINFO mii;
		mii.fMask = MIIM_STATE;
		GetMenuItemInfo( hmenuPopup, IDM_ENABLED, FALSE, &mii );
		//_bUserOverrideEnable = !( mii.fState & MFS_CHECKED );

		_bUserOverrideEnable = !_bUserOverrideEnable;
		theAttributes.refreshGadgetEnabledStates();

		DestroyMenu( hmenu );
	}
#endif
}


dataValidation
uiDialog::copy_to_xdata( byte* & saveData )
{
	assert( canSave() );
	assert( _td );
	(*(long*)saveData) = storedDataSize();	saveData += 4;
	(*(long*)saveData) = _td->type;	saveData += 4;
	(*(long*)saveData) = _td->showAs; saveData += 4;
	strncpy( (char*)saveData, _td->name, strlen( _td->name ) + 1 );	// include NUL
	saveData += strlen( _td->name ) + 1;

	return DATA_OK;
}


int
uiDialog::make_dialog_gadgets( HWND hPanel )
{
	assert( hPanel );
	_hPanel = hPanel;

	assert( _td );

	if ( _td->type != BUTTON_GROUP_START )
	{
		_hwndMenuButton = theAttributes.CreateWindowEx(
			0,
			"BUTTON",
			"",
			0 | WS_TABSTOP | BS_AUTOCHECKBOX,
			3,
			theAttributes.y+3,
			10,		// width
			10,			// height
			hPanel,
			NULL,
			hInstance,
			NULL );
		assert( _hwndMenuButton );
		SetWindowLong( _hwndMenuButton, GWL_USERDATA, (LONG)this );
		Button_SetCheck( _hwndMenuButton, _bEnabled && _bUserOverrideEnable );
	}

	int x = 15;

	hwndLabel = theAttributes.CreateWindowEx(
		0,
#if defined( TOOLTIPS )
		"CustButton",
#else
		"STATIC",
#endif
		_td->xdata.displayName,
		0,
		x,
		theAttributes.y,
		0,
		16,
		hPanel );
	assert( hwndLabel );
	SetWindowFont( hwndLabel, theAttributes._font, TRUE );

	SIZE size;
	// get the DC for the label
	HDC hdc = GetDC( hwndLabel );
	HFONT hOldFont = (HFONT)SelectObject( hdc, theAttributes.ip->GetIObjParam()->GetAppHFont() );
	GetTextExtentPoint32( hdc, _td->xdata.displayName, strlen( _td->xdata.displayName ), &size );
	SelectObject( hdc, hOldFont );
	ReleaseDC( hwndLabel, hdc );
	SetWindowPos( hwndLabel, HWND_TOP, 0, 0, size.cx, 16, SWP_NOMOVE );

#if defined( TOOLTIPS )
	_label = GetICustButton( hwndLabel );
	assert( _label );
	if ( *_td->helpMessage )
		_label->SetTooltip( TRUE, _td->helpMessage );
#endif

	return x + size.cx;
}



int
uiDialog::storedDataSize() const
{
	assert( canSave() );
	return sizeof( int ) + sizeof( int ) + sizeof( int ) + strlen( fieldName() ) + 1;
}


const char*
uiDialog::variableName() const
{
	return _szVariableName;
}


const char*
uiDialog::fieldName() const
{
	return _td->name;
}


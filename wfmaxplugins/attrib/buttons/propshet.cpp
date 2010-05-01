// propshet.cpp

#include <global.hp>
#include "button.h"
#include "propshet.h"

RollUp::RollUp( typeDescriptor* td ) : uiDialog( td )
{
	extern typeDescriptor* pEndOfTypeDescriptors;

	_bSave = true;
	if ( ( _nLinesInRollup = td->xdata.rollUpLength ) == 0 )
	{	// Calculate
		for ( ++td; td < pEndOfTypeDescriptors && td->type != BUTTON_PROPERTY_SHEET; ++td )
		{
			if ( td->showAs != SHOW_AS_HIDDEN )
				++_nLinesInRollup;
//			if ( td->type == BUTTON_GROUP_START )
//				--_nLinesInRollup;
			if ( td->type == BUTTON_MESHNAME )
			{	// Mesh name uses two lines
				++_nLinesInRollup;
			}
		}
	}
}


RollUp::~RollUp()
{
	assert( _hPanel );
	theAttributes.ip->DeleteRollupPage( _hPanel );
}


int
RollUp::storedDataSize() const
{
	return uiDialog::storedDataSize() + sizeof( int32 );
}


dataValidation
RollUp::copy_to_xdata( byte* & saveData )
{
	uiDialog::copy_to_xdata( saveData );

#if 1
	int32 i = 0;
#else
	assert( _hPanel );
	int32 i = IsRollupPanelOpen( _hPanel );

	{
	assert( _hPanel );
	HWND mainPanel = GetParent( GetParent( GetParent( _hPanel ) ) );
	assert( mainPanel );
	IRollupWindow* pRollup = GetIRollup( mainPanel );
	assert( pRollup );
	int idxRollup = pRollup->GetPanelIndex( _hPanel );
	assert( IsRollupPanelOpen( _hPanel ) == pRollup->IsPanelOpen( idxRollup ) );
	ReleaseIRollup( pRollup );
	}
#endif

	(*(int32*)saveData) = i, saveData += sizeof( int32 );

	return DATA_OK;
}


void
RollUp::reset()
{
	assert( _td );
	reset( _td->def );
}


void
RollUp::reset( int32 i )
{
	assert( _td->min <= i && i <= _td->max );

#if 0
	assert( _hPanel );
	HWND mainPanel = GetParent( GetParent( GetParent( _hPanel ) ) );
	assert( mainPanel );
	IRollupWindow* pRollup = GetIRollup( mainPanel );
	assert( pRollup );

	int idxRollup = pRollup->GetPanelIndex( _hPanel );

#if 0
	pRollup->SetPanelOpen( pRollup->GetPanelIndex( _hPanel ), bool( i ) );
	pRollup->SetPanelOpen( 1, FALSE );
	pRollup->SetPanelOpen( 2, FALSE );
#endif

	pRollup->SetPanelOpen( idxRollup, TRUE );
	pRollup->SetPanelOpen( 0, TRUE );

	ReleaseIRollup( pRollup );
#endif
}


void
RollUp::reset( byte* & saveData )
{
	_bUserOverrideEnable = true;
	byte* pSaveData = saveData;
	reset( *((int32*)pSaveData) );
	saveData += sizeof( int32 );
}


extern BOOL CALLBACK AttributesDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


double
RollUp::eval() const
{
	assert( _hPanel );
	return double( IsRollupPanelOpen( _hPanel ) );
}


bool
RollUp::enable( bool )
{
	return uiDialog::enable( true );
}


HWND retPanel;			// temp hack

int
RollUp::make_dialog_gadgets( HWND hPanel )
{
	enum { MAX_Y = 36 };

	assert( _td );
	assert( hInstance );

	_nLinesInRollup = min( _nLinesInRollup, MAX_Y );
	_nLinesInRollup = max( _nLinesInRollup, 1 );

	retPanel =
	hPanel =
	_hPanel = theAttributes.ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE( _nLinesInRollup ),
		AttributesDlgProc,
		_td->xdata.displayName,
		0,
		_td->def ? 0 : APPENDROLL_CLOSED );
	assert( _hPanel );

	theAttributes.x = theAttributes.y = 0;

	assert( hPanel );
	assert( _td->name );

	reset();

	return 0;
}

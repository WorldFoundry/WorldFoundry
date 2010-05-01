#include <stdarg.h>
#include <windowsx.h>
#include "select.h"

////////////////////////////////////////////////////////////////////////////////

Column::Column( HWND hwnd, const char* szText, int iSubItem )
{
	assert( hwnd );
	_hwnd = hwnd;

	assert( szText );
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 75;
	lvc.pszText = (char*)szText;
	lvc.iSubItem = iSubItem;
	int error = ListView_InsertColumn( _hwnd, lvc.iSubItem, &lvc );
	assert( error != -1 );

	_szLabel = _strdup( szText );
	assert( _szLabel );

	_idMenu = Select::IDM_ARRANGEBY + iSubItem;

//	char szMenuItem[ 100 ];
//	sprintf( szMenuItem, "by &%s", GetText() );
//	assert( theSelect._menuArrangeBy );
//	AppendMenu( theSelect._menuArrangeBy, MF_STRING, Select::IDM_ARRANGEBY + iSubItem, szMenuItem );
}


Column::~Column()
{
//#pragma message( __FILE__ ": remove this column from Sort By -> menu" )
	assert( _szLabel );
	free( _szLabel );
}


const char*
Column::GetText() const
{
	return _szLabel;
}

#include <stdarg.h>
#include <windowsx.h>
#include <io.h>
#include <xstddef>
#include <algorithm>
#include <vector>
#include <string>
#include "../lib/registry.h"
#include "select.h"
#include <oas/oad.h>
#include "../lib/wf_id.hp"

enum { WM_COLUMN_RBUTTONDOWN = WM_USER + 550 };

Select theSelect;
extern HINSTANCE hInstance;

////////////////////////////////////////////////////////////////////////////////

void
Error( const char* szMsg )
{
	assert( szMsg );
	MessageBox( GetCOREInterface()->GetMAXHWnd(), szMsg, "Select", MB_OK );
}

////////////////////////////////////////////////////////////////////////////////

// yuk

static void
UpdateStatistics( HWND hDlg )
{
	static int nLastSelected = -1;

	assert( theSelect.hwndList );
	int nSelected = ListView_GetSelectedCount( theSelect.hwndList );

	if ( nSelected != nLastSelected )
	{
		nLastSelected = nSelected;

		HWND hwndText = GetDlgItem( hDlg, IDC_NUMSELECTED );
		assert( hwndText );

		char szBuffer[ 100 ];
		if ( nSelected )
			sprintf( szBuffer, "%d selected", nSelected );
		else
			*szBuffer = '\0';
		Static_SetText( hwndText, szBuffer );
	}
}

////////////////////////////////////////////////////////////////////////////////

void WaitForMaxWindow();

Select::Select()
{
	ip = GetCOREInterface();
	assert( ip );

	_szSelect.push_back( _strdup( "!Name" ) );
	_szSelectDisplayName.push_back( _strdup( "!Name" ) );
	_szSelectHelp.push_back( _strdup( "Name of Object [MAX]" ) );

	_szSelect.push_back( _strdup( "!Class" ) );
	_szSelectDisplayName.push_back( _strdup( "!Class" ) );
	_szSelectHelp.push_back( _strdup( "Class of object [MAX]" ) );

	_szSelect.push_back( _strdup( "!Selected" ) );
	_szSelectDisplayName.push_back( _strdup( "!Selected" ) );
	_szSelectHelp.push_back( _strdup( "Is Object Selected [MAX]" ) );

	int bSuccess = GetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK, "OAD_DIR", szOadDir, sizeof( szOadDir ) );
	if ( !bSuccess )
		Error( "OAD_DIR not set in registry" );

	{
    struct _finddata_t oad_file;
    long hFile;

	char szFileSpec[ _MAX_PATH ];
	strcpy( szFileSpec, szOadDir );
	strcat( szFileSpec, "/*.oad" );

    if ( ( hFile = _findfirst( szFileSpec, &oad_file ) ) != -1L )
	{
		Oad* pOad = new Oad( oad_file.name );
		assert( pOad );
		_oad.push_back( pOad );

		while ( _findnext( hFile, &oad_file ) == 0 )
		{
			Oad* pOad = new Oad( oad_file.name );
			assert( pOad );
			_oad.push_back( pOad );
		}

	_findclose( hFile );
	}
	}

	_miView = 0;
	_miArrangeBy = 0;
	_nSelectedColumn = -1;

	_hSmall = _hLarge = 0;

	HANDLE handle;
	DWORD threadID;

	handle = CreateThread( 0, 0, (LPTHREAD_START_ROUTINE)WaitForMaxWindow, 0, 0, &threadID );
	assert( handle );
}


Select::~Select()
{
	//assert( _hIcon );

	if ( _hSmall )
		ImageList_Destroy( _hSmall ), _hSmall = 0;

	if ( _hLarge )
		ImageList_Destroy( _hLarge ), _hLarge = 0;
}

////////////////////////////////////////////////////////////////////////////////

void
Select::SelectSelected()
{
	theHold.Begin();

	assert( hwndList );
	if ( ListView_GetSelectedCount( hwndList ) )
	{
		assert( _pRoot );

		bool bFirstTime = true;
		for ( int idxList=0; idxList<ListView_GetItemCount( hwndList ); ++idxList )
		{
			int state;
			state = ListView_GetItemState( hwndList, idxList, ~0 );
			if ( ( state & LVIS_SELECTED ) == LVIS_SELECTED )
			{
				LV_ITEM lvi;
				lvi.mask = LVIF_PARAM;
				lvi.iItem = idxList;
				lvi.iSubItem = 0;
				ListView_GetItem( hwndList, &lvi );

				int idxNode = lvi.lParam;	// = lvi.iItem;
				assert( idxNode >= 0 );

				INode* pObject = _pRoot->GetChildNode( idxNode );
				assert( pObject );
				ip->SelectNode( pObject, bFirstTime );
				bFirstTime = false;
			}
		}
	}
	else
	{
		ip->ExecuteMAXCommand( MAXCOM_EDIT_SELECTNONE );
	}

	TSTR undostr( "Select [by World Foundry Attributes]" );
	theHold.Accept( undostr );
}

////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK
ListViewHeaderWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	assert( theSelect._headerWndProc );
	if ( msg == WM_RBUTTONDOWN )
	{
		int i = Header_GetItemCount( hwnd );
		int x = LOWORD( lParam );
		int y = HIWORD( lParam );
		int checkX = 0;

		for ( int col = 0; ; ++col )
		{
			RECT rect;
			Header_GetItemRect( hwnd, col, &rect );

			checkX += rect.right - rect.left + 1;
			if ( checkX > x )
				break;
		}

		RECT rectParentParent;
		GetWindowRect( GetParent( GetParent( hwnd ) ), &rectParentParent );
		RECT rect;
		GetWindowRect( hwnd, &rect );

		x += rect.left - rectParentParent.left;
		lParam = MAKELONG( x, y );
		SendMessage( GetParent( GetParent( hwnd ) ), WM_COLUMN_RBUTTONDOWN, col, lParam );

		return 0;
	}
	else
		return CallWindowProc( theSelect._headerWndProc, hwnd, msg, wParam, lParam );
}


int CALLBACK
ListViewCompareProc( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
	Item* pItem1 = (Item*)theSelect._columns[ lParamSort ]->_items[ lParam1 ];
	Item* pItem2 = (Item*)theSelect._columns[ lParamSort ]->_items[ lParam2 ];

//	Item* pItem1 = (Item*)lParam1;
//	Item* pItem2 = (Item*)lParam2;

	assert( pItem1 && pItem2 );

//	DebugPrint( "%p 1 = %s\n", pItem1, pItem1->GetText() );
//	DebugPrint( "%p 2 = %s\n", pItem2, pItem2->GetText() );

	return theSelect._sortDirection * _stricmp( pItem1->GetText(), pItem2->GetText() );
}


void
Select::_InsertColumn( const char* szHeader, int col )
{
	assert( szHeader );
	assert( *szHeader );
	assert( 0 <= col );

	assert( hwndList );
	assert( hwndHeader );

	assert( _menuArrangeBy );

	//DeleteSelectByMenuItems();
//	for ( int i=col; i<Header_GetItemCount( hwndHeader ); ++i )
//	{
//		DeleteMenu( _menuArrangeBy, i, 0 );
//	}

	Column* pColumn = new Column( hwndList, szHeader, col );
	assert( pColumn );
	_columns.insert( &_columns[ col ], pColumn );

	// Items
	assert( _pRoot );
	int idxItem = 0;
	for ( int idxChild=0; idxChild<_pRoot->NumberOfChildren(); ++idxChild, ++idxItem )
	{
		INode* pObject = _pRoot->GetChildNode( idxChild );
		assert( pObject );

		//if ( !pObject->IsNodeHidden() )
		{
			char szText[ 100 ];
			sprintf( szText, "%s.%s", pObject->GetName(), szHeader );
			Item* pItem = new Item( hwndList, szText, pObject, idxItem, col, idxChild );
			assert( pItem );
			_columns[ col ]->_items.push_back( pItem );
		}
	}

	ListView_SetColumnWidth( hwndList, col, LVSCW_AUTOSIZE );

	_nSelectedColumn = _sortByColumn = col;
	ListView_SortItems( hwndList, ListViewCompareProc, _sortByColumn );

	//CreateSelectByMenuItems();
}


void
Select::_DeleteColumn( int col )
{
	assert( hwndHeader );
	assert( 0 <= col && col < Header_GetItemCount( hwndHeader ) );

	assert( _menuArrangeBy );

	for ( int i=col; i<Header_GetItemCount( hwndHeader ); ++i )
	{
		DeleteMenu( _menuArrangeBy, _columns[i]->_idMenu, 0 );
	}

	assert( hwndList );
	ListView_DeleteColumn( hwndList, col );

	_columns.erase( &_columns[ col ] );

	for ( i=col; i<Header_GetItemCount( hwndHeader ); ++i )
	{
		char szMenuItem[ 100 ];
		sprintf( szMenuItem, "by &%s", theSelect._columns[ i ]->GetText() );
		AppendMenu( _menuArrangeBy, MF_STRING, Select::IDM_ARRANGEBY + col, szMenuItem );
	}

	if ( _sortByColumn >= _columns.size() )
		_sortByColumn = _columns.size()-1;

	if ( _sortByColumn >= 0 )
		ListView_SortItems( hwndList, ListViewCompareProc, _sortByColumn );
	else
		InvalidateRect( hwndList, NULL, TRUE );
}


void
Select::PositionButtons( HWND hDlg )
{
	const top = 0;

	RECT rectDlg;
	GetWindowRect( hDlg, &rectDlg );

	assert( hwndList );
	MoveWindow( hwndList, 7, top,
		rectDlg.right - rectDlg.left - 110,
		rectDlg.bottom - rectDlg.top - 50,
		TRUE );
	ShowWindow( hwndList, SW_SHOWNORMAL );
	SetFocus( hwndList );

	assert( _hwndOk );
	RECT rectOk;
	GetWindowRect( _hwndOk, &rectOk );

	int widthOk = rectOk.right - rectOk.left;
	int heightOk = rectOk.bottom - rectOk.top;
	int xButton = rectDlg.right - rectDlg.left - 93;

	MoveWindow( _hwndOk, xButton, top, widthOk, heightOk, TRUE );

	// Cancel
	assert( _hwndCancel );
	MoveWindow( _hwndCancel, xButton, top + 27, widthOk, heightOk, TRUE );

	//UpdateStatistics( hDlg );
	HWND hwndText = GetDlgItem( hDlg, IDC_NUMSELECTED );
	assert( hwndText );
	MoveWindow( hwndText, xButton, top + 60, 300, 16, TRUE );

	HWND hwndTotal = GetDlgItem( hDlg, IDC_TOTALITEMS );
	assert( hwndTotal );
	MoveWindow( hwndTotal, xButton, top + 76, 300, 16, TRUE );
}


void
Select::FillInDialog( HWND hDlg )
{
	assert( hDlg );

	_hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_WF ) );
	assert( _hIcon );
	_hIconSphere = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_SPHERE ) );
	assert( _hIconSphere );

	SetClassLong( hDlg, GCL_HICON, (LONG)_hIcon );

	_hwndOk = GetDlgItem( hDlg, IDOK );
	assert( _hwndOk );

	_hwndCancel = GetDlgItem( hDlg, IDCANCEL );
	assert( _hwndCancel );

	_hSmall = ImageList_Create( 16, 16, FALSE, 1, 0 );
	assert( _hSmall );

	_hLarge = ImageList_Create( 32, 32, FALSE, 1, 0 );
	assert( _hLarge );

	ImageList_AddIcon( _hSmall, _hIconSphere );
	ImageList_AddIcon( _hLarge, _hIconSphere );

	RECT rectDlg;
	GetWindowRect( hDlg, &rectDlg );

	hwndList = CreateWindow(
		WC_LISTVIEW,
		"",
		LVS_NOLABELWRAP | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE |	// LVS_OWNERDRAWFIXED |
			WS_BORDER | WS_TABSTOP | WS_CHILD,
		0,0,1,1,
		hDlg,
		NULL,
		hInstance,
		NULL );
	assert( hwndList );

	PositionButtons( hDlg );

	hwndHeader = ListView_GetHeader( hwndList );
	assert( hwndHeader );

	DWORD style = ListView_GetExtendedListViewStyle( hwndList );
	style |= LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP;
	ListView_SetExtendedListViewStyle( hwndList, style );

	ListView_SetImageList( hwndList, _hSmall, LVSIL_SMALL );
	ListView_SetImageList( hwndList, _hLarge, LVSIL_NORMAL );

	// intercept WndProc for header to check for right-click
	_headerWndProc = SubclassWindow( hwndHeader, ListViewHeaderWndProc );
	assert( _headerWndProc );

	HMENU hMenu = GetMenu( hDlg );
	assert( hMenu );

	_menuEdit = GetSubMenu( hMenu, 0 );
	assert( _menuEdit );

	HMENU hViewMenu = GetSubMenu( hMenu, 1 );			// View
	assert( hViewMenu );
	_menuArrangeBy = GetSubMenu( hViewMenu, 5 );	// Arrange by ->
	assert( _menuArrangeBy );

	DeleteMenu( _menuArrangeBy, 0, 0 );				// Delete placeholder

	assert( _columns.size() == 0 );
	if ( !LoadConfiguration() )
	{
		_InsertColumn( "!Name", 0 );
		_InsertColumn( "!Class", 1 );
		//_InsertColumn( "!Selected", 2 );
	}
}


void
Select::SaveConfiguration()
{
	char szColumns[ _MAX_CONFIGURATION_LINE ];
	*szColumns = '\0';

	std::vector< Column* >::iterator pColumn;
	for ( pColumn=_columns.begin(); pColumn != _columns.end(); ++pColumn )
	{
		strcat( szColumns, (*pColumn)->GetText() );
		strcat( szColumns, "," );
	}

	bool bSuccess;
	bSuccess = SetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK "\\Select", "Columns", szColumns );
	assert( bSuccess );
}


bool
Select::LoadConfiguration()
{
	char szColumns[ _MAX_CONFIGURATION_LINE ];

	bool bSuccess;
	bSuccess = GetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK "\\Select", "Columns", szColumns, sizeof( szColumns ) );
	if ( bSuccess )
	{
		int col = 0;
		char* pszColumns = szColumns;

		for ( ;; )
		{
			char* pComma = strchr( pszColumns, ',' );

			if ( !pComma )
				break;

			*pComma = '\0';
			_InsertColumn( pszColumns, col );

			pszColumns = pComma+1;
			++col;
		}
	}

	return bSuccess;
}



BOOL CALLBACK
SelectDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	theSelect._hDlg = hDlg;
	theSelect._hMenu = GetMenu( theSelect._hDlg );
	assert( theSelect._hMenu );
	assert( theSelect.ip );
	theSelect._pRoot = theSelect.ip->GetRootNode();
	assert( theSelect._pRoot );

	switch ( msg )
	{
		case WM_INITMENUPOPUP:
		{
			assert( theSelect._menuEdit );
			EnableMenuItem( theSelect._menuEdit, ID_DELETECOLUMN,
				theSelect._columns.size() == 0 ? MF_GRAYED : MF_ENABLED );
			break;
		}

		case WM_INITDIALOG:
		{
			theSelect.FillInDialog( hDlg );
			theSelect._SetView( ID_VIEW_DETAILS, LVS_REPORT );
			UpdateStatistics( hDlg );
			HWND hwndTotal = GetDlgItem( hDlg, IDC_TOTALITEMS );
			assert( hwndTotal );
			char buffer[ 100 ];
			sprintf( buffer, "%d total items", ListView_GetItemCount( theSelect.hwndList ) );
			Static_SetText( hwndTotal, buffer );
			return TRUE;
		}

		case WM_COLUMN_RBUTTONDOWN:
		{
			int col = theSelect._nSelectedColumn = wParam;

			POINT point;
			point.x = LOWORD( lParam );
			point.y = HIWORD( lParam );
			ClientToScreen( hDlg, &point );

			int nCols = theSelect._columns.size();
			int bChecked = nCols && ( col != nCols ) ? MF_ENABLED : MF_GRAYED;

			assert( theSelect._columnRCmenu );
			EnableMenuItem( theSelect._columnRCmenu, ID_ASCENDING, bChecked );
			EnableMenuItem( theSelect._columnRCmenu, ID_DESCENDING, bChecked );
			EnableMenuItem( theSelect._columnRCmenu, ID_DELETECOLUMN, bChecked );

			TrackPopupMenu( theSelect._columnRCmenu, 0, point.x, point.y, 0, hDlg, NULL );

			return 0;
		}

		case WM_SIZE:
		{
			theSelect.PositionButtons( hDlg );
			break;
		}

#if 0
		case WM_DRAWITEM:
		{
			char achTemp[256];       /* temporary buffer            */

			LPDRAWITEMSTRUCT lpdis;
			COLORREF clrBackground;
			COLORREF clrForeground;
			TEXTMETRIC tm;
			int x, y;


            lpdis = (LPDRAWITEMSTRUCT) lParam;
            if (lpdis->itemID == -1)            /* empty item */
                break;

#if 0
            /* Determine the bitmaps used to draw the icon. */

            switch (lpdis->itemData) {
                case ID_BREAD:
                    hbmIcon = hbmBread;
                    hbmMask = hbmBreadMask;
                    break;

                case ID_DAIRY:
                    hbmIcon = hbmDairy;
                    hbmMask = hbmDairyMask;
                    break;

                case ID_FRUIT:
                    hbmIcon = hbmFruit;
                    hbmMask = hbmFruitMask;
                    break;

                default:                /* meat */
                    hbmIcon = hbmMeat;
                    hbmMask = hbmMeatMask;
                    break;
            }
#endif

            // The colors depend on whether the item is selected.

            clrForeground = SetTextColor(lpdis->hDC, GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
            clrBackground = SetBkColor(lpdis->hDC, GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));

            // Calculate the vertical and horizontal position.
            GetTextMetrics(lpdis->hDC, &tm);
            y = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;
            x = LOWORD(GetDialogBaseUnits()) / 4;

            // Get and display the text for the list item.
            //SendMessage(lpdis->hwndItem, CB_GETLBTEXT, lpdis->itemID, (LPARAM) (LPCSTR) achTemp);
			//strcpy( achTemp, "Text" );
            //ExtTextOut( lpdis->hDC, /*CX_BITMAP + 2 * */ x, y, ETO_CLIPPED | ETO_OPAQUE, &lpdis->rcItem, achTemp, lstrlen(achTemp), NULL );

			for ( int i=0; i<theSelect._columns.size(); ++i )
			{
//				strcpy( achTemp, theSelect._columns[i]->_items[0]->GetText() );
				strcpy( achTemp, "Test" );
				ExtTextOut( lpdis->hDC, /*CX_BITMAP + 2 * */ x, y, ETO_CLIPPED | ETO_OPAQUE, &lpdis->rcItem, achTemp, lstrlen(achTemp), NULL );
				x += 35;
			}

            // Restore the previous colors.
            SetTextColor(lpdis->hDC, clrForeground);
            SetBkColor(lpdis->hDC, clrBackground);


#if 0
            // Show the icon.

            hdc = CreateCompatibleDC(lpdis->hDC);
            if (hdc == NULL)
                break;

            SelectObject(hdc, hbmMask);
            BitBlt(lpdis->hDC, x, lpdis->rcItem.top + 1,
                CX_BITMAP, CY_BITMAP, hdc, 0, 0, SRCAND);

            SelectObject(hdc, hbmIcon);
            BitBlt(lpdis->hDC, x, lpdis->rcItem.top + 1,
                CX_BITMAP, CY_BITMAP, hdc, 0, 0, SRCPAINT);

            DeleteDC(hdc);
#endif

            // If the item has the focus, draw focus rectangle.
            if (lpdis->itemState & ODS_FOCUS)
                DrawFocusRect(lpdis->hDC, &lpdis->rcItem);

			break;
		}
#endif

		case WM_NOTIFY:
		{
			UpdateStatistics( hDlg );
//			assert( wParam == IDC_LIST1 );

			LV_DISPINFO* pLvdi = (LV_DISPINFO*)lParam;
			assert( pLvdi );
			switch ( pLvdi->hdr.code )
			{
				case LVN_GETDISPINFO:
				{
					LV_DISPINFO* pLvdi = (LV_DISPINFO*)lParam;
					assert( pLvdi );

					int nColumn = pLvdi->item.iSubItem;
					//DebugPrint( "iSubItem = %d\t", nColumn );
					//assert( nColumn < theSelect._columns.size() );
					Column* pColumn = theSelect._columns[ nColumn ];
					assert( pColumn );

					int nRow = pLvdi->item.lParam;
					//DebugPrint( "iItem = %d\n", nRow );
					assert( nRow < pColumn->_items.size() );
					Item* item = pColumn->_items[ nRow ];
					assert( item );

					pLvdi->item.pszText = (char*)( item->GetText() );
					break;
				}

				case LVN_COLUMNCLICK:
				{
					NM_LISTVIEW* pNm = (NM_LISTVIEW*)lParam;
					assert( pNm );

					if ( pNm->iSubItem == theSelect._sortByColumn )
						theSelect._sortDirection = -theSelect._sortDirection;
					else
					{
						theSelect._sortByColumn = pNm->iSubItem;
						theSelect._sortDirection = 1;
					}
					ListView_SortItems( pNm->hdr.hwndFrom, ListViewCompareProc, (LPARAM)( pNm->iSubItem ) );
					break;
				}

				case LVN_BEGINLABELEDIT:
				{
					DisableAccelerators();
					return 0;
				}

				case LVN_ENDLABELEDIT:
				{
					LV_DISPINFO* pLvdi = (LV_DISPINFO*)lParam;
					assert( pLvdi );

					if ( ( pLvdi->item.iItem != -1 )
					&& ( pLvdi->item.pszText != NULL )
					&& ( *pLvdi->item.pszText )
					)
						;	//DebugPrint( "rename to %s\n", pLvdi->item.pszText );

					EnableAccelerators();
					break;
				}
			}
			break;
		}

		case WM_COMMAND:
		{
			int nCommand = LOWORD( wParam );

			if ( Select::IDM_ARRANGEBY <= nCommand && nCommand < Select::IDM_ARRANGEBY + theSelect._columns.size() )
			{
				theSelect._SetArrangeBy( nCommand );
			}
			else
			{
				switch ( nCommand )
				{
					case IDOK:
					{
						theSelect.SelectSelected();
						theSelect.SaveConfiguration();
						while ( theSelect._columns.size() )
							theSelect._columns.erase( theSelect._columns.end()-1 );
						assert( theSelect._columns.size() == 0 );
						EndDialog( hDlg, true );
						return TRUE;
					}

					case IDCANCEL:
						while ( theSelect._columns.size() )
							theSelect._columns.erase( theSelect._columns.end()-1 );
						assert( theSelect._columns.size() == 0 );
						EndDialog( hDlg, false );
						return TRUE;

					case ID_EDIT_SELECTNONE:		theSelect.OnEditSelectNone(); break;
					case ID_EDIT_SELECTALL:			theSelect.OnEditSelectAll(); break;
					case ID_EDIT_INVERTSELECTION:	theSelect.OnEditInvertSelection(); break;
					case ID_EDIT_SELECTBYREGULAREXPRESSION: theSelect.OnEditSelectByRegularExpression(); break;
					case ID_VIEW_LARGEICONS:		theSelect.OnViewLargeIcons(); break;
					case ID_VIEW_SMALLICONS:		theSelect.OnViewSmallIcons(); break;
					case ID_VIEW_LIST:				theSelect.OnViewList(); break;
					case ID_VIEW_DETAILS:			theSelect.OnViewDetails(); break;
					case ID_HELP_ABOUT:				theSelect.OnHelpAbout(); break;

					case ID_ASCENDING:				theSelect.OnAscending(); break;
					case ID_DESCENDING:				theSelect.OnDescending(); break;
					case ID_INSERTCOLUMN:			theSelect.OnInsertColumn(); break;
					case ID_DELETECOLUMN:			theSelect.OnDeleteColumn(); break;
				}
				break;
			}

			UpdateStatistics( hDlg );
		}
	}

	return FALSE;
}


void
Select::DoDialog()
{
	assert( hInstance );
	assert( ip );

	_sortDirection = -1;
	_sortByColumn = 0;

	if ( DialogBox( hInstance, MAKEINTRESOURCE( IDD_SELECT ), ip->GetMAXHWnd(), SelectDlgProc ) )
	{
		ip->RedrawViews( ip->GetTime(), REDRAW_BEGIN );
		ip->RedrawViews( ip->GetTime(), REDRAW_INTERACTIVE );
		ip->RedrawViews( ip->GetTime(), REDRAW_END );
	}
}

////////////////////////////////////////////////////////////////////////////////

void
Select::OnEditSelectNone()
{
	assert( hwndList );
	for ( int i=0; i<ListView_GetItemCount( hwndList ); ++i )
	{
		int state;
		state = ListView_GetItemState( hwndList, i, ~0 );
		state &= ~LVIS_SELECTED;
		ListView_SetItemState( hwndList, i, state, ~0 );
	}
}


void
Select::OnEditSelectAll()
{
	assert( hwndList );
	for ( int i=0; i<ListView_GetItemCount( hwndList ); ++i )
	{
		int state;
		state = ListView_GetItemState( hwndList, i, ~0 );
		state |= LVIS_SELECTED;
		ListView_SetItemState( hwndList, i, state, ~0 );
	}
}


void
Select::OnEditInvertSelection()
{
	assert( hwndList );
	for ( int i=0; i<ListView_GetItemCount( hwndList ); ++i )
	{
		int state;
		state = ListView_GetItemState( hwndList, i, ~0 );
		state ^= LVIS_SELECTED;
		ListView_SetItemState( hwndList, i, state, ~0 );
	}
}


BOOL CALLBACK
ExpressionDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_COMMAND:
		{
			int nCommand = LOWORD( wParam );
			switch ( nCommand )
			{
				case IDOK:
				case IDCANCEL:
				{
					EndDialog( hDlg, nCommand == IDOK );
					return nCommand == IDCANCEL;
				}
			}
		break;
		}
	}

	return FALSE;
}


void
Select::OnEditSelectByRegularExpression()
{
	assert( hwndList );

	// put up dialog box
	if ( !DialogBox( hInstance, MAKEINTRESOURCE( IDD_EXPRESSION ), ip->GetMAXHWnd(), ExpressionDlgProc ) )
		return;

	for ( int i=0; i<ListView_GetItemCount( hwndList ); ++i )
	{
		int state;
		state = ListView_GetItemState( hwndList, i, ~0 );
		if ( 1 )
			state |= LVIS_SELECTED;
		else
			state &= ~LVIS_SELECTED;
		ListView_SetItemState( hwndList, i, state, ~0 );
	}
}


void
Select::_SetArrangeBy( DWORD mi )
{
	int nColumn = mi - Select::IDM_ARRANGEBY;
	assert( 0 <= nColumn && nColumn < theSelect._columns.size() );

	if ( nColumn == _sortByColumn )
		_sortDirection = -_sortDirection;
	else
	{
		_sortByColumn = nColumn;
		_sortDirection = 1;
	}
	assert( _hDlg );
	assert( hwndList );
	ListView_SortItems( hwndList, ListViewCompareProc, nColumn );

	if ( mi != _miArrangeBy )
	{
		if ( _miArrangeBy )
		{
			assert( _hMenu );
			CheckMenuItem( _hMenu, _miArrangeBy, MF_UNCHECKED );
		}
		_miArrangeBy = mi;
		CheckMenuItem( _hMenu, _miArrangeBy, MF_CHECKED );
	}
}


void
Select::_SetView( DWORD mi, DWORD lvs )
{
	assert( hwndList );

	if ( mi != _miView )
	{
		assert( _hMenu );
		if ( _miView )
		{
			CheckMenuItem( _hMenu, _miView, MF_UNCHECKED );
		}
		_miView = mi;
		CheckMenuItem( _hMenu, _miView, MF_CHECKED );

		DWORD dwStyle;
		dwStyle = GetWindowLong( hwndList, GWL_STYLE );
		if ( ( dwStyle & LVS_TYPEMASK ) != lvs )
			SetWindowLong( hwndList, GWL_STYLE, ( dwStyle & ~LVS_TYPEMASK ) | lvs );
	}
}


void
Select::OnViewArrangeIcons()
{
	assert( hwndList );
	ListView_Arrange( hwndList, LVA_SNAPTOGRID );
}


void
Select::OnHelpAbout()
{
	void AboutBox( HWND );
	AboutBox( _hDlg );
}


void
Select::OnAscending()
{
	_sortDirection = 1;
	assert( hwndList );
	ListView_SortItems( hwndList, ListViewCompareProc, _sortByColumn );
}


void
Select::OnDescending()
{
	_sortDirection = -1;
	assert( hwndList );
	ListView_SortItems( hwndList, ListViewCompareProc, _sortByColumn );
}


BOOL CALLBACK InsertColumnDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );

void
Select::OnInsertColumn()
{
	assert( _szInsertColumnExpressions.size() == 0 );
	if ( DialogBox( hInstance, MAKEINTRESOURCE( IDD_INSERTCOLUMN ), _hDlg, InsertColumnDlgProc ) )
	{
		std::vector<char*>::iterator str = _szInsertColumnExpressions.begin();
	  	while ( str != _szInsertColumnExpressions.end() )
		{
			assert( *str );
			int colAdd = _nSelectedColumn + 1;
			assert( hwndHeader );
			if ( colAdd > Header_GetItemCount( hwndHeader ) )
				colAdd = Header_GetItemCount( hwndHeader );
			_InsertColumn( *str, colAdd );
//			free( str );
			_szInsertColumnExpressions.erase( _szInsertColumnExpressions.begin() );
		}
		assert( _szInsertColumnExpressions.size() == 0 );

		SetFocus( hwndList );
	}
}


void
Select::OnDeleteColumn()
{
	if ( _nSelectedColumn >= _columns.size() )
		_nSelectedColumn = _columns.size() - 1;

	_DeleteColumn( _nSelectedColumn );
}

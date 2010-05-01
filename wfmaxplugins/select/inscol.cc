#include <stdarg.h>
#include <windowsx.h>
#include <xstddef>
#include <vector>
#include "select.h"
#include <oas/oad.h>

BOOL CALLBACK
InsertColumnDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_INITDIALOG:
		{
			HWND hwndList = GetDlgItem( hDlg, IDC_COLUMN_LIST );
			assert( hwndList );

			DWORD style = ListView_GetExtendedListViewStyle( hwndList );
			style |= LVS_EX_FULLROWSELECT;
			ListView_SetExtendedListViewStyle( hwndList, style );

			LV_COLUMN lvc;
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;

			lvc.cx = 140;
			lvc.iSubItem = 0;
			lvc.pszText = "Field";
			ListView_InsertColumn( hwndList, lvc.iSubItem, &lvc );

			lvc.cx = 335;
			lvc.iSubItem = 1;
			lvc.pszText = "Description";
			ListView_InsertColumn( hwndList, lvc.iSubItem, &lvc );

			int idxItem = 0;
			std::vector< string >::iterator str;
			std::vector< string >::iterator strHelp = theSelect._szSelectHelp.begin();
			for ( str=theSelect._szSelectDisplayName.begin();
				str != theSelect._szSelectDisplayName.end();
				++str, ++strHelp )
			{
				assert( strHelp != theSelect._szSelectHelp.end() );
				assert( *str );
				assert( **str );

				LV_ITEM lvi;

				lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
				lvi.state = lvi.stateMask = 0;
				lvi.iItem = idxItem;
				lvi.iSubItem = 0;
				lvi.pszText = *str;
				lvi.cchTextMax = strlen( lvi.pszText );
				lvi.lParam = str - theSelect._szSelectDisplayName.begin();
				assert( lvi.lParam >= 0 );
				assert( lvi.lParam < theSelect._szSelectDisplayName.size() );
				ListView_InsertItem( hwndList, &lvi );

				ListView_SetItemText( hwndList, idxItem, 1, LPSTR_TEXTCALLBACK );

				++idxItem;
			}
			return TRUE;
		}

		case WM_LBUTTONDBLCLK:
			assert( 0 );
			break;

		case WM_COMMAND:
		{
			switch ( LOWORD( wParam ) )
			{
				case IDOK:
				{
					HWND hwndList = GetDlgItem( hDlg, IDC_COLUMN_LIST );
					assert( hwndList );

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

							int idxSelect = lvi.lParam;
							assert( idxSelect >= 0 );

							char* str = theSelect._szSelect[ idxSelect ];
							assert( str );
							assert( *str );
							theSelect._szInsertColumnExpressions.push_back( str );
						}
					}

					EndDialog( hDlg, true );
					return TRUE;
				}

				case IDCANCEL:
				{
					EndDialog( hDlg, false );
					return FALSE;
				}

				case IDC_COLUMN_LIST:
				{
					if ( HIWORD( wParam ) == LBN_DBLCLK )
						SendMessage( hDlg, WM_COMMAND, IDOK, (LPARAM)GetDlgItem( hDlg, IDOK ) );
					break;
				}
			}
			break;
		}

		case WM_NOTIFY:
		{
			LV_DISPINFO* pLvdi = (LV_DISPINFO*)lParam;
			assert( pLvdi );

			if ( pLvdi->hdr.code == LVN_GETDISPINFO )
				pLvdi->item.pszText = theSelect._szSelectHelp[ pLvdi->item.lParam ];

			break;
		}

	}

	return FALSE;
}

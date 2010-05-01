#include <pigsys/assert.hp>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <windows.h>
#include <attrib/attrib.hp>
#include <attrib/oad.hp>
#include "resource.h"
#include <commdlg.h>

Attributes* theAttributes;
extern Oad* theOad;

#pragma message( "Turn preferences into attrib module when more mature" )
bool bExpandedDisplay;
extern char szWorldFoundryDir[];
extern char szOadDir[];

// A quick little routine that will center a window on the screen.
// Handy for dialog boxes
BOOL CenterWindow (HWND hwnd)
{
    RECT    rect;
    int     w, h;
    int     wScreen, hScreen, xNew, yNew;
    HDC     hdc;

    GetWindowRect (hwnd, &rect);
    w = rect.right - rect.left;
    h = rect.bottom - rect.top;

    hdc = GetDC (hwnd);
    wScreen = GetDeviceCaps (hdc, HORZRES);
    hScreen = GetDeviceCaps (hdc, VERTRES);
    ReleaseDC (hwnd, hdc);

    xNew = wScreen/2 - w/2;
    yNew = hScreen/2 - h/2;

    return SetWindowPos( hwnd, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}


#pragma message( "make dlgproc static proc of attributes" )
extern BOOL CALLBACK AttributesDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

BOOL APIENTRY
AboutDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if ( msg == WM_INITDIALOG )
	{
		CenterWindow( hDlg );

		HWND hwndVersion = GetDlgItem( hDlg, IDC_VERSION );
		assert( hwndVersion );
		extern char szVersion[];
		char szText[ 512 ];
		sprintf( szText, "UDM\nVersion %s", szVersion );
		Static_SetText( hwndVersion, szText );
		return TRUE;
	}
	else if ( ( msg == WM_COMMAND ) && ( LOWORD( wParam ) == IDOK ) )
	{
		EndDialog( hDlg, TRUE );
		return TRUE;
	}

	return FALSE;
}


BOOL APIENTRY
CLDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    if ( msg == WM_INITDIALOG )
	{
        // We need to initialize stuff in the dialog box...
        CenterWindow( hDlg );

		if ( theOad )
		{
			theAttributes = new Attributes( hDlg, theOad );
			assert( theAttributes );
		}

		AttributesDlgProc( hDlg, msg, wParam, lParam );

        return TRUE;
	}
	else
	{
		if ( msg == WM_COMMAND )
		{
            int wmId = LOWORD(wParam);
            switch ( wmId )
			{
				case ID_FILE_EXIT:
                case IDOK:
				case IDCANCEL:
                    EndDialog( hDlg, wmId != IDCANCEL );
                    return TRUE;

				case ID_PREFERENCES_EXPANDEDDISPLAY:
				{
					MENUITEMINFO mii;
					mii.cbSize = sizeof( mii );
					mii.fMask = MIIM_STATE;

					HMENU hMenu = GetMenu( hDlg );
					assert( hMenu );
					int error = GetMenuItemInfo( hMenu, wmId, FALSE, &mii );
					//assert( error == 0 );
					mii.fState ^= MFS_CHECKED;
					error = SetMenuItemInfo( hMenu, wmId, FALSE, &mii );
					//assert( error == 0 );

					if ( theAttributes )
					{
#pragma message( "Temp hack [?] to refresh" )
						theAttributes->DestroyGadgets();
						bExpandedDisplay = bool( mii.fState & MFS_CHECKED );
						theAttributes->CreateGadgets();
					}
					break;
				}

				case ID_FILTER_OPEN:
				{
					char szOadFilename[ _MAX_PATH ] = { 0 };
					static char szInitialDir[ _MAX_PATH ] = { 0 };

					char szTitle[] = "Open Filter...";

					OPENFILENAME ofn = {
						sizeof( OPENFILENAME ),
						hDlg,			// hwndOwner
						::hInstance,			// hInstance
						"Object Attribute Description (*.oad)\0*.oad\0\0",	// lpstrFilter
						NULL,				// lpstrCustomFilter,
						0,					// nMaxCustFilter
						0,					// nFilterIndex
						szOadFilename,
						sizeof( szOadFilename ),
						NULL,				// lpstrFileTitle
						0,					// nMaxFileTitle
						szInitialDir,		// lpstrInitialDir
						szTitle,			// lpstrTitle
						OFN_HIDEREADONLY | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,	// Flags
						0,					// nFileOffset
						0,					// nFileExtension
						NULL,				// lpStrDefExt,
						NULL,				// lCustData
						NULL,				// lpfnHook
						NULL				// lpTemplateName
					};

					if ( !*szInitialDir )
						strcpy( szInitialDir, szOadDir );

					if ( GetOpenFileName( &ofn ) )
					{
//						char szCurFilePath[ _MAX_PATH ];
//						strcpy( szCurFilePath, theAttributes.ip->GetCurFilePath() );

						delete theAttributes;			// also deletes theOad

//						strcpy( szOadFilename, "d:/wfsrc/levels.src/oad/room.oad" );
						printf( "Opening \"%s\"", szOadFilename );
						theOad = new Oad( szOadFilename );
						assert( theOad );
						theAttributes = new Attributes( hDlg, theOad );
						assert( theAttributes );
						theAttributes->CreateGadgets();
					}

					break;
				}

				case ID_HELP_ABOUT:
					DialogBox( ::hInstance, MAKEINTRESOURCE( IDD_ABOUT ), hDlg, (DLGPROC)AboutDlgProc );
					break;
            }

			return AttributesDlgProc( hDlg, msg, wParam, lParam );
		}
		else
			return AttributesDlgProc( hDlg, msg, wParam, lParam );
    }
    return FALSE;
}

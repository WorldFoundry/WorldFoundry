// play.cc

#include <pigsys/pigsys.hp>
#include <windows.h>
HINSTANCE hInstance;
#include <attrib/oad.hp>
#include <commctrl.h>
#include "../registry.h"
#include "resource.h"

//=============================================================================

BOOL APIENTRY CLDlgProc( HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam );

Oad* theOad;
char szWorldFoundryDir[ _MAX_PATH ];
char szOadDir[ _MAX_PATH ];

int
main( int argc, char* argv[] )
{
	if ( argc == 2 )
	{
		const char* szOadFilename = argv[ 1 ];
		cout << "loading oad" << endl;
		theOad = new Oad( szOadFilename );
		assert( theOad );
	}
	else
		cout << "usage: " << argv[0] << " <.lc file>  " << endl;

    hInstance = GetModuleHandle( NULL );
	assert( hInstance );

	InitCommonControls();

	{
	const char szRegWorldFoundry[] = "Software\\World Foundry\\GDK";
	GetLocalMachineStringRegistryEntry( szRegWorldFoundry, "WORLD_FOUNDRY_DIR", szWorldFoundryDir, sizeof( szWorldFoundryDir ) );
	GetLocalMachineStringRegistryEntry( szRegWorldFoundry, "OAD_DIR", szOadDir, sizeof( szOadDir ) );
	}

    //HWND hwnd = GetFocus();
	//assert( hwnd );

	cout << "opening dialog box" << endl;
    int ret = DialogBoxParam( hInstance, MAKEINTRESOURCE( IDD_UDM ), NULL, (DLGPROC)CLDlgProc, (LPARAM)0 );

	AssertMsg( ret != -1, "Unable to create dialog: #" << GetLastError() );

	return 0;
}

//=============================================================================

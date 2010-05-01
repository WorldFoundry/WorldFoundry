//==============================================================================
// ui.cc:
//==============================================================================

#include "global.hpp"
#include "max2lev.hpp"			// C++ header file
#include "max2lev.h"				// Resource header file
#include "../lib/registry.h"		// registry reader
#include <oas/oad.h>
//#include <iffwrite/iffwrite.h>
//#include <iffwrite/fixed.hp>
#include "box.h"
//#include <stl/vector.h>
#include "../lib/wf_id.hp"
#include "../lib/loaddll.h"
extern HINSTANCE hInstance;

static HWND hwndAttributes, hwndPaths, hwndModels;
static HWND hwndAscii, hwndBinary;
static HWND hwndGeometry, hwndAnimation, hwndMaterials, hwndHandles, hwndEvents;
//static HWND hwndGPosition, hwndGRotation, hwndGScale;
static HWND hwndScale;
struct HwndFixedPoint
{
	HWND hwndSign;
	HWND hwndWhole;
	HWND hwndFraction;
};
static HwndFixedPoint hwndReal, hwndQuaternion;
//static HWND hwndDontPrompt;


extern max2ifflvlOptions theOptions;

#define GET_BUTTON( __hwnd__, __id__ ) \
	do { __hwnd__ = GetDlgItem( hDlg, __id__ ); assert( __hwnd__ ); } while ( 0 )


#define szRegMax2IFFLVL szRegWorldFoundryGDK "\\max2ifflvl"

void
SET_BUTTON( HWND hwnd, const char* szRegistryEntry, bool bDefault )
{
	assert( hwnd );

	bool bValue = bDefault;

	char szValue[ 64 ];
	if ( GetLocalMachineStringRegistryEntry( szRegMax2IFFLVL, szRegistryEntry, szValue, sizeof( szValue ) ) )
		bValue = atoi( szValue );

	Button_SetCheck( hwnd, bValue );
}


void
SAVE_BUTTON( HWND hwnd, const char* szRegistryEntry )
{
	assert( hwnd );
	SetLocalMachineStringRegistryEntry( szRegMax2IFFLVL, szRegistryEntry, Button_GetState( hwnd ) ? "1" : "0" );
}


void
Edit_SetNumber( HWND hwnd, /*string szRegistryEntry,*/ int val )
{
	char szBuffer[ 256 ];
	sprintf( szBuffer, "%d", val );

//	GetLocalMachineStringRegistryEntry( szRegMax2IFFLVL, szRegistryEntry.c_str(), szBuffer, sizeof( szBuffer ) );

	Edit_SetText( hwnd, szBuffer );
}


void
SetHWNDFixedPointPrecision( HwndFixedPoint& hwnd, string szRegistryEntry, size_specifier& ss )
{
	char szBuffer[ 128 ];
	if ( GetLocalMachineStringRegistryEntry( szRegMax2IFFLVL, szRegistryEntry.c_str(), szBuffer, sizeof( szBuffer ) ) )
		sscanf( szBuffer, "%d.%d.%d", &ss.sign, &ss.whole, &ss.fraction );

	Edit_SetNumber( hwnd.hwndSign, ss.sign );
	Edit_SetNumber( hwnd.hwndWhole, ss.whole );
	Edit_SetNumber( hwnd.hwndFraction, ss.fraction );
}


void
GetHWNDFixedPointPrecision( HwndFixedPoint& hwnd, string szRegistryEntry, size_specifier& ss )
{
	char szBuffer[ 128 ];
	Edit_GetText( hwnd.hwndSign, szBuffer, sizeof( szBuffer ) );
	ss.sign = atoi( szBuffer );
	Edit_GetText( hwnd.hwndWhole, szBuffer, sizeof( szBuffer ) );
	ss.whole = atoi( szBuffer );
	Edit_GetText( hwnd.hwndFraction, szBuffer, sizeof( szBuffer ) );
	ss.fraction = atoi( szBuffer );

	sprintf( szBuffer, "%d.%d.%d", ss.sign, ss.whole, ss.fraction );
	SetLocalMachineStringRegistryEntry( szRegMax2IFFLVL, szRegistryEntry.c_str(), szBuffer );
}


BOOL CALLBACK
OutputOptionsDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if ( msg == WM_INITDIALOG )
	{
		char str[ 512 ];
//		bool bDontPrompt = false;
		bool bOutputBinary = true;
		char szScale[ 100 ] = "1.0";

		HICON _hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_WF ) );
		assert( _hIcon );
		SetClassLong( hDlg, GCL_HICON, (LONG)_hIcon );

//		HWND hwndObjectName = GetDlgItem( hDlg, IDC_OBJECT_NAME );
//		assert( hwndObjectName );
//		assert( pOutputNode );
//		Button_SetText( hwndObjectName, pOutputNode->GetName() );

//		HWND hwndOutputFilename = GetDlgItem( hDlg, IDC_OUTPUT_FILENAME );
//		assert( hwndOutputFilename );
//		Button_SetText( hwndOutputFilename, szOutputObjectName );

		if ( GetLocalMachineStringRegistryEntry( szRegMax2IFFLVL, "Output Mode", str, sizeof( str ) ) )
			bOutputBinary = tolower( *str ) == 'b';

		GetLocalMachineStringRegistryEntry( szRegMax2IFFLVL, "Scale", szScale, sizeof( szScale ) );
		GET_BUTTON( hwndScale, IDC_SCALE );
		Edit_SetText( hwndScale, szScale );

		GET_BUTTON( hwndAscii, IDC_ASCII );
		GET_BUTTON( hwndBinary, IDC_BINARY );
		Button_SetCheck( bOutputBinary ? hwndBinary : hwndAscii, true );

		GET_BUTTON( hwndGeometry, IDC_GEOMETRY );
		GET_BUTTON( hwndAnimation, IDC_ANIMATION );
		GET_BUTTON( hwndMaterials, IDC_MATERIALS );
		GET_BUTTON( hwndHandles, IDC_HANDLES );
		GET_BUTTON( hwndEvents, IDC_EVENTS );
//		GET_BUTTON( hwndGPosition, IDC_GPOSITION );
//		GET_BUTTON( hwndGRotation, IDC_GROTATION );
//		GET_BUTTON( hwndGScale, IDC_GSCALE );
		GET_BUTTON( hwndAttributes, IDC_ATTRIBUTES );
		GET_BUTTON( hwndPaths, IDC_PATH );
		GET_BUTTON( hwndModels, IDC_MODELS );

		GET_BUTTON( hwndReal.hwndSign, IDC_REAL_SIGN );
		GET_BUTTON( hwndReal.hwndWhole, IDC_REAL_WHOLE );
		GET_BUTTON( hwndReal.hwndFraction, IDC_REAL_FRACTION );
		GET_BUTTON( hwndQuaternion.hwndSign, IDC_QUAT_SIGN );
		GET_BUTTON( hwndQuaternion.hwndWhole, IDC_QUAT_WHOLE );
		GET_BUTTON( hwndQuaternion.hwndFraction, IDC_QUAT_FRACTION );

		SET_BUTTON( hwndGeometry, "Output Geometry", true );
		SET_BUTTON( hwndAnimation, "Output Animation", false );
		SET_BUTTON( hwndMaterials, "Output Materials", true );
		SET_BUTTON( hwndHandles, "Output Handles", false );
		SET_BUTTON( hwndEvents, "Output Events", false );
#if 0
		SET_BUTTON( hwndGPosition, "Output Global Position", false );
		SET_BUTTON( hwndGRotation, "Output Global Rotation", false );
		SET_BUTTON( hwndGScale, "Output Global Scale", false );
#endif
		SET_BUTTON( hwndAttributes, "Output Attributes", true );
		SET_BUTTON( hwndPaths, "Output Paths", true );
		SET_BUTTON( hwndModels, "Output Models", true );

		SetHWNDFixedPointPrecision( hwndReal, "real", theOptions.max2iffOptions.sizeReal );
		SetHWNDFixedPointPrecision( hwndQuaternion, "quaternion", theOptions.sizeQuaternion );

#if 0
		hwndDontPrompt = GetDlgItem( hDlg, IDC_DONT_PROMPT );
		assert( hwndDontPrompt );

		if ( GetLocalMachineStringRegistryEntry( szRegMax2IFFLVL, "Don't Prompt", str, sizeof( str ) ) )
			bDontPrompt = atof( str );

		bool bShiftPressed = GetAsyncKeyState( VK_SHIFT );
		if ( bDontPrompt && !bShiftPressed )
		{
			EndDialog( hDlg, TRUE );
		}
		else
		{
			Button_SetCheck( hwndDontPrompt, bDontPrompt );
		}
#endif
	}
	else if ( msg == WM_COMMAND )
	{
		int button = LOWORD( wParam );
		switch ( button )
		{
			case IDOK:
			{
				assert( hwndBinary );
				theOptions.max2iffOptions.bOutputBinary = Button_GetState( hwndBinary );
				SetLocalMachineStringRegistryEntry( szRegMax2IFFLVL, "Output Mode",
					theOptions.max2iffOptions.bOutputBinary ? "binary" : "ascii" );

				assert( hwndScale );
				char szScale[ 100 ];
				Edit_GetText( hwndScale, szScale, sizeof( szScale ) );
				SetLocalMachineStringRegistryEntry( szRegMax2IFFLVL, "Scale", szScale );
				theOptions.max2iffOptions.geometryScale = atof( szScale );

//				SAVE_BUTTON( hwndDontPrompt, "Don't Prompt" );

				SAVE_BUTTON( hwndGeometry, "Output Geometry" );
				theOptions.max2iffOptions.bOutputGeometry = bool( Button_GetState( hwndGeometry ) );
				SAVE_BUTTON( hwndAnimation, "Output Animation" );
				theOptions.max2iffOptions.bOutputAnimation = bool( Button_GetState( hwndAnimation ) );
				SAVE_BUTTON( hwndMaterials, "Output Materials" );
				theOptions.max2iffOptions.bOutputMaterials = bool( Button_GetState( hwndMaterials ) );
				SAVE_BUTTON( hwndHandles, "Output Handles" );
				theOptions.max2iffOptions.bOutputHandles = bool( Button_GetState( hwndHandles ) );
				SAVE_BUTTON( hwndEvents, "Output Events" );
				theOptions.max2iffOptions.bOutputEvents = bool( Button_GetState( hwndEvents ) );
#if 0
				SAVE_BUTTON( hwndGPosition, "Output Global Position" );
				theOptions.bOutputGlobalPosition = bool( Button_GetState( hwndGPosition ) );
				SAVE_BUTTON( hwndGRotation, "Output Global Rotation" );
				theOptions.bOutputGlobalRotation = bool( Button_GetState( hwndGRotation ) );
				SAVE_BUTTON( hwndGScale, "Output Global Scale" );
				theOptions.bOutputGlobalScale = bool( Button_GetState( hwndGScale ) );
#endif
				SAVE_BUTTON( hwndAttributes, "Output Attributes" );
				theOptions.bOutputAttributes = bool( Button_GetState( hwndAttributes ) );
				SAVE_BUTTON( hwndPaths, "Output Paths" );
				theOptions.bOutputPaths = bool( Button_GetState( hwndPaths ) );
				SAVE_BUTTON( hwndModels, "Output Models" );
				theOptions.bOutputModels = bool( Button_GetState( hwndModels ) );

				GetHWNDFixedPointPrecision( hwndReal, "real", theOptions.max2iffOptions.sizeReal );
				GetHWNDFixedPointPrecision( hwndQuaternion, "quaternion", theOptions.sizeQuaternion );

				EndDialog( hDlg, TRUE );
				return TRUE;
			}

			case IDCANCEL:
				EndDialog( hDlg, FALSE );
				return FALSE;
		}
	}
	return FALSE;
}


#if 0
	ofstream out( szOutputFilename, ios::out | ios::binary );
	if ( out.fail() )
	{
		MessageBox( GetActiveWindow(), szOutputFilename, "Unable to create output file:", MB_OK );
		return false;
	}
#endif
